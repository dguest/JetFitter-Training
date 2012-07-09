#!/usr/bin/env python2.7

# from jetnet.perf import reject
from matplotlib import pyplot as plt
import numpy as np
import argparse, sys, re, cPickle, os
from collections import defaultdict

def draw_job_comparison(ratio, hist_dict, eff, logx = False): 
    tt = (r'${num}$-jet tagging ${den}$ rejection'
          r' ($w_{num} / w_{den}$) with' 
          r' $\epsilon$ = {eff:.2}')
    full_title = tt.format(num = ratio[0], den = ratio[1], eff = eff) 

    # plt_args = []

    y_maxes = []
    y_mins = []
    plots_and_legs = []
    fig = plt.figure()
    ax = fig.add_subplot(111)
    for hist_namer, color in zip(hist_dict,['+' + x for x in 'kbrgm']): 
        x, y = hist_dict[hist_namer]
        leg_str = hist_namer.split('{}')[-1]

        if logx: 
            p = ax.semilogx(x,y,color, label = leg_str)[0]
        else: 
            p = ax.plot(x,y,color, label = leg_str)[0]

        plots_and_legs.append((p,leg_str))
        y_maxes.append(max(y))
        y_mins.append(min(y))
            
    things = zip(*plots_and_legs)
    # print things, plots_and_legs
    leg = ax.legend(*things)
    leg.get_frame().set_alpha(0.5)
    # leg.draw_frame(False)
    ax.set_title(full_title)
    big_max = max(y_maxes)
    big_min = min(y_mins)
    y_range =  big_max - big_min
    x_range = max(x) - min(x)
    ax.axis([min(x),max(x),
              big_min - 0.1 * y_range, 
              big_max + 0.1 * y_range])
    if not logx: 
        ax.set_aspect(x_range / y_range * 2 / 3)
    ax.set_xlabel('job number [arb]')
    ax.set_ylabel('rejection')
    s_name = '{}_over_{}_e{e:.0f}.pdf'.format(*ratio,e = eff * 100)
    save_path = os.path.join(args.save_dir,s_name)
    print 'saving {}'.format(save_path)
    fig.savefig( save_path )
    plt.close()

if __name__ == '__main__': 

    parser = argparse.ArgumentParser()
    parser.add_argument('input_pickles', nargs = '+')

    parser.add_argument('--save-dir', default = '')
    parser.add_argument('--logx', action = 'store_true')
    args = parser.parse_args(sys.argv[1:])

    pickle_contents = {}
    for pickle in args.input_pickles: 
        with open(pickle,'rb') as p: 
            pickle_contents[pickle] = cPickle.load(p)

    if args.save_dir and not os.path.isdir(args.save_dir): 
        os.mkdir(args.save_dir)

    required_eff_points = []
    rej_by_file = {}
    bounds_by_file = {}
    for pickle, pk_contents in pickle_contents.iteritems(): 
        eff_points = pk_contents['eff_points']
        if not required_eff_points: 
            required_eff_points = eff_points
        else: 
            # TODO: make this check more robust
            if not required_eff_points == eff_points: 
                raise IOError('eff point mismatch')

        if 'rejections' in pk_contents: 
            rej_by_file[pickle] = pk_contents['rejections']
        if 'rej_minmax' in pk_contents: 
            bounds_by_file[pickle] = pk_contents['rej_minmax']
        
    for eff_index, eff in enumerate(required_eff_points): 
        ratio_name_dict = defaultdict(dict)
        for pickle, rejections in rej_by_file.iteritems(): 
            for (ratio,hist_name), points in rejections.iteritems(): 
                xy = ((p[0],p[1][eff_index]) for p in sorted(points.items()))
                x, y = zip(*xy)
                ext_name = '{} {}'.format(hist_name, pickle.split('.')[0])
                ratio_name_dict[ratio][ext_name] = (x,y)

        for ratio, hist_dict in ratio_name_dict.iteritems():
            draw_job_comparison(ratio, hist_dict, eff, logx = args.logx)
        
