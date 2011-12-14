#!/usr/bin/env python2.6

import os 
import ConfigParser
import sys

methods = ['JetNet', 'LikelihoodHistos'] 

conf = ConfigParser.ConfigParser()
conf.read(['training.config']) 
collections_to_process = conf.get('collections', 'process').split()

for m in methods: 
    method_dir_name = 'trainingResults' + m

    for jet_dir_name in collections_to_process: 
        full_dir_path = method_dir_name + '/' + jet_dir_name
        if os.path.isdir(full_dir_path): 
            sys.exit('%s already exists, remove to continue' % full_dir_path)

        else:
            os.makedirs(full_dir_path)

