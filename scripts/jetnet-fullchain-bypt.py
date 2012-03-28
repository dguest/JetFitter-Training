#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

"""
runs full chain on <file list>, trains one nn for each pt category 
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import training, pyprep, profile
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

    if not double_variables: 
        double_variables = [
            'energyFraction', 
            'significance3d',         
            'meanTrackRapidity', 
            'maxTrackRapidity', 
            'minTrackRapidity', 
            ]

    if not int_variables: 
        int_variables = [ 
            'nVTX', 
            'nTracksAtVtx', 
            'nSingleTracks', 
            ]
    
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

        proc = RDSProcess(
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

class RDSProcess(multiprocessing.Process): 
    def __init__(self, reduced_dataset, working_dir, training_variables, 
                 do_test = False): 
        super(RDSProcess,self).__init__()

        self._reduced_dataset = reduced_dataset
        self._working_dir = working_dir
        self._training_variables = training_variables
        self._do_test = do_test

    def run(self): 

        reduced_dataset = self._reduced_dataset
        working_dir = self._working_dir
        training_variables = self._training_variables

        # --- profiling 
        profile_dir = os.path.join(working_dir,'profile')
        if not os.path.isdir(profile_dir): 
            os.mkdir(profile_dir)

        profile_file = os.path.join(profile_dir, 'profiled.root')
        mean_rms_file = os.path.join(profile_dir, 'mean_rms.txt')
        if not do_test: 
            if not os.path.isfile(mean_rms_file): 
                if not os.path.isfile(profile_file): 
                    profile.make_profile_file(reduced_dataset, profile_file)
            
                profile.build_mean_rms_from_profile(
                    profile_file = profile_file, 
                    text_file_name = mean_rms_file)

        # --- training part 
        training_dir = os.path.join(working_dir,'training')
        if not os.path.isdir(training_dir): 
            os.mkdir(training_dir)
    
        normalization_file = os.path.join(training_dir, 'normalization.txt')
        if not os.path.isfile(normalization_file) and not do_test: 
            if not os.path.isfile(profile_file): 
                profile.make_profile_file(reduced_dataset, profile_file)
    
            profile.make_normalization_file(
                profile_file, 
                normalization_file = normalization_file, 
                whitelist = training_variables)
                                            
        normalization_dict = {}
        if os.path.isfile(normalization_file): 
            with open(normalization_file) as norm_file: 
                for line in norm_file: 
                    line = line.strip()
                    if not line: continue
                    name = line.split()[0]
                    offset, scale = (float(n) for n in line.split()[1:])
                    normalization_dict[name] = (offset, scale)
    
        print 'normalization:'
        text_size = max(len(x) for x in normalization_dict.keys()) + 1
        for value, (offset, scale) in normalization_dict.iteritems(): 
            print '%-*s offset: % -10.4g scale: % -10.4g' % (
                text_size, value, offset, scale)
    
        weights_path = os.path.join(training_dir, 'weightMinimum.root')
        if not os.path.isfile(weights_path): 
            training.run_training(reduced_dataset = reduced_dataset, 
                                  output_directory = training_dir, 
                                  normalization = normalization_dict, 
                                  debug = do_test)
    
        # --- diagnostics part 
        testing_dir = os.path.join(working_dir, 'testing')
        if not os.path.isdir(testing_dir): 
            os.mkdir(testing_dir)
    
        ovrtrn_hist_path = os.path.join(testing_dir,'overtraining_hists.root')
        if not os.path.isfile(ovrtrn_hist_path): 
            training.run_performance(reduced_dataset = reduced_dataset, 
                                     weights_file = weights_path, 
                                     output_file = ovrtrn_hist_path, 
                                     with_ip3d = True, 
                                     print_and_exit = do_test) 
    
        test_ntuple_path = os.path.join(testing_dir,'perf_ntuple.root')
        if not os.path.isfile(test_ntuple_path): 
            training.run_test_ntuple(reduced_dataset = reduced_dataset, 
                                     weights_file = weights_path, 
                                     output_file = test_ntuple_path, 
                                     print_and_exit = do_test)
        
        rej_hist_path = os.path.join(testing_dir, 'performance_hists') 
        if not os.path.isdir(rej_hist_path): 
            os.mkdir(rej_hist_path)
    
        # --- other diagnostics 
    
        if not do_test: 
            try: 
                import AtlasStyle
            except ImportError: 
                warn('could not import AtlasStyle', UglyWarning)
    
            all_canvas = rejection.make_plots_from(test_ntuple_path)
    
            formats = ['.pdf','.png']
    
            for ext in formats: 
                for plot in all_canvas: 
                    fullname = plot.GetName() + ext
                    fullpath = os.path.join(rej_hist_path,fullname)
                    print 'printing to %s' % fullpath
                    plot.Print(fullpath)
    
            for signal in ['charm','bottom']: 
    
                perf_canvases = performance.b_tag_plots(
                    ovrtrn_hist_path, 
                    normalize = False, 
                    signal = signal)
    
                output_name = '%s_perf_result' % (signal)
                output_path = os.path.join(rej_hist_path, output_name)
                for output_format in formats: 
                    full_name = output_path + output_format
                    perf_canvases[0].Print(full_name)
    
    
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
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('--rapidity', action = 'store_true', 
                      dest = 'do_rapidity', 
                      help = 'use rapidity variables in training')
    parser.add_option('--kinematic', action = 'store_true', 
                      help = 'use pt and eta variables in training')
    parser.add_option('--sv1', action = 'store_true', 
                      help = 'use SV1')
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

    if options.sv1: 
        training_variables += ['discriminatorSV1']

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
