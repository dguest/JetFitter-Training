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
    'IP3D', 
    'SV1', 
    'JetFitterCOMBNN', 
    'MV1', 
    'MV2', 
    ]

def make_flat_ntuple(
    input_files, 
    pt_divisions, 
    jet_collection = 'BTag_AntiKt4TopoEMJetsReTagged', 
    jet_tagger = 'JetFitterCharm', 
    output_path = None, 
    rds_path = 'reduced_dataset.root', 
    observer_discriminators = _default_observers, 
    do_test = False, 
    ): 

    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        jet_collection = jet_collection)

    # --- rds part

    rds_dir, rds_file = os.path.split(rds_path)
    if rds_dir and not os.path.isdir(rds_dir): 
        os.mkdir(rds_dir)

    if os.path.isfile(rds_path): 
        raise IOError(
            "{} already exists, refusing to overwrite".format(rds_path) )
    else: 
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
                        help = 'use this configuration file'
                        ' (default: %(default)s)', 
                        default = 'jetnet.cfg')

    options = parser.parse_args(sys.argv[1:])

    do_test = options.test

    rds_path = 'reduced_dataset.root'

    config_file_name = options.config
    flavor_weights = {}
    if os.path.isfile(config_file_name): 
        config = SafeConfigParser()
        config.read(config_file_name)
        pt_divisions = map(
            float,config.get('preprocessing','pt_divisions').split())

        observer_discriminators = config.get(
            'preprocessing','observer_discriminators').split()
        jet_tagger = config.get('preprocessing','jet_tagger')
        print jet_tagger
        if 'ARRAYID' in jet_tagger: 
            the_array_id = os.environ['PBS_ARRAYID']
            jet_tagger = jet_tagger.replace('ARRAYID',the_array_id)
            rds_path = os.path.join(jet_tagger,rds_path)
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
        observer_discriminators = observer_discriminators, 
        jet_tagger = jet_tagger, 
        rds_path = rds_path, 
        pt_divisions = pt_divisions)
