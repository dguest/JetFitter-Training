#!/usr/bin/env python2.7

# Author: Daniel Guest (dguest@cern.ch)

"""
runs full chain on <file list>

options set in 'jetnet.cfg'
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import pyprep, process, rds
import os, sys, glob, shutil
from warnings import warn
from math import log, exp

import argparse
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass


def train_and_test(input_files, testing_dataset, 
                   pt_divisions, training_variables, 
                   observer_discriminators, 
                   flavor_weights, 
                   config_file, # this should be used for more opts
                   jet_collection = 'BTag_AntiKt4TopoEMJetsReTagged', 
                   jet_tagger = 'JetFitterCharm', 
                   working_dir = None, 
                   output_path = None, 
                   rds_name = 'reduced_dataset.root', 
                   do_test = False, 
                   ): 

    if working_dir is None: 
        working_dir = jet_collection

    if not os.path.isfile(testing_dataset): 
        raise IOError('{} not found'.format(testing_dataset))


    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- rds part

    # get weights file 
    reduced_dir = os.path.join(working_dir, 'reduced')
    if not os.path.isdir(reduced_dir): 
        os.mkdir(reduced_dir)

    rds_path = os.path.join(reduced_dir, rds_name)

    weight_file = os.path.join(reduced_dir, 'weights.root')
    if not os.path.isfile(weight_file): 
        
        # build a light ntuple if one doesn't exist
        if os.path.isfile(rds_path): 
            small_rds_path = rds_path 

        else: 
            print '--- making flat ntuple to build weight file ---'

            rds_dir, rds_name = os.path.split(rds_path)
            small_rds = '.'.join(rds_name.split('.')[:-1]) + '_small.root'
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


    proc = process.RDSProcess(
        reduced_dataset = rds_path, 
        working_dir = working_dir, 
        training_variables = training_variables, 
        flavor_weights = flavor_weights, 
        testing_dataset = testing_dataset, 
        do_more_diagnostics = False,
        do_test = do_test, 
        config_file = config_file)
    proc.start()
    proc.join()
    proc_outputs = proc.out_queue.get(block = False)

    # make the summary folder 

    working_dir_list = working_dir.split('/')[:-1]
    if not working_dir_list: 
        summary_dir = 'summary'
    else: 
        working_dir_parent = os.path.join(*working_dir_list)
        summary_dir = os.path.join(working_dir_parent,'summary')

    if not os.path.isdir(summary_dir): 
        os.mkdir(summary_dir)

    if 'profile' in proc_outputs: 

        profile_summery_name = jet_tagger + '_profile.root'
        profile_summery_path = os.path.join(summary_dir,profile_summery_name)
        shutil.copyfile(proc_outputs['profile'], profile_summery_path)


    this_config_name = jet_tagger + '_config_file.cfg'
    this_config_path = os.path.join(summary_dir, this_config_name)
    shutil.copyfile(config_file, this_config_path)

    
if __name__ == '__main__': 

    description = __doc__

    parser = argparse.ArgumentParser(
        description = description, 
        epilog = 'author: Dan Guest <dguest@cern.ch>')
    parser.set_defaults(
        test = False, 
        )

    parser.add_argument('input_files')
    parser.add_argument('--test', action = 'store_true')
    parser.add_argument(
        '--whitelist', dest = 'wt', 
        help = 'whitelist textfile for training (default: %(default)s)', 
        default = 'whitelist.txt')
    parser.add_argument(
        '--config', 
        help = 'use this configuration file (default: %(default)s)', 
        default = 'jetnet.cfg')

    options = parser.parse_args(sys.argv[1:])

    do_test = options.test
    working_dir = None

    training_variables = []

    config_file_name = options.config
    flavor_weights = {}
    if not os.path.isfile(config_file_name): 
        config_files = glob.glob('*.cfg')
        if len(config_files) > 1: 
            sys.exit('found multiple config files {}, '
                     'not sure which to use'.format(config_files))
        elif not config_files: 
            sys.exit('could not find config file')
        else: 
            config_file_name = config_files[0]

    config = SafeConfigParser()
    config.read(config_file_name)
    flavor_weights = { k : float(w) for k, w in config.items('weights') }
    pt_divisions = map(
        float,config.get('preprocessing','pt_divisions').split() )
    jet_tagger = config.get('preprocessing','jet_tagger')
    testing_dataset = config.get('testing', 'testing_dataset')
    observer_discriminators = config.get(
        'preprocessing','observer_discriminators').split()

    if config.has_option('training','variables'): 
        training_variables = config.get('training','variables').split()
        if os.path.isfile(options.wt): 
            sys.exit(
                "variables already given in config file, don't use "
                "whitelist")
        
    else: 
        with open(options.wt) as white_file: 
            for line in white_file: 
                var = line.strip()
                if var: 
                    training_variables.append(var)

        warn('whitelist will soon be merged with the config file '
             'under [training] --> variables', 
             FutureWarning)

    if 'ARRAYID' in jet_tagger: 
        the_array_id = os.environ['PBS_ARRAYID'].rjust(2,'0')
        jet_tagger = jet_tagger.replace('ARRAYID',the_array_id)
        working_dir = jet_tagger
        testing_dataset = os.path.join(working_dir,testing_dataset)


    input_files = []
    with open(options.input_files) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())

    print 'running full chain' 
    train_and_test(
        input_files, 
        testing_dataset = testing_dataset, 
        do_test = do_test, 
        config_file = config_file_name, 
        training_variables = training_variables, 
        flavor_weights = flavor_weights, 
        jet_tagger = jet_tagger, 
        pt_divisions = pt_divisions, 
        working_dir = working_dir, 
        observer_discriminators = observer_discriminators)
