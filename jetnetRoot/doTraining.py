#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

import sys, dl, os, glob

# without this root has trouble
binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
sys.setdlopenflags(binding_rules)
import pynn

# **** should be config-based *****
input_ds = 'AntiKt4TopoEMJets'

if len(sys.argv) > 1: 
    input_ds = sys.argv[1]

with_ip3d = False
if len(sys.argv) > 2: 
    if 'True' in sys.argv[2:]: 
        with_ip3d = True

out_dir = 'weights'
if not os.path.isdir(out_dir): 
    os.mkdir(out_dir)
elif glob.glob(out_dir + '/*'): 
    sys.exit("files found in 'weights', "
             "remove them (or the directory) to continue")


full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name
class_name = 'JetFitterNN_' + input_ds

pynn.trainNN(input_file = full_path, 
             output_class = class_name, 
             n_iterations = 10000, 
             dilution_factor = 2, 
             use_sd = False, 
             with_ip3d = with_ip3d, 
             nodes_first_layer = 10, 
             nodes_second_layer = 6, 
             debug = False)
             
