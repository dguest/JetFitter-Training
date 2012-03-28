#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

"""
runs full chain on <file list>, trains one nn for each pt category 
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import training, pyprep, profile, process, utils
from jetnet.perf import rejection, performance
import os, sys, glob
from warnings import warn
import multiprocessing

from optparse import OptionParser, OptionGroup


class UglyWarning(UserWarning): pass

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

pt_rel_vars = [ 
   'minTrackPtRel', 
   'meanTrackPtRel', 
   'maxTrackPtRel', 
    ]

kinematic_vars = [ 
    'JetEta', 
    'JetPt', 
    ]


def run_full_chain_by_pt(
    input_files, 
    working_dir = None, 
    output_path = None, 
    rds_dir = 'reduced_pt', 
    jet_collection = 'AntiKt4TopoEMJets', 
    do_test = False, 
    double_variables = None, 
    int_variables = None, 
    training_variables = training_variable_whitelist, 
    cram = False, 
    sequential = False): 

    
    if working_dir is None: 
        working_dir = jet_collection

    if not os.path.isdir(working_dir): 
        os.mkdir(working_dir)

    # --- rds part
    reduced_dir = os.path.join(working_dir, rds_dir)
    if not os.path.isdir(reduced_dir): 
        os.mkdir(reduced_dir)

    reduced_datasets = glob.glob('%s/reduced_*' % reduced_dir)
    if len(reduced_datasets) == 0: 
        double_variables, int_variables = utils.get_allowed_rds_variables(
            input_files = input_files, jet_collection = jet_collection)

        pyprep.make_ntuples_ptcat(
            input_files = input_files, 
            double_variables = double_variables, 
            int_variables = int_variables, 
            observer_discriminators = observer_discriminators, 
            jet_collection = jet_collection, 
            output_dir = reduced_dir, 
            debug = do_test )
    
    reduced_datasets = glob.glob('%s/reduced_*' % reduced_dir)

    n_processors = multiprocessing.cpu_count()
    # -- allow one less cpu than process, 
    #    the low bin doesn't run anyway 
    if n_processors < len(reduced_datasets) - 1: 
        print 'WARNING: not enough processors for these subjobs '
        'want %i, found %i' % (len(reduced_datasets), n_processors)
        if not cram and not sequential: 
            sys.exit('quitting...')
            
        
    subprocesses = []
    for rds in reduced_datasets: 
        rds_basename = os.path.basename(rds).rstrip('.root')
        category = rds_basename.lstrip('reduced_')
        working_subdir = os.path.join(working_dir,'pt_' + category)
        if not os.path.isdir(working_subdir): 
            os.mkdir(working_subdir)

        proc = process.RDSProcess(
            reduced_dataset = rds, 
            working_dir = working_subdir, 
            training_variables = training_variables, 
            do_test = do_test)
        proc.start()
        subprocesses.append(proc)
        if sequential: 
            proc.join()

    for proc in subprocesses: 
        proc.join()

    return 0

    
    
if __name__ == '__main__': 

    usage = 'usage: %prog <file list> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False, 
        do_rapidity = False, 
        kinematic = False, 
        sv1 = False, 
        cram = False, 
        sequential = False, 
        dis = False, 
        ptrel = False, 
        )

    parser.add_option('--test', action = 'store_true')

    var_opt = OptionGroup(parser, title = 'variable flags', 
                          description = 'flags to control training '
                          'variables')
    var_opt.add_option('--rapidity', action = 'store_true', 
                       dest = 'do_rapidity', 
                       help = 'use rapidity variables in training')
    var_opt.add_option('--kinematic', action = 'store_true', 
                       help = 'use pt and eta variables in training')
    var_opt.add_option('--ptrel', action = 'store_true', 
                       dest = 'ptrel', 
                       help = 'use ptrel')
    var_opt.add_option('--sv1', action = 'store_true', 
                       help = 'use SV1')
    var_opt.add_option('--dis', action = 'store_true', 
                       help = 'use distance to secondary vertex')
    parser.add_option_group(var_opt)

    parser.add_option('--cram', action = 'store_true', 
                      help = 'allow more procs than we have')
    parser.add_option('--sequential', action = 'store_true', 
                      help = 'force sequential processing')

    (options, args) = parser.parse_args(sys.argv)

    do_test = options.test

    training_variables = training_variable_whitelist
    if options.do_rapidity: 
        training_variables += rapidity_vars

    if options.kinematic: 
        training_variables += kinematic_vars
        
    if options.ptrel: 
        training_variables += pt_rel_vars

    if options.sv1: 
        training_variables += ['discriminatorSV1']

    if options.dis: 
        training_variables += ['leadingVertexPosition']

    

    if not len(args) == 2: 
        print parser.get_usage()

    else: 
        input_files = []
        with open(args[1]) as file_list: 
            for line in file_list: 
                input_files.append(line.strip())

        print 'running full chain' 
        run_full_chain_by_pt(
            input_files, 
            do_test = do_test, 
            training_variables = training_variables, 
            cram = options.cram, 
            sequential = options.sequential)
