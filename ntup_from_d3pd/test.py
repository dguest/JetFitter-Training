#!/usr/bin/env python2.7
from jetnet import ctuple_maker
import sys, os

if __name__ == '__main__': 
    ctuple_maker.ntup_from_d3pd(
        sys.argv[1:], 'btagd3pd', 'antikt4lctopo', 'out_tuple.root')
