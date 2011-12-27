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

# with_ip3d = False
# if len(sys.argv) > 2: 
#     if set(['True','ip3d']) & set(sys.argv[2:]): 
#         print 'using ip3d'
#         with_ip3d = True

# out_dir = 'weights'


full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name
class_name = 'JetFitterNN_' + input_ds

for out_dir, ip3d_state in [('ip3d_test_weights',True)]:

    if not os.path.isdir(out_dir): 
        os.mkdir(out_dir)
    elif glob.glob(out_dir + '/*'): 
        print "files found in %s, skipping" % out_dir
        continue


    pynn.trainNN(input_file = full_path, 
                 output_class = class_name, 
                 n_iterations = 10000, 
                 dilution_factor = 200, 
                 use_sd = False, 
                 with_ip3d = ip3d_state, 
                 nodes_first_layer = 10, 
                 nodes_second_layer = 6, 
                 debug = False, 
                 output_dir = out_dir)
             
