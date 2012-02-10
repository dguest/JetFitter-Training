import sys, dl, os, glob
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


def _set_linker_flags(): 
    """without this root has trouble"""
    binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
    sys.setdlopenflags(binding_rules)

def run_training(reduced_dataset, 
                 output_directory,
                 with_ip3d = True, nodes = None, 
                 debug = False): 

    if not os.path.isdir(output_directory): 
        os.mkdir(output_directory)
    elif glob.glob(output_directory + '/*.root*'): 
        raise OverwriteError('root files found in %s' % output_directory)

    if nodes is None:
        if with_ip3d: 
            nodes = (20, 10)
        else: 
            nodes = (15,  8)

    pynn.trainNN(reduced_dataset = reduced_dataset, 
                 output_directory = output_directory, 
                 n_iterations = 10000, 
                 dilution_factor = 2, 
                 use_sd = False, 
                 with_ip3d = with_ip3d, 
                 nodes = nodes, 
                 debug = debug)


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
                    pt_categories = pt_categories, 
                    eta_categories = eta_categories, 
                    debug = print_and_exit)
    
