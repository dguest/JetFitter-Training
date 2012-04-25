#!/usr/bin/env python2.7

import os
from glob import glob
from pyprep import prep_ntuple

data_path = os.path.expandvars('$DATA/jetnet')
files = glob(data_path + '/user.dguest**/*.root*')

assert files

prep_ntuple(input_files = files[0:3], 
            observer_discriminators = ['IP2D','IP3D','SV1','COMB'],
            output_file = 'with_write.root', 
            debug = True)
