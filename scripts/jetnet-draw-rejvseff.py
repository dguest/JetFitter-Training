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
    parser.add_argument('-l','--logy', action = 'store_true', dest = 'logy')
    parser.add_argument('--baseline', 
                        help = 'use this as baseline if found', 
                        default = '')
    parser.add_argument('-t','--output-type', default = '.pdf', 
                        choices = ['.pdf','.png','.svg'], 
                        help = 'default %(default)s ')
    parser.add_argument('--leg', default = 'best', 
                        choices = [
            'upper left', 'upper right', 'lower left', 'lower right', 'best'], 
                        help = 'legend location, default: %(default)s ')
    parser.add_argument('--xlim', nargs = 2, type = float, default = None, 
                        dest = 'x')

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

    colors = iter(['red','green','blue','magenta','black'])

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
                    if np.any(baseline_y): 
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

            plot_color = next(colors)
            # skip baseline
            if args.baseline and args.baseline in id_tuple[1]: 
                continue

            
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
            label += ' ' + pickle.replace('-',' ').split()[0]
            
            proxy = plt.Rectangle((0, 0), 1, 1, fc = plot_color)

            plot_lines.append(proxy)
            plot_names.append(label)

    leg = ax.legend(plot_lines, plot_names, shadow = False, loc = args.leg)
    leg.get_frame().set_alpha(0.5)
    x_char = args.r[0].lower().replace('u','l')
    ax.set_xlabel('${}$-jet efficiency'.format(x_char))

    rej_flavor = args.r[1].replace('u','l')
    if args.baseline: 
        bl = args.baseline
        y_str = '${}$-jet rejection / {}'.format(rej_flavor, bl)
        out_file_name = '{}-effvsrej-over-{}{}'.format(
            args.r, bl, args.output_type)
    else: 
        y_str = '${}$-jet rejection'.format(rej_flavor)
        out_file_name = '{}-effvsrej{}'.format(args.r, args.output_type)

    ax.set_ylabel(y_str)
    ax.grid(True)
    ax.set_ymargin(0.1)
    ax.autoscale()
    if args.x: 
        ax.set_xlim(args.x[0],args.x[1])

    
    savename = os.path.join(args.save_dir, out_file_name)
    fig.savefig( savename, bbox_inches='tight' )
