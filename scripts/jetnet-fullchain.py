#!/usr/bin/env python2.6

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

from optparse import OptionParser, OptionGroup
from ConfigParser import SafeConfigParser

class UglyWarning(UserWarning): pass

default_pt_divisions = [25.0, 35.0, 50.0, 80.0, 120.0, 200.0]
observer_discriminators = ['IP2D','IP3D','SV1','COMB']
training_variable_whitelist = [
    'nVTX', 
    'nTracksAtVtx', 
    'nSingleTracks', 
    'energyFraction', 
    'mass',  
    'significance3d', 
    'discriminatorIP3D', 
    'cat_eta', 
    'cat_pT', 
    ]

rapidity_vars = [
   'minTrackRapidity', 
   'meanTrackRapidity', 
   'maxTrackRapidity', 
    ]

def run_full_chain(input_files, working_dir = None, output_path = None, 
                   rds_name = 'reduced_dataset.root', 
                   jet_collection = 'AntiKt4TopoEMJets', 
                   do_test = False, 
                   pt_divisions = default_pt_divisions, 
                   flavor_weights = {}, 
                   training_variables = training_variable_whitelist): 

    double_variables, int_variables = rds.get_allowed_rds_variables(
        input_files = input_files, 
        jet_collection = jet_collection)

    if working_dir is None: 
        working_dir = jet_collection

    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- rds part
    reduced_dir = os.path.join(working_dir, 'reduced')
    if not os.path.isdir(reduced_dir): 
        os.mkdir(reduced_dir)

    rds_path = os.path.join(reduced_dir, rds_name)
    
    if not os.path.isfile(rds_path): 
        pyprep.prep_ntuple(input_files = input_files, 
                           double_variables = double_variables, 
                           int_variables = int_variables, 
                           observer_discriminators = observer_discriminators, 
                           pt_divisions = pt_divisions, 
                           jet_collection = jet_collection, 
                           output_file = rds_path, 
                           debug = do_test)


    proc = process.RDSProcess(
        reduced_dataset = rds_path, 
        working_dir = working_dir, 
        training_variables = training_variables, 
        flavor_weights = flavor_weights, 
        do_test = do_test)
    proc.start()
    proc.join()


    
if __name__ == '__main__': 

    usage = 'usage: %prog <file list> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False, 
        randomize_reduced_dataset = False, 
        do_rapidity = False, 
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('--rapidity', action = 'store_true', 
                      dest = 'do_rapidity', 
                      help = 'use rapidity variables in training')
    parser.add_option('--whitelist', 
                      help = 'whitelist textfile for training', 
                      default = None)
    parser.add_option('--config', 
                      help = 'use this configuration file', 
                      default = 'jetnet.cfg')

    (options, args) = parser.parse_args(sys.argv)

    do_test = options.test

    training_variables = training_variable_whitelist
    if options.whitelist: 
        training_variables = []
        if options.do_rapidity: 
            warn('--whitelist option overrides --rapidity option')
        with open(options.whitelist) as white_file: 
            for line in white_file: 
                var = line.strip()
                if var: 
                    training_variables.append(var)

    config_file_name = options.config
    flavor_weights = {}
    if os.path.isfile(config_file_name): 
        config = SafeConfigParser()
        config.read(config_file_name)
        flavor_weights = dict( 
            (f, float(w)) for f,w in config.items('weights') )
        pt_divisions = config.get('preprocessing','pt_divisions').split()
    else: 
        sys.exit('could not find config file %s' % config_file_name)

    else: 
        if options.do_rapidity: 
            training_variables += rapidity_vars

    if len(args) == 2: 
        input_files = []
        with open(args[1]) as file_list: 
            for line in file_list: 
                input_files.append(line.strip())

        print 'running full chain' 
        run_full_chain(input_files, do_test = do_test, 
                       training_variables = training_variables, 
                       flavor_weights = flavor_weights, 
                       pt_divisions = pt_divisions)
