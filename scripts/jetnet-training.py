#!/usr/bin/env python2.7
# Author: Daniel Guest (dguest@cern.ch)

import sys, os
from jetnet import training as tr
             
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
    optional.add_option('-w',dest = 'output_path', 
                        help = "weights path [default: %default]", 
                        default = 'weights')
    optional.add_option('--perf-hist-file', dest = 'hist_file',
                        help = 'name of performance hist file'
                        ' [default: %default]', 
                        default = 'performance/performance_plots.root')
    optional.add_option('--perf-ntuple', dest = 'perf_ntuple', 
                        help = 'name of performance ntuple'
                        ' [default: %default]', 
                        default = 'performance/performance.root')
    parser.add_option_group(optional)

    debug_opts = OptionGroup(parser,'debug options')
    debug_opts.add_option('--print-and-exit', action = 'store_true', 
                          help = "don't run training, just make dirs and"
                          " some info to confirm that c++ works")
    parser.add_option_group(debug_opts)

    (options, args) = parser.parse_args()

    if len(args) != 1: 
        parser.print_help()
        sys.exit('ERROR: wrong number of inputs')

    weights_file_path = os.path.join(options.output_path,
                                     'weightMinimum.root')

    input_ds = args[0]

    if not os.path.isfile(input_ds): 
        sys.exit('ERROR: %s not found' % input_ds)

    # --- run stuff 
    do_training = (
        options.overwrite_training or 
        not os.path.isfile(weights_file_path) 
        )

    if do_training: 
        tr.run_training(reduced_dataset = input_ds, 
                        output_directory = options.output_path, 
                        with_ip3d = options.with_ip3d, 
                        nodes = (options.nodes_1, options.nodes_2), 
                        debug = options.print_and_exit)
    
    tr.run_performance(reduced_dataset = input_ds, 
                       weights_file = weights_file_path, 
                       output_file = options.hist_file, 
                       with_ip3d = options.with_ip3d, 
                       print_and_exit = options.print_and_exit)

    tr.run_test_ntuple(weights_file = weights_file_path, 
                       reduced_dataset = input_ds, 
                       output_file = options.perf_ntuple, 
                       print_and_exit = options.print_and_exit)
