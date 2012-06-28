import sys, os, glob
from jetnet.dirs import OverwriteError
import pynn


def _get_pt_eta_categories(file_name): 
    from ROOT import TFile, TIter

    if not os.path.isfile(file_name): 
        return None, None
    
    the_file = TFile(file_name)
    pt_categories = []
    eta_categories = []
    file_keys = the_file.GetListOfKeys()
    for key in TIter(file_keys): 
        key_name = key.GetName()
        key_class = key.GetClassName()

        if not key_class == 'TTree': continue
        
        if key_name == 'pt_cat': 
            tree = key.ReadObj()
            for entry in tree: 
                pt_categories.append(entry.pt_gev)

        elif key_name == 'eta_cat': 
            tree = key.ReadObj()
            for entry in tree: 
                eta_categories.append(entry.abs_eta)
            
    return pt_categories, eta_categories


def run_training(reduced_dataset, 
                 output_directory,
                 flavor_weights, 
                 normalization, 
                 nodes = None, 
                 debug = False, 
                 events = 1000000): 

    if not os.path.isdir(output_directory): 
        os.mkdir(output_directory)
    elif glob.glob(output_directory + '/*.root*'): 
        raise OverwriteError('root files found in %s' % output_directory)

    if nodes is None:
        nodes = (20, 10)

    flags = 't'

    if debug: flags += 'd'

    pynn.trainNN(reduced_dataset = reduced_dataset, 
                 output_directory = output_directory, 
                 n_iterations = 10000, 
                 normalization = normalization, 
                 nodes = nodes, 
                 flavor_weights = flavor_weights, 
                 n_training_events_target = events, 
                 flags = flags)


def run_performance(reduced_dataset, weights_file, 
            output_file = 'hists/performance_plots.root', 
            with_ip3d = True, 
            print_and_exit = False): 


    output_dir = os.path.dirname(output_file)
    if not os.path.isdir(output_dir): 
        os.makedirs(output_dir)

    pynn.testNN(reduced_dataset = reduced_dataset, 
                weights_file = weights_file, 
                # dilution_factor = 2, 
                use_sd = False, 
                with_ip3d = with_ip3d, 
                debug = print_and_exit, 
                output_file = output_file) 


def run_test_ntuple(reduced_dataset, 
                    weights_file, 
                    output_file, 
                    output_tree = 'performance', 
                    print_and_exit = False): 

    output_dir = os.path.dirname(output_file)
    if not os.path.isdir(output_dir): 
        os.makedirs(output_dir)

    pt_categories, eta_categories = _get_pt_eta_categories(reduced_dataset)

    if print_and_exit: 
        pt_categories = [1.0,20.3]
        eta_categories = [0.0,1.0,3.5]

    pynn.makeNtuple(weights_file = weights_file, 
                    reduced_dataset = reduced_dataset,
                    output_file = output_file, 
                    output_tree = output_tree, 
                    debug = print_and_exit)
    
