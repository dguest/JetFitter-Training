#!/usr/bin/env python2.7

# Author: Daniel Guest (dguest@cern.ch)

"""
runs full chain on <file list>

options set in 'jetnet.cfg'
"""
__author__ = 'Daniel Guest <dguest@cern.ch>'

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import pyprep, process, rds
import os, sys, glob, shutil, time
from warnings import warn
from math import log, exp

import argparse
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass

_hold_name = 'HOLD_JOBS'
def hold_job(working_dir): 
    sleep_for = 5
    while os.path.isfile(os.path.join(working_dir,_hold_name)): 
        if sleep_for < 10: 
            print 'found hold order, holding...'
        time.sleep(sleep_for)
        if sleep_for < 300: 
            sleep_for += 10

def set_hold(working_dir, value = True): 
    hold_file = os.path.join(working_dir,_hold_name)
    if value == True: 
        if not os.path.isfile(hold_file): 
            print 'setting hold'
            with open(hold_file,'w') as hold: 
                hold.write('holding while datasets are transcribed\n')
    else: 
        if os.path.isfile(hold_file): 
            os.remove(hold_file)


def train_and_test(input_files, 
                   config_file, 
                   working_dir = None, 
                   do_test = False, 
                   ): 


    config = SafeConfigParser()
    config.read(config_file)

    # --- setup preprocessing
    preproc = dict(config.items('preprocessing'))
    jet_collection = preproc['jet_collection']


    pt_divisions = [float(x) for x in preproc['pt_divisions'].split() ]
    observer_discriminators = preproc['observer_discriminators'].split()

    # --- early load of post-training options  
    training_opts = dict(config.items('training'))
    testing_opts = dict(config.items('testing'))
    training_variables = training_opts['variables'].split()

    testing_dataset = None

    if 'testing_dataset' in testing_opts: 
        testing_dataset = testing_opts['testing_dataset']

    # --- change some things if this is an array job
    jet_tagger = preproc['jet_tagger']
    if 'ARRAYID' in jet_tagger: 
        the_array_id = os.environ['PBS_ARRAYID'].rjust(2,'0')
        jet_tagger = jet_tagger.replace('ARRAYID',the_array_id)
        working_dir = jet_tagger
        if testing_dataset: 
            testing_dataset = os.path.join(working_dir,testing_dataset)

    if testing_dataset and not os.path.isfile(testing_dataset): 
        raise IOError('{} not found'.format(testing_dataset))

    flavor_weights = {}
    if config.has_section('weights'): 
        warn('moving [weights] contents into [training] section', 
             FutureWarning)
        flavor_weights = dict( config.items('weights') )
        for wt_name, wt in flavor_weights.items(): 
            config.set('training', wt_name + '_wt', wt)
        config.remove_section('weights')
        with open(config_file_name,'w') as new_cfg: 
            config.write(new_cfg)

    flavors = ['bottom','charm','light']
    flavor_weights = { 
        f : config.get('training', f + '_wt') for f in flavors
        }
    for f in flavor_weights: 
        flavor_weights[f] = float(flavor_weights[f])


    # --- setup the working directory 
    if not working_dir: 
        working_dir = jet_collection
    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- hold here if someone else is working 
    hold_job(working_dir)
    set_hold(working_dir)

    # --- rds part
    rds_name = 'reduced_dataset.root'
    # get weights file 
    rds_dir = os.path.join(working_dir, 'reduced')
    if not os.path.isdir(rds_dir): 
        os.mkdir(rds_dir)

    rds_path = os.path.join(rds_dir, rds_name )
    if not testing_dataset: 
        testing_dataset = rds_path

    weight_file = os.path.join(rds_dir, 'weights.root')
    if not os.path.isfile(weight_file): 
        
        # build a light ntuple if one doesn't exist
        if os.path.isfile(rds_path): 
            small_rds_path = rds_path 

        else: 
            print '--- making flat ntuple to build weight file ---'
            small_rds = 'small_rds.root'
            small_rds_path = os.path.join(rds_dir,small_rds)
            if not os.path.isfile(small_rds_path): 
                pyprep.make_flat_ntuple(
                    input_files = input_files, 
                    jet_collection = jet_collection, 
                    jet_tagger = jet_tagger, 
                    output_file = small_rds_path)
            
        pt_low, pt_high = (15.0, 300)
        log_span = log(pt_high) - log(pt_low)
        log_range = [log(pt_low) + i * log_span / 10 for i in xrange(11)]
        pt_bins = [exp(x) for x in log_range]

        print '--- making weight file ---'
        from jetnet import cxxprofile
        cxxprofile.pro2d(
            in_file = small_rds_path, 
            tree = 'SVTree', 
            plots = [( ('JetPt', pt_bins),
                       ('JetEta',10,-2.5,2.5) )], 
            tags = ['bottom','charm','light'], 
            out_file = weight_file, 
            show_progress = True)


    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        full_dir_name = jet_collection + '_' + jet_tagger)


    if not os.path.isfile(rds_path): 
        print '--- making flattened dataset for training ---'
        pyprep.make_flat_ntuple(
            input_files = input_files, 
            weight_file = weight_file, 
            double_variables = double_variables, 
            int_variables = int_variables, 
            observer_discriminators = observer_discriminators, 
            pt_divisions = pt_divisions, 
            jet_collection = jet_collection, 
            jet_tagger = jet_tagger, 
            output_file = rds_path, 
            debug = do_test, 
            )

    # --- unset other job hold 
    set_hold(working_dir, value = False)

    proc = process.RDSProcess(
        reduced_dataset = rds_path, 
        working_dir = working_dir, 
        training_variables = training_variables, 
        flavor_weights = flavor_weights, 
        testing_dataset = testing_dataset, 
        do_test = do_test, 
        config_file = config_file)
    proc.start()
    proc.join()
    proc_outputs = proc.out_queue.get(block = False)


    # --- make the summary folder 

    working_dir_list = working_dir.split('/')[:-1]
    if not working_dir_list: 
        summary_dir = 'summary'
    else: 
        working_dir_parent = os.path.join(*working_dir_list)
        summary_dir = os.path.join(working_dir_parent,'summary')

    if not os.path.isdir(summary_dir): 
        os.mkdir(summary_dir)

    summary_base_name, cfg_ext = os.path.splitext(config_file)
    if 'PBS_ARRAYID' in os.environ: 
        summary_base_name += '_subjob{}'.format(os.environ['PBS_ARRAYID'])

    if 'profile' in proc_outputs: 
        profile_summary_name = summary_base_name + '_profile.root'
        profile_summary_path = os.path.join(summary_dir,profile_summary_name)
        shutil.copyfile(proc_outputs['profile'], profile_summary_path)


    this_config_name = summary_base_name + cfg_ext
    this_config_path = os.path.join(summary_dir, this_config_name)
    shutil.copyfile(config_file, this_config_path)

    
if __name__ == '__main__': 

    description = __doc__

    parser = argparse.ArgumentParser(
        description = description, 
        epilog = 'author: ' + __author__)
    parser.set_defaults(
        test = False, 
        )

    parser.add_argument('input_files')
    parser.add_argument(
        'config', 
        help = 'use this configuration file (default: %(default)s)', 
        default = 'jetnet.cfg')
    parser.add_argument('--test', action = 'store_true')

    options = parser.parse_args(sys.argv[1:])

    do_test = options.test
    working_dir = None

    config_file_name = options.config
    if not os.path.isfile(config_file_name): 
        config_files = glob.glob('*.cfg')
        if len(config_files) > 1: 
            sys.exit('found multiple config files {}, '
                     'not sure which to use'.format(config_files))
        elif not config_files: 
            sys.exit('could not find config file')
        else: 
            config_file_name = config_files[0]

    input_files = []
    with open(options.input_files) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())


    print 'running full chain' 
    train_and_test(
        input_files, 
        config_file = config_file_name, 
        do_test = do_test, 
        working_dir = working_dir, 
        )
