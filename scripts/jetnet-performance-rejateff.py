#!/usr/bin/env python2.7

from jetnet.perf import reject
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict


if __name__ == '__main__': 

    parser = argparse.ArgumentParser()
    parser.add_argument('input_files', nargs = '+')
    parser.add_argument('-e', dest = 'eff_points', nargs = '+', 
                        default = [0.2,0.5,0.8], type = float, 
                        help = 'default: %(default)s')
    parser.add_argument('-p', default = 'eff_vs_rej.pkl', 
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

    for root_file in args.input_files: 
        subjob = int(jobfinder.search(root_file).group(1))

        print 'making plots for subjob {}'.format(subjob)
        
        from ROOT import TFile
        root_file = TFile(root_file)
        for sig, a in hist_info.items(): 
            rej_points = reject.get_rejection_at(
                root_file = root_file, 
                points = args.eff_points, 
                **a)
            rejections[sig][subjob] = rej_points

    with open(args.p,'wb') as p: 
        # this should be dict-ified
        pkl_thing = { 
            'rejections': rejections, 
            'eff_points': args.eff_points, 
            }
        cPickle.dump(pkl_thing,p)

