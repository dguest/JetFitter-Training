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

def _simplify_tagger_name(tagger_dict): 
    name = tagger_dict['tagger']
    if name == 'JetFitterCOMBNN': 
        simple_name = 'COMBNN'
    elif 'COMBNN' in name and 'SV' in name: 
        simple_name = 'JetFitterCharm'
    else: 
        simple_name = name

    if tagger_dict['window_cut'] and 'MV' not in name: 
        simple_name += ' window'

    return simple_name

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
                 do_rel = False, z_range=None, do_contour=False, 
                 do_abs_contour=False): 
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
        do_contour=do_contour, do_abs_contour=do_abs_contour)

def _get_contour_order_and_lines(z_range): 
    """
    returns (order, lines), where order is an int, lines an array

    we want somewhere between 6 and 15 contour lines in z_range, 
    there's a messy way to do this, and it's not worth thinking about the 
    cleaner way
    """

    z_min = min(z_range)
    z_max = max(z_range)
    contour_range = z_max - z_min
    contour_order = math.trunc(math.log10(contour_range)) - 1
    
    c_min = round(z_min,-contour_order)
    c_max = round(z_max,-contour_order)
    round_range = c_max - c_min

    base_increment = 10**contour_order
    n_increments = round_range / base_increment
    
    if n_increments > 40: 
        base_increment *= 5
    elif n_increments > 15: 
        base_increment *= 2
    elif n_increments < 6: 
        base_increment *= 0.5

    n_increments = round_range / base_increment

    the_lines = np.arange(c_min, c_max, base_increment)
    
    subset_in_range = (the_lines > z_min) & (the_lines < z_max)
    return contour_order, the_lines[np.nonzero( subset_in_range)]
    

def _overlay_rejrej(array_one, array_two,
                    out_name = 'rejrej.pdf', 
                    do_rel = False, 
                    do_contour = False, 
                    do_abs_contour = False, 
                    z_range = None):     

    if do_contour and do_abs_contour: 
        warnings.warn("can't do both abs_contour and contour, "
                      "won't use abs_contour")
        do_abs_contour = False

    arrays = array_one, array_two

    # for a in arrays: 
    #     blank_cells = np.nonzero(a['eff'] <= 0.0 )
    #     array_one['eff'][blank_cells] = np.NaN
    #     array_two['eff'][blank_cells] = np.NaN
    # blank_cells = np.nonzero( )
    array_one['eff'][np.nonzero(array_one['eff'] <= 0.0)] = np.NaN
    array_two['eff'][np.nonzero(array_two['eff'] <= 0.0)] = np.NaN

        
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

    rel_min = np.min(eff_array[np.nonzero(np.isfinite(eff_array))])
    rel_max = np.max(eff_array[np.nonzero(np.isfinite(eff_array))]) 
    if z_range: 
        rel_min, rel_max = z_range

    if do_abs_contour: 
        contour_array = array_one['eff']
        signal = array_one['signal']
        contour_range = (0,0.5) if signal == 'charm' else (0.5,1)
    else: 
        contour_array = eff_array
        contour_range = (rel_min, rel_max)

    contour_order, c_lines = _get_contour_order_and_lines(contour_range)

    print 'plot range: {: .2f}--{:.2f}'.format(
        rel_min, rel_max)

    im = ax.imshow(
        eff_array, 
        origin = 'upper', 
        extent = (x_min,x_max, y_min, y_max), 
        aspect = aspect, 
        interpolation = 'nearest', 
        vmin = rel_min, 
        vmax = rel_max, 
        )

    
    if len(c_lines) == 0: 
        do_contour = False
    if do_contour or do_abs_contour: 
        ct = ax.contour(
            contour_array, 
            origin = 'upper', 
            extent = (x_min,x_max, y_min, y_max), 
            aspect = aspect, 
            linewidths = 2, 
            levels = c_lines,
            colors = 'k', 
            )
        plt.clabel(ct, fontsize=9, inline=1, 
                   fmt = '%.{}f'.format(-contour_order + 1 ))

    im.get_cmap().set_bad(alpha=0)

    ax.set_xticks([])
    ax.set_yticks([])
    ax.invert_yaxis()

    cb_ax = plt.subplot(gs[:,-1])
    plt.subplots_adjust(left = 0.25) # FIXME: use OO call here
    # cb = plt.colorbar()
    cb = Colorbar(ax = cb_ax, mappable = im)
    taggers = [_simplify_tagger_name(x) for x in arrays]
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
