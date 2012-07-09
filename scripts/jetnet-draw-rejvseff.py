#!/usr/bin/env python2.7

"""
draws rejection plots stored in a .pkl file. 

Expects 'eff_points' and 'rej_minmax' in the pickle. 
"""
_author = 'Dan Guest <dguest@cern.ch>'


from matplotlib import pyplot as plt
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict

_rejmmkey = 'rej_minmax'

if __name__ == '__main__': 

    parser = argparse.ArgumentParser(description = __doc__ , 
                                     epilog = 'author: ' + _author)
    parser.add_argument('input_pickles', nargs = '+')

    parser.add_argument('--save-dir', default = 'plots', 
                        help = 'default: %(default)s')
    parser.add_argument('-r', default = 'cu', 
                        help = 'ratio to plot (default: %(default)s)')
    parser.add_argument('-l,--logy', action = 'store_true', dest = 'logy')
    parser.add_argument('--baseline', 
                        help = 'use this as baseline if found', 
                        default = '')
    args = parser.parse_args(sys.argv[1:])

    pickle_contents = {}
    for pickle in args.input_pickles: 
        with open(pickle,'rb') as p: 
            pickle_contents[pickle] = cPickle.load(p)

    if args.save_dir and not os.path.isdir(args.save_dir): 
        os.mkdir(args.save_dir)


    fig = plt.figure()
    ax = fig.add_subplot(111)
    if args.logy: 
        ax.set_yscale('log')

    colors = iter(['red','green','blue'])

    plot_lines = []
    plot_names = []

    baseline_x = np.zeros(0)
    baseline_y = np.zeros(0)
        
    if args.baseline: 
        for pickle, pk_contents in pickle_contents.iteritems(): 
            rej_pts_dict = pk_contents[_rejmmkey]
            id_keys = rej_pts_dict.keys()
            for key in id_keys: 
                ratio, tagger = key
                if ratio == args.r and args.baseline in tagger: 
                    if baseline_y: 
                        sys.exit('ACHTUNG: doppelsteinbuck')
                    hi, low = rej_pts_dict[key]
                    baseline_y = (hi + low) / 2
                    baseline_x = pk_contents['eff_points']

    for pickle, pk_contents in pickle_contents.iteritems(): 
        if not 'eff_points' in pk_contents: 
            for thing in pickle_contents: print thing
            sys.exit('ACHTUNG: Steinbuck!')
        
        if not _rejmmkey in pk_contents: 
            if not 'rejection_vals' in pickle_contents: 
                sys.exit('ACHTUNG: you are nothing without your values') 

        eff_points = pk_contents['eff_points']
        
        for id_tuple, y_series in pk_contents[_rejmmkey].items(): 
            if id_tuple[0].lower() != args.r: 
                continue

            # skip baseline
            if args.baseline in id_tuple[1]: 
                continue

            plot_color = next(colors)
            
            y_min, y_max = y_series
            if np.any(baseline_y): 
                y_min_ratio = []
                y_max_ratio = []
                for yl, yh, base_y in zip(y_min, y_max, baseline_y): 
                    y_min_ratio.append(yl / base_y)
                    y_max_ratio.append(yh / base_y)

                y_min = y_min_ratio
                y_max = y_max_ratio

            p = ax.fill_between(eff_points, y_min, y_max, 
                                alpha = 0.5, facecolor = plot_color, 
                                edgecolor = plot_color, 
                                )
            label = id_tuple[1].split('{}')[-1]
            
            proxy = plt.Rectangle((0, 0), 1, 1, fc = plot_color)

            plot_lines.append(proxy)
            plot_names.append(label)

    ax.legend(plot_lines, plot_names)
    ax.set_xlabel('efficiency')
    ax.set_ylabel('${}$-jet rejection'.format(args.r[1].replace('u','l')))
    fig.savefig( 'test.pdf' )
