# Author: Daniel Guest (dguest@cern.ch)

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from jetnet import training, pyprep, profile
from jetnet.perf import rejection, performance
import os, sys, glob
from warnings import warn
import multiprocessing


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
        if not os.path.isfile(profile_file):
            rds_dir = os.path.split(reduced_dataset)[0]
            alt_profile_file = os.path.join(rds_dir, 'profiled.root')
            if os.path.isfile(alt_profile_file): 
                profile_file = alt_profile_file

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