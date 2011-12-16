#!/usr/bin/env python2.6
import sys, dl

# without this root has trouble
sys.setdlopenflags(dl.RTLD_NOW | dl.RTLD_GLOBAL)
import pynn

input_ds = 'AntiKt4TopoEMJets'

if len(sys.argv) > 1: 
    input_ds = sys.argv[1]

with_ip3d = False

if len(sys.argv) > 2: 
    if 'True' in sys.argv[2:]: 
        with_ip3d = True

full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name
class_name = 'JetFitterNN_' + input_ds

pynn.trainNN(input_file = full_path, 
             output_class = class_name, 
             n_iterations = 1000, 
             dilution_factor = 200, 
             use_sd = False, 
             with_ip3d = with_ip3d, 
             nodes_first_layer = 10, 
             nodes_second_layer = 4, 
             debug = False)
             
