import numpy as np
import os, sys, warnings, cPickle, itertools, math
import glob
import matplotlib.pyplot as plt
import matplotlib as mp
from matplotlib.gridspec import GridSpec
from matplotlib.colorbar import Colorbar

"""
routine to draw b-rejection vs c- or l-rejection plots
"""

def _simplify_tagger_name(name): 
    if name == 'JetFitterCOMBNN': 
        return 'COMBNN'
    elif 'COMBNN' in name and 'SV' in name: 
        return 'JetFitterCharm'
    else: 
        return name

_flav_to_char = {
    'light':'u', 
    'charm':'c', 
    'bottom':'b', 
    }

def plot_pickle(pickle, output_name, use_contour = True): 
    """
    draw rejrej plot in pickle
    """
    with open(pickle) as pkl: 
        pdict = cPickle.load(pkl)
        _plot_eff_array(pdict, use_contour=use_contour,out_name=output_name)

def _plot_eff_array(array_dict, use_contour = True, 
                    out_name = 'rejrej.pdf'):     

    eff_array = array_dict['eff']
    x_min, x_max = array_dict['x_range']
    y_min, y_max = array_dict['y_range']
    eff_array[np.nonzero(eff_array < 0.0)] = np.nan

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
        vmin = 0, 
        vmax = 1, 
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

    im.get_cmap().set_bad(alpha=0)

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
    
def plot_overlay(new_pickle, old_pickle , out_name, 
                 do_rel = False, z_range=None, do_contour=False): 
    """
    overlay two rejrej plots. The difference shown will be the first 
    plot relative to the second. 
    """
    with open(old_pickle) as pkl: 
        old_plot = cPickle.load(pkl)
    with open(new_pickle) as pkl: 
        new_plot = cPickle.load(pkl)

    _overlay_rejrej(
        array_one=new_plot, array_two=old_plot, 
        do_rel=do_rel, out_name=out_name, z_range=z_range, 
        do_contour=do_contour)

def _overlay_rejrej(array_one, array_two,
                    out_name = 'rejrej.pdf', 
                    do_rel = False, do_contour = False, z_range = None):     


    arrays = array_one, array_two

    for a in arrays: 
        blank_cells = np.nonzero(a['eff'] <= 0.0 )
        array_one['eff'][blank_cells] = np.NaN

        
    if do_rel: 
        old_warn_set = np.seterr(divide = 'ignore') 
        eff_array = array_one['eff'] / array_two['eff']
        np.seterr(**old_warn_set)
    else: 
        eff_array = array_one['eff'] - array_two['eff']


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

    plt_min = np.min(eff_array[np.nonzero(np.isfinite(eff_array))])
    plt_max = np.max(eff_array[np.nonzero(np.isfinite(eff_array))]) 

    plt_range = plt_max - plt_min
    plt_order = math.trunc(math.log10(plt_range))
    
    new_min = round(plt_min - 0.5*10**(plt_order),-plt_order)
    new_max = round(plt_max + 0.5*10**(plt_order),-plt_order)
    new_range = new_max - new_min
    n_subdiv = new_range * 10**math.floor(math.log10(new_range) )
    if n_subdiv <= 5: 
        n_subdiv *= 2

    plt_min = new_min
    plt_max = new_max

    if z_range: 
        plt_min, plt_max = z_range

    print 'plot range: {: .4f}--{:.4f}'.format(plt_min, plt_max)

    im = ax.imshow(
        eff_array, 
        origin = 'upper', 
        extent = (x_min,x_max, y_min, y_max), 
        aspect = aspect, 
        interpolation = 'nearest', 
        vmin = plt_min, 
        vmax = plt_max, 
        )

    
    c_lines = np.linspace(plt_min, plt_max, n_subdiv + 1)
    if len(c_lines) == 0: 
        do_contour = False
    if do_contour: 
        ct = ax.contour(
            eff_array, 
            origin = 'upper', 
            extent = (x_min,x_max, y_min, y_max), 
            aspect = aspect, 
            linewidths = 2, 
            levels = c_lines,
            colors = 'k', 
            )
        plt.clabel(ct, fontsize=9, inline=1, 
                   fmt = '%.{}f'.format(-plt_order + 1))

    im.get_cmap().set_bad(alpha=0)

    ax.set_xticks([])
    ax.set_yticks([])
    ax.invert_yaxis()

    cb_ax = plt.subplot(gs[:,-1])
    plt.subplots_adjust(left = 0.25) # FIXME: use OO call here
    # cb = plt.colorbar()
    cb = Colorbar(ax = cb_ax, mappable = im)
    taggers = [_simplify_tagger_name(x['tagger']) for x in arrays]
    flav_char = _flav_to_char[array_one['signal']]
    sig_label = '$\epsilon_\mathrm{{ {} }}$'.format(flav_char)
    cb_lab_string = '{} $/$ {} {s}' if do_rel else '{} $-$ {} {s}'
    cb.set_label(cb_lab_string.format(*taggers, s = sig_label ))
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
