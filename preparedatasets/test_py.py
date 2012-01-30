#!/usr/bin/env python2.6

from glob import glob
from pyprep import prep_ntuple

files = glob('files/*.root')[:4]

prep_ntuple(files, 
            ['IP3D','SV1'])
