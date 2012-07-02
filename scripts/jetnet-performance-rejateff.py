#!/usr/bin/env python2.7

from jetnet.perf import reject
from matplotlib import pyplot as plt
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict

class RejArgs(dict): 
    """
    wrapper class for dictionary to be fed to get_rejection_at
    """
    _sig_bkg_re = re.compile('log([A-Z])([a-z])')
    _expander = {x[0] : x for x in ['bottom','charm']}
    _expander['u'] = 'light'
    def __init__(self, basename, signal = '' , background = ''): 
        if not signal: 
            sig_char = self._sig_bkg_re.search(basename).group(1)
            signal = self._expander[sig_char.lower()]
        if not background: 
            bkg_char = self._sig_bkg_re.search(basename).group(2)
            background = self._expander[bkg_char]
            
        self['basename'] = basename
        self['signal'] = signal
        self['background'] = background

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
    parser.add_argument('--save-dir', default = '')
    args = parser.parse_args(sys.argv[1:])

    ratios = [x + y for x in 'cb' for y in 'ucb' if x != y]
    hist_info = {}
    for r in ratios: 
        for n in args.hist_name: 
            init_string = n.format(r[0].upper() + r[1])
            hist_info[(r,n)] = RejArgs(init_string) 
    
    if os.path.isfile(args.p): 
        with open(args.p,'rb') as p: 
            rejections = cPickle.load(p)
    else: 
        rejections = defaultdict(dict)

    jobfinder = re.compile(args.subjob_prefix + '([0-9]+)')

    for root_file in args.input_files: 
        subjob = int(jobfinder.search(root_file).group(1))

        if not all(subjob in rejections[x] for x in hist_info): 
            print 'subjob {} cache not found, making'.format(subjob)
            from ROOT import TFile
            root_file = TFile(root_file)
            for sig, a in hist_info.items(): 
                rej_points = reject.get_rejection_at(
                    root_file = root_file, 
                    points = args.eff_points, 
                    **a)
                rejections[sig][subjob] = rej_points

    with open(args.p,'wb') as p: 
        cPickle.dump(rejections,p)

    if args.save_dir and not os.path.isdir(args.save_dir): 
        os.mkdir(args.save_dir)

    for eff_index, eff in enumerate(args.eff_points): 
        ratio_name_dict = defaultdict(dict)
        for (ratio,hist_name), points in rejections.iteritems(): 
            xy = ((p[0],p[1][eff_index]) for p in sorted(points.items()))
            x, y = zip(*xy)
            ratio_name_dict[ratio][hist_name] = (x,y)

        for ratio, hist_dict in ratio_name_dict.iteritems():

            tt = (r'${num}$-jet tagging ${den}$ rejection'
                  r' ($w_{num} / w_{den}$) with' 
                  r' $\epsilon$ = {eff:.2}')
            full_title = tt.format(num = ratio[0], den = ratio[1], eff = eff) 

            # plt_args = []

            y_maxes = []
            y_mins = []
            plots_and_legs = []
            for hist_namer, color in zip(args.hist_name,['+k','+b']): 
                x, y = hist_dict[hist_namer]
                leg_str = hist_namer.split('{}')[-1]
                p = plt.plot(x,y,color, label = leg_str)[0]
                plots_and_legs.append((p,leg_str))
                y_maxes.append(max(y))
                y_mins.append(min(y))
            
            things = zip(*plots_and_legs)
            # print things, plots_and_legs
            plt.legend(*things)
            plt.title(full_title)
            big_max = max(y_maxes)
            big_min = min(y_mins)
            y_range =  big_max - big_min
            plt.axis([min(x),max(x),
                      big_min - 0.1 * y_range, 
                      big_max + 0.1 * y_range])
            plt.xlabel('job number [arb]')
            plt.ylabel('rejection')
            s_name = '{}_over_{}_e{e:.0f}.pdf'.format(*ratio,e = eff * 100)
            save_path = os.path.join(args.save_dir,s_name)
            print 'saving {}'.format(save_path)
            plt.savefig( save_path )
            plt.close()
