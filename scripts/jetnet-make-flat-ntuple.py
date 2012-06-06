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

from argparse import ArgumentParser
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass

_default_observers = [ 
    'JetFitterCOMBNN', 
    'MV1', 
    'MV2', 
    ]

def make_flat_ntuple(
    input_files, 
    pt_divisions, 
    jet_collection = 'BTag_AntiKt4TopoEMJetsReTagged', 
    jet_tagger = 'JetFitterCharm', 
    working_dir = '.', 
    output_path = None, 
    rds_name = 'reduced_dataset.root', 
    observer_discriminators = _default_observers, 
    do_test = False, 
    ): 

    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        jet_collection = jet_collection)

    if not working_dir: 
        working_dir = jet_collection

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


    
if __name__ == '__main__': 

    description = __doc__

    parser = ArgumentParser(description = description)
    parser.set_defaults(
        test = False, 
        )

    parser.add_argument('input_files', nargs = 1)
    parser.add_argument('--test', action = 'store_true')
    parser.add_argument('--config', 
                        help = 'use this configuration file', 
                        default = 'jetnet.cfg')

    options = parser.parse_args(sys.argv[1:])

    do_test = options.test

    config_file_name = options.config
    flavor_weights = {}
    if os.path.isfile(config_file_name): 
        config = SafeConfigParser()
        config.read(config_file_name)
        pt_divisions = map(
            float,config.get('preprocessing','pt_divisions').split())
    else: 
        sys.exit('could not find config file %s' % config_file_name)


    input_files = []
    with open(options.input_files[0]) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())

    print 'making flat ntuple' 
    make_flat_ntuple(
        input_files, 
        do_test = do_test, 
        pt_divisions = pt_divisions)