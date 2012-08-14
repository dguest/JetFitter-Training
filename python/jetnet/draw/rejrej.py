import numpy as np
import os, sys, warnings, cPickle, itertools
import glob
import matplotlib.pyplot as plt
import matplotlib as mp
from matplotlib.gridspec import GridSpec
from matplotlib.colorbar import Colorbar

"""
routine to draw b-rejection vs c- or l-rejection plots
"""

_tags = ['bottom','charm','light']

def plot_pickle(pickle, output_name, use_contour = True): 
    with open(pickle) as pkl: 
        pdict = cPickle.load(pkl)
        _plot_eff_array(pdict, use_contour=use_contour,out_name=output_name)

def _plot_eff_array(array_dict, use_contour = True, 
                    out_name = 'rejrej.pdf'):     

    eff_array = array_dict['eff']
    x_min, x_max = array_dict['x_range']
    y_min, y_max = array_dict['y_range']

    fig = plt.figure()
    gs = GridSpec(20,21)
    ax = plt.subplot(gs[:,0:-1])
    aspect = float(x_max - x_min) / (y_max - y_min)

    im = ax.imshow(
        eff_array, 
        origin = 'upper', 
        extent = (x_min,x_max, y_min, y_max), 
        aspect = aspect, 
        interpolation = 'nearest', 
        )

    if use_contour: 
        im.set_cmap('Greys')

        ct = ax.contour(
            eff_array, 
            origin = 'upper', 
            extent = (x_min,x_max, y_min, y_max), 
            aspect = aspect, 
            linewidths = 2, 
            levels = np.arange(0.1,1.0,0.1),
            )

    ax.set_xticks([])
    ax.set_yticks([])
    ax.invert_yaxis()

    cb_ax = plt.subplot(gs[:,-1])
    plt.subplots_adjust(left = 0.25) # FIXME: use OO call here
    # cb = plt.colorbar()
    cb = Colorbar(ax = cb_ax, mappable = im)
    cb.set_label('{} efficiency'.format(array_dict['signal']))
    if use_contour: cb.add_lines(ct)
    position = ax.get_position()
    new_aspect = ax.get_aspect()
    # ax.set_position(position)
        

    ax_log = fig.add_subplot(111, frameon = False)
    ax_log.set_xscale('log')
    ax_log.set_yscale('log')
    ax_log.axis((10**x_min, 10**x_max, 10**y_min, 10**y_max))
    ax_log.set_aspect(aspect)
    ax_log.set_xlabel('{} rejection'.format(array_dict['x_bg']))
    ax_log.set_ylabel('{} rejection'.format(array_dict['y_bg']))
    ax_log.grid(True)


    ax_log.set_aspect(new_aspect)
    ax_log.set_position(position)
            
    plt.savefig(out_name, bbox_inches = 'tight')
    plt.close()

def _get_rel_plot_alignment(*plots): 
    """
    I don't really know what this returns, but it gets bigger if the plots
    aren't aligned 
    """
    total_diff = 0
    total_sum = 0
    for ax in ['x_range','y_range']: 
        ranges = [p[ax] for p in plots]
        for pt in zip(*ranges): 
            diff = (max(pt) - min(pt))**2
            add = sum(pt)**2
            total_diff += diff
            total_sum += add
    return float(total_diff) / float(total_sum)

def _check_sig_bg_match(*plots): 
    sig = plots[0]['signal']
    bgx = plots[0]['x_bg']
    bgy = plots[0]['y_bg']
    sig_match = all(sig == s['signal'] for s in plots)
    bgx_match = all(bgx == s['x_bg'] for s in plots)
    bgy_match = all(bgy == s['y_bg'] for s in plots)
    return sig_match and bgx_match and bgy_match
    

def _overlay_rejrej(array_one, array_two,
                    out_name = 'rejrej.pdf', 
                    do_rel = False, do_contour = False):     

    diff_array = array_one['eff'] - array_two['eff']

    eff_array = diff_array 

    arrays = array_one, array_two

    # TODO: this is a bit messy, should find a more robust way to mask out
    # the no-entry cells
    for a in arrays: 
        blank_cells = np.nonzero(a['eff'] < 1e-6)
        diff_array[blank_cells] = -1e3

    if do_rel: 
        eff_array = eff_array / array_two['eff']

    for a in arrays: 
        if not 'tagger' in a: 
            warnings.warn('no tagger name given',stacklevel = 2)
            a['tagger'] = 'unknown'

    if _get_rel_plot_alignment(array_one, array_two) > 1e-6: 
        ranges = [(p['x_range'], p['y_range']) for p in arrays]
        raise ValueError("ranges {} don't match {}".format(*ranges))

    if not _check_sig_bg_match(*arrays): 
        err = 'array mismatch ---'
        for a in arrays: 
            err += 'sig: {signal}, bgx: {x_bg}, bgy: {y_bg} '.format(**a)
        raise ValueError(err)

    x_min, x_max = array_one['x_range']
    y_min, y_max = array_one['y_range']

    fig = plt.figure()
    gs = GridSpec(20,21)
    ax = plt.subplot(gs[:,0:-1])
    aspect = float(x_max - x_min) / (y_max - y_min)

    # cmap = mp.colors.Colormap('rainbow')

    plt_min = np.min(eff_array[np.nonzero(eff_array > -2)])
    plt_max = np.max(eff_array[np.nonzero(eff_array > -2)]) 

    print 'plot range: {: .4f}--{:.4f}'.format(plt_min, plt_max)

    im = ax.imshow(
        eff_array, 
        origin = 'upper', 
        extent = (x_min,x_max, y_min, y_max), 
        aspect = aspect, 
        interpolation = 'nearest', 
        vmin = plt_min, 
        vmax = plt_max, 
        # cmap = cmap, 
        )

    
    c_lines = np.arange(0.05, plt_max, 0.05)
    if len(c_lines) == 0: 
        do_contour = False
    if do_contour: 
        print 'clines', c_lines
        ct = ax.contour(
            eff_array, 
            origin = 'upper', 
            extent = (x_min,x_max, y_min, y_max), 
            aspect = aspect, 
            linewidths = 2, 
            levels = c_lines,
            )


    im.get_cmap().set_under(alpha=0)
    im.get_cmap().set_over(alpha=0)

    ax.set_xticks([])
    ax.set_yticks([])
    ax.invert_yaxis()

    cb_ax = plt.subplot(gs[:,-1])
    plt.subplots_adjust(left = 0.25) # FIXME: use OO call here
    # cb = plt.colorbar()
    cb = Colorbar(ax = cb_ax, mappable = im)
    taggers = [x['tagger'] for x in arrays]
    sig_label = array_one['signal']
    cb.set_label('{}-{} {s} efficiency'.format(*taggers, s = sig_label ))
    if do_contour: 
        cb.add_lines(ct)

    position = ax.get_position()
    new_aspect = ax.get_aspect()
    # ax.set_position(position)
        

    ax_log = fig.add_subplot(111, frameon = False)
    ax_log.set_xscale('log')
    ax_log.set_yscale('log')
    ax_log.axis((10**x_min, 10**x_max, 10**y_min, 10**y_max))
    ax_log.set_aspect(aspect)
    ax_log.set_xlabel('{} rejection'.format(array_one['x_bg']))
    ax_log.set_ylabel('{} rejection'.format(array_one['y_bg']))
    ax_log.grid(True)


    ax_log.set_aspect(new_aspect)
    ax_log.set_position(position)
            
    plt.savefig(out_name, bbox_inches = 'tight')
    plt.close()
