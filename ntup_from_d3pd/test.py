#!/usr/bin/env python2.7
from jetnet import ctuple_maker
import sys, os

GeV = 1e3

limits = { 
    'min_pt': 30*GeV, 
    'max_pt': 200*GeV, 
    'max_abs_eta': 2.5, 
    }
flags = 'ufm'

if __name__ == '__main__': 
    ctuple_maker.ntup_from_d3pd(
        sys.argv[1:], 'btagd3pd', 'antikt4lctopo', 'out_tuple.root', 
        limits, flags)
