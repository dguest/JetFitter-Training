#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)
"""
runs n-1 variable nn training. 

directories are created with the current directory as the base
"""

import sys, os
import multiprocessing
from optparse import OptionParser, OptionGroup

from jetnet.dirs import OverwriteError
from jetnet import pynn, profile, utils, pyprep

    
observer_discriminators = ['IP2D','IP3D','SV1','COMB']
training_variable_whitelist = [
    'nVTX', 
    'nTracksAtVtx', 
    'nSingleTracks', 
    'energyFraction', 
    'mass',  
    'significance3d', 
    'discriminatorIP3D', 
    'discriminatorSV1',
    'cat_eta', 
    'cat_pT', 
    'minTrackRapidity', 
    'meanTrackRapidity', 
    'maxTrackRapidity', 
    'minTrackPtRel', 
    'meanTrackPtRel', 
    'maxTrackPtRel', 
    'leadingVertexPosition', 
    'JetEta', 
    'JetPt', 
    ]

def get_all_vars_in_rds(rds_name): 
    from ROOT import TFile
    the_rds = TFile(rds_name)
    tree = the_rds.Get('SVTree')
    leaf_dict = utils.get_leaves_in_tree(tree)
    all_leaf_names = []
    for name_list in leaf_dict.values(): 
        all_leaf_names += name_list

    return all_leaf_names

def run_pruned_chains(
    input_files, 
    working_dir = None, 
    output_path = None, 
    rds_name = 'reduced_dataset.root', 
    rds_dir = 'reduced', 
    jet_collection = 'AntiKt4TopoEMJets', 
    do_test = False, 
    double_variables = None, 
    int_variables = None, 
    training_variables = training_variable_whitelist): 


    
    if working_dir is None: 
        working_dir = jet_collection

    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- rds part
    reduced_dir = os.path.join(working_dir, rds_dir)
    if not os.path.isdir(reduced_dir): 
        os.mkdir(reduced_dir)

    reduced_dataset = os.path.join(reduced_dir, rds_name)

    if not os.path.isfile(reduced_dataset): 
        double_variables, int_variables = utils.get_allowed_rds_variables(
            input_files = input_files, jet_collection = jet_collection)

        pyprep.prep_ntuple(input_files = input_files, 
                           double_variables = double_variables, 
                           int_variables = int_variables, 
                           observer_discriminators = observer_discriminators, 
                           jet_collection = jet_collection, 
                           output_file = reduced_dataset, 
                           debug = do_test)
        
    profile_file = os.path.join(reduced_dir, 'profiled.root')
    if not os.path.isfile(profile_file): 
        profile.make_profile_file(reduced_dataset, profile_file)

    rds_variables = get_all_vars_in_rds(reduced_dataset)
    both_vars = [v for v in training_variables if v in rds_variables]

    n_processors = multiprocessing.cpu_count()

    if n_processors < len(both_vars): 
        sys.exit('ERROR: too few procs: need %i, have %i' %
                 (len(both_vars), n_processors))
            
        
    subprocesses = []
    for exclude in both_vars: 
        working_subdir = os.path.join(working_dir,'no_' + exclude)
        if not os.path.isdir(working_subdir): 
            os.mkdir(working_subdir)

        training_subset = [v for v in both_vars if v != exclude ] 
        proc = process.RDSProcess(
            reduced_dataset = rds, 
            working_dir = working_subdir, 
            training_variables = training_subset, 
            do_test = do_test)
        proc.start()
        subprocesses.append(proc)

    for proc in subprocesses: 
        proc.join()

    return 0

    
    
if __name__ == '__main__': 

    usage = 'usage: %prog <file list> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False, 
        )

    parser.add_option('--test', action = 'store_true')

    (options, args) = parser.parse_args(sys.argv)

    do_test = options.test

    if not len(args) == 2: 
        print parser.get_usage()

    else: 
        input_files = []
        with open(args[1]) as file_list: 
            for line in file_list: 
                input_files.append(line.strip())

        print 'running pruning' 
        run_pruned_chains(
            input_files, 
            do_test = do_test, 
            training_variables = training_variable_whitelist)
