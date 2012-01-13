#!/usr/bin/env python 
# Author: Daniel Guest (dguest@cern.ch)

import sys, dl, os, glob
from jetnet.dirs import OverwriteError

def do_training(jet_collection_name, in_path, 
                output_dir = None, 
                with_ip3d = True, nodes = None, 
                debug = False): 
    # without this root has trouble
    binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
    sys.setdlopenflags(binding_rules)
    import pynn

    class_name = 'JetFitterNN_' + jet_collection_name

    if not os.path.isdir(output_dir): 
        os.mkdir(output_dir)
    elif glob.glob(output_dir + '/*.root*'): 
        raise OverwriteError('root files found in %s' % out_dir)

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
             
             
if __name__ == '__main__': 
    from optparse import OptionParser, OptionGroup

    parser = OptionParser()
    parser.set_defaults(
        with_ip3d = True, 
        nodes_1 = 20, 
        nodes_2 = 10, 
        debug = False
        )
    required = OptionGroup(parser,'required options')
    required.add_option('-t','--training-file', 
                      help = 'a reduced data set')
    required.add_option('-o','--output-path')
    parser.add_option_group(required)
    
    optional = OptionGroup(parser,'optional options')
    optional.add_option('-j',dest = 'jet_collection', 
                        help = "used to find in input ds in root file.\n"
                        "Also to name output"
                        " [default: %default]", 
                        default = 'AntiKt4TopoEMJets')
    optional.add_option('-i','--ip3d', dest = 'with_ip3d', 
                      action = 'store_true', 
                      help = 'use ip3d [default]')
    optional.add_option('-m','--no-ip3d', dest = 'with_ip3d', 
                      action = 'store_false', 
                      help = "don't use ip3d")
    optional.add_option('-1', dest = 'nodes_1', 
                      help = 'nodes in first hidden layer'
                      ' [default: %default]', type = 'int')
    optional.add_option('-2', dest = 'nodes_2', 
                      help = 'nodes in second hidden layer'
                      ' [default: %default]', type = 'int')
    parser.add_option_group(optional)

    debug_opts = OptionGroup(parser,'debug options')
    debug_opts.add_option('-d','--debug', action = 'store_true', 
                          help = "don't run training, just test")
    parser.add_option_group(debug_opts)

    (options, args) = parser.parse_args()

    do_training(jet_collection_name = options.jet_collection, 
                in_path = options.training_file, 
                output_dir = options.output_path, 
                with_ip3d = options.with_ip3d, 
                nodes = (options.nodes_1, options.nodes_2), 
                debug = True)
