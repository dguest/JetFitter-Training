#!/usr/bin/env python2.7

"""
runs over a list of histogram files, calculates efficiency vs rejection
curves. 
The min and max values for each efficiency point are stored in a .pkl file
"""
_author = 'Dan Guest <dguest@cern.ch>'

from jetnet.perf import reject
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict

from matplotlib import pyplot as plt


if __name__ == '__main__': 

    parser = argparse.ArgumentParser(
        description = __doc__, 
        epilog = 'author: ' + _author )
    parser.add_argument('input_files', nargs = '+')
    parser.add_argument('-p', default = 'rej_range.pkl', 
                        help = 'default: %(default)s')
    parser.add_argument('--hist-name', default = ['log{}NewTune'], 
                        nargs = '+', 
                        help = 'default: %(default)s')
    parser.add_argument('--subjob-prefix', default = 'subjob', 
                        help = 'default: %(default)s')
    args = parser.parse_args(sys.argv[1:])

    ratios = [x + y for x in 'cb' for y in 'ucb' if x != y]
    hist_info = {}
    for r in ratios: 
        for n in args.hist_name: 
            init_string = n.format(r[0].upper() + r[1])
            hist_info[(r,n)] = reject.RejArgs(init_string) 
    
    rejections = defaultdict(dict)

    jobfinder = re.compile(args.subjob_prefix + '([0-9]+)')

    eff_points = np.arange(0.1,0.9,0.01)

    for root_file in args.input_files: 
        subjob = int(jobfinder.search(root_file).group(1))

        print 'making plots for subjob {}'.format(subjob)
        
        from ROOT import TFile
        root_file = TFile(root_file)
        for sig, a in hist_info.items(): 
            rej_points = reject.get_rejection_at(
                root_file = root_file, 
                points = eff_points, 
                **a)
            rejections[sig][subjob] = rej_points

    rej_minmax = {}
    for idx_tuple, subjobs in rejections.iteritems(): 

        j_vals = defaultdict(list)
        for eff_idx, eff_pt in enumerate(eff_points): 
            for rej_points in subjobs.values(): 
                the_rejection = rej_points[eff_idx]
                j_vals[eff_idx].append(the_rejection)
            
        min_vals = np.fromiter((min(j) for j in j_vals.values()),'d')
        max_vals = np.fromiter((max(j) for j in j_vals.values()),'d')

        rej_minmax[idx_tuple] = (min_vals, max_vals)


    with open(args.p,'wb') as p: 
        # this should be dict-ified
        pkl_thing = { 
            'rej_minmax': rej_minmax, 
            'eff_points': eff_points, 
            }
        cPickle.dump(pkl_thing,p)

