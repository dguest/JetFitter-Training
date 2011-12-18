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

trained_nn = 'weights/weightMinimum.root'
if not os.path.isfile(trained_nn): 
    sys.exit("ERROR: no file %s found" % trained_nn)


full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name

pynn.testNN(input_file = full_path, 
            trained_nn_file = trained_nn, 
            # dilution_factor = 2, 
            use_sd = False, 
            with_ip3d = with_ip3d, 
            debug = False) 

