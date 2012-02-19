#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

"""
runs full chain on <file list>
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


from jetnet import training, pyprep
from jetnet.perf import rejection, performance
import os, sys
from warnings import warn

from optparse import OptionParser, OptionGroup


class UglyWarning(UserWarning): pass

observer_discriminators = ['IP2D','IP3D','SV1','COMB']

def run_full_chain(input_files, working_dir = None, output_path = None, 
                   rds_name = 'reduced_dataset.root', 
                   jet_collection = 'AntiKt4TopoEMJets', 
                   do_test = False, 
                   randomize_reduced_dataset = False): 
    
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
                           observer_discriminators = observer_discriminators, 
                           jet_collection = jet_collection, 
                           output_file = rds_path, 
                           debug = do_test, 
                           randomize = randomize_reduced_dataset)

    # --- training part 
    training_dir = os.path.join(working_dir,'training')
    if not os.path.isdir(training_dir): 
        os.mkdir(training_dir)

    weights_path = os.path.join(training_dir, 'weightMinimum.root')
    if not os.path.isfile(weights_path): 
        training.run_training(reduced_dataset = rds_path, 
                              output_directory = training_dir, 
                              with_ip3d = True, 
                              debug = do_test)

    # --- diagnostics part 
    testing_dir = os.path.join(working_dir, 'testing')
    if not os.path.isdir(testing_dir): 
        os.mkdir(testing_dir)

    ovrtrn_hist_path = os.path.join(testing_dir,'overtraining_hists.root')
    if not os.path.isfile(ovrtrn_hist_path): 
        training.run_performance(reduced_dataset = rds_path, 
                                 weights_file = weights_path, 
                                 output_file = ovrtrn_hist_path, 
                                 with_ip3d = True, 
                                 print_and_exit = do_test) 

    test_ntuple_path = os.path.join(testing_dir,'perf_ntuple.root')
    if not os.path.isfile(test_ntuple_path): 
        training.run_test_ntuple(reduced_dataset = rds_path, 
                                 weights_file = weights_path, 
                                 output_file = test_ntuple_path, 
                                 print_and_exit = do_test)
    
    rej_hist_path = os.path.join(testing_dir, 'performance_hists') 
    if not os.path.isdir(rej_hist_path): 
        os.mkdir(rej_hist_path)


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
        randomize_reduced_dataset = False
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('--random', action = 'store_true', 
                      dest = 'randomize_reduced_dataset', 
                      help = 'randomize the reduced dataset' )

    (options, args) = parser.parse_args(sys.argv)

    do_test = options.test
    randomize_reduced_dataset = options.randomize_reduced_dataset


    if len(args) == 2: 
        input_files = []
        with open(args[1]) as file_list: 
            for line in file_list: 
                input_files.append(line.strip())

        print 'running full chain' 
        run_full_chain(input_files, do_test = do_test, 
                       randomize_reduced_dataset = randomize_reduced_dataset)