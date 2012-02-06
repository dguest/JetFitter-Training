#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

"""
usage: 
jetnet-chain.py  <input file list> 

runs full chain
"""


from jetnet import training, pyprep
from jetnet.perf import rejection
import os, sys
from warnings import warn

class UglyWarning(UserWarning): pass

observer_discriminators = ['IP2D','IP3D','SV1','COMB']

def run_full_chain(input_files, working_dir = None, output_path = None, 
                   rds_name = 'reduced_dataset.root', 
                   jet_collection = 'AntiKt4TopoEMJets', 
                   do_test = False): 
    
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
        pyprep.prep_ntuple(input_file_list = input_files, 
                           observer_discriminators = observer_discriminators, 
                           jet_collection_name = jet_collection, 
                           output_file_name = rds_path, 
                           debug = do_test)

    # --- training part 
    training_dir = os.path.join(working_dir,'training')
    if not os.path.isdir(training_dir): 
        os.mkdir(training_dir)

    weights_path = os.path.join(training_dir, 'weightMinimum.root')
    if not os.path.isfile(weights_path): 
        training.run_training(in_path = rds_path, 
                              output_dir = training_dir, 
                              with_ip3d = True, 
                              debug = do_test)

    # --- diagnostics part 
    testing_dir = os.path.join(working_dir, 'testing')
    if not os.path.isdir(testing_dir): 
        os.mkdir(testing_dir)

    ovrtrn_hist_path = os.path.join(testing_dir,'overtraining_hists.root')
    if not os.path.isfile(ovrtrn_hist_path): 
        training.run_performance(input_file = rds_path, 
                                 weights_file = weights_path, 
                                 output_file = ovrtrn_hist_path, 
                                 with_ip3d = True, 
                                 print_and_exit = do_test) 

    test_ntuple_path = os.path.join(testing_dir,'perf_ntuple.root')
    if not os.path.isfile(test_ntuple_path): 
        training.run_test_ntuple(input_weights = weights_path, 
                                 input_dataset = rds_path, 
                                 output_file_name = test_ntuple_path, 
                                 print_and_exit = do_test)
    
    rej_hist_path = os.path.join(testing_dir, 'performance_hists') 
    if not os.path.isdir(rej_hist_path): 
        os.mkdir(rej_hist_path)


    if not do_test: 
        all_canvas = rejection.make_plots_from(test_ntuple_path)
        
        formats = ['.pdf','.png']

        try: 
            import AtlasStyle
        except ImportError: 
            warn('could not import AtlasStyle', UglyWarning)

        for ext in formats: 
            for plot in all_canvas: 
                fullname = plot.GetName() + ext
                fullpath = os.path.join(rej_hist_path,fullname)
                print 'printing to %s' % fullpath
                plot.Print(fullpath)

    
if __name__ == '__main__': 
    if len(sys.argv) == 1: 
        sys.exit(__doc__)

    elif len(sys.argv) == 2: 
        input_files = []
        with open(sys.argv[1]) as file_list: 
            for line in file_list: 
                input_files.append(line.strip())


        run_full_chain(input_files, do_test = False)
