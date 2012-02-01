import sys, dl, os, glob
from jetnet.dirs import OverwriteError


def _set_linker_flags(): 
    """without this root has trouble"""
    binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
    sys.setdlopenflags(binding_rules)

def run_training(in_path, 
                 output_dir = None, 
                 with_ip3d = True, nodes = None, 
                 debug = False): 
    # _set_linker_flags()

    import pynn

    if not os.path.isdir(output_dir): 
        os.mkdir(output_dir)
    elif glob.glob(output_dir + '/*.root*'): 
        raise OverwriteError('root files found in %s' % output_dir)

    if nodes is None:
        if with_ip3d: 
            nodes = (20, 10)
        else: 
            nodes = (15,  8)
    nodes_first_layer, nodes_second_layer = nodes

    pynn.trainNN(input_file = in_path, 
                 n_iterations = 10000, 
                 dilution_factor = 2, 
                 use_sd = False, 
                 with_ip3d = with_ip3d, 
                 nodes_first_layer = nodes_first_layer, 
                 nodes_second_layer = nodes_second_layer, 
                 debug = debug, 
                 output_dir = output_dir)


def run_performance(input_file, weights_file, 
            output_file = 'hists/performance_plots.root', 
            with_ip3d = True, 
            print_and_exit = False): 

    # _set_linker_flags()
    
    import pynn

    output_dir = os.path.dirname(output_file)
    if not os.path.isdir(output_dir): 
        os.makedirs(output_dir)

    pynn.testNN(input_file = input_file, 
                trained_nn_file = weights_file, 
                # dilution_factor = 2, 
                use_sd = False, 
                with_ip3d = with_ip3d, 
                debug = print_and_exit, 
                out_file = output_file) 


def run_test_ntuple(input_weights, 
                    input_dataset, 
                    output_file_name, 
                    output_tree_name = 'performance'): 

    import pynn

    output_dir = os.path.dirname(output_file_name)
    if not os.path.isdir(output_dir): 
        os.makedirs(output_dir)


    pynn.make_test_ntuple(input_weights = input_weights, 
                          input_dataset = input_dataset, 
                          output_file_name = output_file_name, 
                          output_tree_name = output_tree_name)
                    
