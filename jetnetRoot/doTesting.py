#!/usr/bin/env python2.7
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

with_ip3d = True
# if len(sys.argv) > 2: 
#     if 'True' in sys.argv[2:]: 
#         with_ip3d = True

trained_nn = 'ip3d_weights/weightMinimum.root'
if not os.path.isfile(trained_nn): 
    sys.exit("ERROR: no file %s found" % trained_nn)

out_path = 'draw/all_plots.root'
out_dir, out_name  = os.path.split(out_path)
if out_dir and not os.path.isdir(out_dir): 
    os.mkdir(out_dir)

full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name

if not os.path.isfile(full_path): 
    sys.exit('no file to read!')

pynn.testNN(input_file = full_path, 
            trained_nn_file = trained_nn, 
            # dilution_factor = 2, 
            use_sd = False, 
            with_ip3d = with_ip3d, 
            debug = False, 
            out_file = out_path) 

