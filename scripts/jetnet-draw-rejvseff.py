#!/usr/bin/env python2.7

from matplotlib import pyplot as plt
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict

if __name__ == '__main__': 

    parser = argparse.ArgumentParser()
    parser.add_argument('input_pickles', nargs = '+')

    parser.add_argument('--save-dir', default = 'plots', 
                        help = 'default: %(default)s')
    parser.add_argument('-r', default = 'cu', 
                        help = 'ratio to plot (default: %(default)s)')
    parser.add_argument('--logy', action = 'store_true')
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

    for pickle, pk_contents in pickle_contents.iteritems(): 
        if not 'eff_points' in pk_contents: 
            for thing in pickle_contents: print thing
            sys.exit('ACHTUNG: Steinbuck!')
        
        if not 'rej_minmax' in pk_contents: 
            if not 'rejection_vals' in pickle_contents: 
                sys.exit('ACHTUNG: you are nothing without your values') 

        eff_points = pk_contents['eff_points']
        
        for id_tuple, y_series in pk_contents['rej_minmax'].items(): 
            if id_tuple[0].lower() != args.r: 
                continue

            plot_color = next(colors)
            p = ax.fill_between(eff_points, y_series[0], y_series[1], 
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
