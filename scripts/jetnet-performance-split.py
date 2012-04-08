#!/usr/bin/env python2.6
"""
Author: Daniel Guest (dguest@cern.ch)
"""

from jetnet.perf import splittree
from ROOT import TFile
import sys

if __name__ == '__main__': 
    values = [15,30,100]
    
    splittree.split_tree(sys.argv[1], values = values)
