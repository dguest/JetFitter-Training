#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

import sys, dl, os, glob

# without this root has trouble
# binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
# sys.setdlopenflags(binding_rules)
import pynn


if len(sys.argv) > 1: 
    reduced_dataset = sys.argv[1]

else: 
    sys.exit('give a ds to run on') 

output_directory = 'test' 
             
# pynn.trainNN(reduced_dataset = reduced_dataset, 
#              output_directory = output_directory, 
#              n_iterations = 10, 
#              dilution_factor = 2, 
#              use_sd = False, 
#              with_ip3d = True, 
#              nodes = (10,10), 
#              debug = False)

pynn.makeNtuple(weights_file = output_directory + '/weightMinimum.root', 
                reduced_dataset = reduced_dataset, 
                output_file = output_directory + '/test_ntuple.root')
