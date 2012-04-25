#!/usr/bin/env python2.7
# Author: Daniel Guest (dguest@cern.ch)

import sys, dl, os, glob, ConfigParser

# without this root has trouble
binding_rules =  dl.RTLD_GLOBAL | dl.RTLD_NOW
sys.setdlopenflags(binding_rules)
import pynn

config_file = '../training.config'

config = ConfigParser.ConfigParser()
config.read([config_file])

collections_to_process = config.get('collections', 'process').split()
input_ds = collections_to_process[0]

if len(sys.argv) > 1: 
    input_ds = sys.argv[1]

# with_ip3d = False
# if len(sys.argv) > 2: 
#     if set(['True','ip3d']) & set(sys.argv[2:]): 
#         print 'using ip3d'
#         with_ip3d = True

# out_dir = 'weights'

settings = dict(config.items('net'))

full_ds_name = 'reduceddataset_%s_forNN.root' % input_ds
full_path = '../reduceddatasets/' + full_ds_name
class_name = 'JetFitterNN_' + input_ds

for out_dir, ip3d_state in [('ip3d_weights',True),('no_ip3d_weights',False)]:

    if not os.path.isdir(out_dir): 
        os.mkdir(out_dir)
    elif glob.glob(out_dir + '/*'): 
        print "files found in %s, skipping" % out_dir
        continue


    pynn.trainNN(input_file = full_path, 
                 output_class = class_name, 
                 n_iterations = int(settings['n_iterations']), 
                 dilution_factor = int(settings['dilution_factor']), 
                 use_sd = False, 
                 with_ip3d = ip3d_state, 
                 nodes_first_layer = int(settings['nodes_1']), 
                 nodes_second_layer = int(settings['nodes_2']), 
                 debug = True, 
                 output_dir = out_dir)
             
