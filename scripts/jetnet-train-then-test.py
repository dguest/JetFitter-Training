#!/usr/bin/env python2.7

"""
runs full chain on <file list>

options set in 'jetnet.cfg'

Author: Daniel Guest (dguest@cern.ch)
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import training, pyprep, profile, rds, process
from jetnet.perf import rejection, performance
import os, sys
from warnings import warn

import argparse
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass

_default_flav_weights = { 
    'light': 1, 
    'charm': 1, 
    'bottom':1, 
    }


def train_and_test(input_files, testing_dataset, 
                   pt_divisions, training_variables, 
                   observer_discriminators, 
                   jet_collection = 'BTag_AntiKt4TopoEMJetsReTagged', 
                   jet_tagger = 'JetFitterCharm', 
                   flavor_weights = _default_flav_weights, 
                   working_dir = None, 
                   output_path = None, 
                   rds_name = 'reduced_dataset.root', 
                   do_test = False, 
                   ): 

    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        jet_collection = jet_collection)

    if working_dir is None: 
        working_dir = jet_collection

    if not os.path.isfile(testing_dataset): 
        raise IOError('{} not found'.format(testing_dataset))

    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- rds part
    reduced_dir = os.path.join(working_dir, 'reduced')
    if not os.path.isdir(reduced_dir): 
        os.mkdir(reduced_dir)

    rds_path = os.path.join(reduced_dir, rds_name)
    
    if not os.path.isfile(rds_path): 
        pyprep.make_flat_ntuple(
            input_files = input_files, 
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
        do_test = do_test)
    proc.start()
    proc.join()


    
if __name__ == '__main__': 

    description = __doc__

    parser = argparse.ArgumentParser(
        description = description, 
        formatter_class = argparse.RawDescriptionHelpFormatter)
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
    with open(options.wt) as white_file: 
        for line in white_file: 
            var = line.strip()
            if var: 
                training_variables.append(var)

    config_file_name = options.config
    flavor_weights = {}
    if os.path.isfile(config_file_name): 
        config = SafeConfigParser()
        config.read(config_file_name)
        flavor_weights = { k : float(w) for k, w in config.items('weights') }
        pt_divisions = map(
            float,config.get('preprocessing','pt_divisions').split() )
        jet_tagger = config.get('preprocessing','jet_tagger')
        testing_dataset = config.get('testing', 'testing_dataset')
        observer_discriminators = config.get(
            'preprocessing','observer_discriminators').split()

        if 'ARRAYID' in jet_tagger: 
            the_array_id = os.environ['PBS_ARRAYID']
            jet_tagger = jet_tagger.replace('ARRAYID',the_array_id)
            working_dir = jet_tagger
            testing_dataset = os.path.join(working_dir,testing_dataset)

    else: 
        sys.exit('could not find config file %s' % config_file_name)


    input_files = []
    with open(options.input_files) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())

    print 'running full chain' 
    train_and_test(
        input_files, 
        testing_dataset = testing_dataset, 
        do_test = do_test, 
        training_variables = training_variables, 
        flavor_weights = flavor_weights, 
        jet_tagger = jet_tagger, 
        pt_divisions = pt_divisions, 
        working_dir = working_dir, 
        observer_discriminators = observer_discriminators)
