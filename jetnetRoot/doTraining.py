#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

import sys, dl, os, glob
from jetnet.dirs import OverwriteError

def _set_linker_flags(): 
    """without this root has trouble"""
    binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
    sys.setdlopenflags(binding_rules)

def run_training(jet_collection_name, in_path, 
                output_dir = None, 
                with_ip3d = True, nodes = None, 
                debug = False): 
    _set_linker_flags()

    import pynn

    class_name = 'JetFitterNN_' + jet_collection_name

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
                 output_class = class_name, 
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

    _set_linker_flags()
    
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

             
             
if __name__ == '__main__': 
    from optparse import OptionParser, OptionGroup

    usage = 'usage: %prog <reduced data set> [options]'
    description = 'runs neural net training on <reduced data set>'

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        with_ip3d = True, 
        print_and_exit = False, 
        overwrite_training = False
        )
    
    optional = OptionGroup(parser,'options')
    optional.add_option('-o',dest = 'output_path', 
                        help = "[default: %default]", 
                        default = 'weights')
    optional.add_option('-j',dest = 'jet_collection', 
                        help = "used name output"
                        " [default: %default]", 
                        default = 'AntiKt4TopoEMJets')
    optional.add_option('-i','--ip3d', dest = 'with_ip3d', 
                        action = 'store_true', 
                        help = 'use ip3d [default]')
    optional.add_option('-m','--no-ip3d', dest = 'with_ip3d', 
                        action = 'store_false', 
                        help = "don't use ip3d")
    optional.add_option('-r', dest = 'overwrite_training', 
                        help = 'overwrite training ' 
                        ' (by default skips training if weights are found)', 
                        action = 'store_true')
    optional.add_option('-1', dest = 'nodes_1', 
                        help = 'nodes in first hidden layer'
                        ' [default: %default]', type = 'int', 
                        default = 20)
    optional.add_option('-2', dest = 'nodes_2', 
                        help = 'nodes in second hidden layer'
                        ' [default: %default]', type = 'int', 
                        default = 10)
    optional.add_option('--hist-file', 
                        help = 'name of performance hist file'
                        ' [default: %default]', 
                        default = 'hists/performance_plots.root')
    parser.add_option_group(optional)

    debug_opts = OptionGroup(parser,'debug options')
    debug_opts.add_option('--print-and-exit', action = 'store_true', 
                          help = "don't run training, just print"
                          " some info to confirm that c++ works")
    parser.add_option_group(debug_opts)

    (options, args) = parser.parse_args()

    if len(args) != 1: 
        parser.print_help()
        sys.exit('ERROR: wrong number of inputs')

    weights_file_path = os.path.join(options.output_path,
                                     'weightMinimum.root')

    do_training = (
        options.overwrite_training or 
        not os.path.isfile(weights_file_path) 
        )

    if do_training: 
        run_training(jet_collection_name = options.jet_collection, 
                     in_path = args[0], 
                     output_dir = options.output_path, 
                     with_ip3d = options.with_ip3d, 
                     nodes = (options.nodes_1, options.nodes_2), 
                     debug = options.print_and_exit)
    
    run_performance(input_file = args[0], weights_file = weights_file_path, 
            output_file = options.hist_file, 
            with_ip3d = options.with_ip3d, 
            print_and_exit = options.print_and_exit)

