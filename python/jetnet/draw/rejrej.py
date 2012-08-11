import numpy as np
import os, sys, warnings, cPickle
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec
from matplotlib.colorbar import Colorbar
from jetnet.cxxprofile import pro2d

"""
routine to draw b-rejection vs c- or l-rejection plots
"""

_tags = ['bottom','charm','light']

def _get_bins(file_name, hist_name, message = ''): 
    
    if message: 
        print message
    from ROOT import TFile
    root_file = TFile(file_name)
    
    hist = root_file.Get(hist_name)
    if not hist: 
        raise IOError('no {} found in {}'.format(hist_name, file_name))

    x_bins = hist.GetNbinsX()
    y_bins = hist.GetNbinsY()
    
    bins = np.zeros((x_bins,y_bins))

    for y_bin in xrange(y_bins): 
        for x_bin in xrange(x_bins): 
            bins[x_bin,y_bin] = hist.GetBinContent(x_bin + 1,y_bin + 1)

    return bins

def _get_integrals(bins, verbose_string = None): 
    """
    verbose string should have two {} in it
    """
    sums = np.zeros(bins.shape)
    xmax = bins.shape[0]
    for x in xrange(xmax): 
        if verbose_string: 
            sys.stdout.write('\r')
            sys.stdout.write(verbose_string.format(x,xmax))
            sys.stdout.flush()
        for y in xrange(bins.shape[1]): 
            sums[x,y] = bins[x:,y:].sum()
    if verbose_string: 
        sys.stdout.write('\n')
    return sums

def _get_integrals_fast(bins, verbose_string = None): 
    """
    much faster and simpler way to compute integrals, uses numpy cumsum
    """
    print verbose_string
    y_sums = np.cumsum(bins[:,::-1], axis = 1)[:,::-1]
    sums = np.cumsum(y_sums[::-1,:], axis = 0)[::-1,:]
    return sums

def _max_noninf(array): 
    return np.amax(array[np.nonzero(np.isfinite(array))])

def _get_binner(max_val, n_bins = 100): 
    def binner(x): 
        frac = x / max_val * 0.99999999 # never give top bin
        return int(frac * n_bins )
    return binner

def _build_hists(input_file, cache_file = 'rejrej_cache.root', 
                 plot_list = '', bins = 600): 
    if not plot_list: 
        plot_list = cache_file.split('.')[0] + '.txt'
    if not os.path.isfile(cache_file): 
        out_file, hists = pro2d(
            in_file = input_file, 
            tree = 'SVTree', 
            out_file = cache_file, 
            plots = [ 
                ( 
                    ('logBcCOMBNN_SVPlus_rapLxy', bins, -5, 10), 
                    ('logBuCOMBNN_SVPlus_rapLxy', bins, -6, 12) 
                    ), 
                (
                    ('logBcJetFitterCOMBNN', bins, -5, 10), 
                    ('logBuJetFitterCOMBNN', bins, -6, 12) 
                    ), 
                (
                    ('logCbCOMBNN_SVPlus_rapLxy', bins, -8, 4), 
                    ('logCuCOMBNN_SVPlus_rapLxy', bins, -5, 8) 
                    ), 
                (
                    ('logCbJetFitterCOMBNN', bins, -8, 4), 
                    ('logCuJetFitterCOMBNN', bins, -5, 8) 
                    ), 
                ], 
            tags = _tags, 
            # max_entries = 100000, 
            show_progress = True)
        with open(plot_list,'w') as pl: 
            for name in hists: 
                pl.write(name + '\n')
    else: 
        out_file = cache_file
        with open(plot_list) as pl: 
            hists = [n.strip() for n in pl]

    return out_file, hists

def _get_cuts_cache(cuts_pkl, tagged_hists, root_hists): 

    cache_dict = {
        'cuts':{}, 
        'rej':{}, 
        }

    integrals = cache_dict['cuts']

    if not os.path.isfile(cuts_pkl): 
        for h in tagged_hists: 
            integrals[h] = _get_integrals_fast(
                _get_bins(root_hists, h, message = 'getting bins for' + h), 
                verbose_string = 'integrating ' + h)
        with open(cuts_pkl,'w') as pkl: 
            cPickle.dump(cache_dict, pkl)
    else: 
        sys.stdout.write('reading pickle...')
        sys.stdout.flush()
        with open(cuts_pkl) as pkl: 
            cache_dict = cPickle.load(pkl)
        sys.stdout.write('done\n')

    return cache_dict



def build_rej_plots(in_ntuple, bins): 
    cache_dir = 'cache'
    if not os.path.isdir(cache_dir): 
        os.mkdir(cache_dir)

    cache_file = os.path.join(cache_dir,'wtdist_cache.root')
    cache_file, hist_names = _build_hists(in_ntuple, 
                                          cache_file = cache_file, 
                                          bins = bins)

    if not os.path.isfile(cache_file): 
        sys.exit('vuf')

    rej_pickle = os.path.join(cache_dir,'rej_cache.pkl')
    if os.path.isfile(rej_pickle): 
        with open(rej_pickle) as pkl: 
            rej_plots = cPickle.load(pkl)
    else: 
        rej_plots = {}

    taggers = ['COMBNN_SVPlus','JetFitterCOMBNN']
    missing_taggers = [t for t in taggers if t not in rej_plots]

    if missing_taggers: 
        tagged_hists = [h for h in hist_names if h.split('_')[-1] in _tags]
        cuts_pickle = os.path.join(cache_dir,'cuts_cache.pkl')
        cache_dict = _get_cuts_cache(cuts_pickle, tagged_hists, cache_file)
        for tagger in missing_taggers: 
            rej_plots[tagger] = _build_plots_from_integrals(
                cache_dict, tagger = tagger )
        with open(rej_pickle,'w') as pkl: 
            cPickle.dump(rej_plots, pkl)

    return rej_plots

def run(in_ntuple = 'perf_ntuple.root', bins = 600): 
    """
    run on in_ntuple, produce bins^2 cuts
    """


    rej_plots = build_rej_plots(in_ntuple, bins)

    for tagger, flavors in rej_plots.iteritems(): 
        for flavor, plot in flavors.iteritems(): 
            print 'making {} {}'.format(tagger, flavor)
            name = '{t}_{f}_contour.pdf'.format(t = tagger, f = flavor)
            _plot_eff_array(plot, use_contour = True,  out_name = name)
            name = '{t}_{f}.pdf'.format(t = tagger, f = flavor)
            _plot_eff_array(plot, use_contour = True,  out_name = name)

def _match_hist(hist_dict, matches): 
    match = None
    for h in hist_dict: 
        if all(m in h for m in matches): 
            if not match: 
                match = h
            else: 
                raise LookupError('both {} and {} match {}'.format(
                        match, h, matches))
    if not match: 
        raise LookupError('no {} in {}'.format(matches, hist_dict.keys()))
    return hist_dict[match]

def _make_log_tag(signal, background): 
    short_tags = {'light':'u','bottom':'b','charm':'c'}
    tag = short_tags[signal].upper() + short_tags[background].lower()
    return 'log{}'.format(tag)

def _loop_over_entries(x_bins, y_bins, used_eff, n_out_bins): 
    """
    builds max efficiency at rejection by looping over all points in 
    cut grid. For small grids this should be faster. 
    """
    sys.stdout.write('finding eff at rejection... ')
    sys.stdout.flush()
    eff_array = np.zeros((n_out_bins,n_out_bins))
    for x_bin, y_bin, z in zip(x_bins, y_bins, used_eff): 
        # y_bin comes first because that's what imshow wants... 
        eff_array[y_bin,x_bin] = max(z, eff_array[y_bin,x_bin])

    sys.stdout.write('done\n')
    return eff_array

def _loop_over_bins(x_bins, y_bins, used_eff, n_out_bins): 
    """
    builds max efficiency at rejection in a different way, seems slower...
    """
    eff_array = np.zeros((n_out_bins,n_out_bins))
    info = '\rfinding eff at rejection, {} of {} rows... '
    for x_bin in xrange(n_out_bins): 
        sys.stdout.write(info.format(x_bin, n_out_bins))
        sys.stdout.flush()
        for y_bin in xrange(n_out_bins): 
            eff_index_this_bin = np.flatnonzero( (y_bins == y_bin) & 
                                                 (x_bins == x_bin) )
            if eff_index_this_bin.size: 
                max_in_bin = np.max(used_eff[eff_index_this_bin])
                # y_bin comes first because that's what imshow wants... 
                eff_array[y_bin,x_bin] = max_in_bin
    sys.stdout.write('done\n')
    return eff_array

def _get_rejrej_array(flat_eff, flat_x, flat_y): 
    indices = np.nonzero( (flat_eff > 0.05) & 
                          np.isfinite(flat_x) & 
                          np.isfinite(flat_y)
                          )

    used_x = np.log10(flat_x[indices])
    used_y = np.log10(flat_y[indices])
    used_eff = flat_eff[indices]

    # allow 1% safety margin on max value
    max_x = _max_noninf(used_x) * 1.0001
    max_y = _max_noninf(used_y) * 1.0001

    min_x = np.min(used_x)
    min_y = np.min(used_y)

    if min_y != 0.0 or min_x != 0.0: 
        warnings.warn(
            'may not work with current minx ({}) or miny ({})'.format(
                min_x, min_y))

    n_out_bins = 100

    x_bin_values = np.linspace(min_x, max_x, n_out_bins)
    x_bins = np.digitize(used_x, bins = x_bin_values) - 1 # no underflow

    y_bin_values = np.linspace(min_y, max_y, n_out_bins)
    y_bins = np.digitize(used_y, bins = y_bin_values) - 1 # no underflow


    make_eff_array = _loop_over_entries

    eff_array = make_eff_array(x_bins, y_bins, used_eff, n_out_bins)
    
    return eff_array, (min_x, max_x), (min_y, max_y)


def _build_plots_from_integrals(cache_dict, tagger = 'COMBNN_SVPlus',
                                calc_by_grid = True): 

    try: 
        integrals = cache_dict['cuts']
    except KeyError: 
        integrals = cache_dict

    sys.stdout.write('normalizing histograms...')
    sys.stdout.flush()

    eff_dict = {n : a / a.max() for n, a in integrals.iteritems()}
    old_warn_set = np.seterr(divide = 'ignore') 
    rej_dict = {n : a.max() / a for n, a in integrals.iteritems()}
    np.seterr(**old_warn_set)

    sys.stdout.write('done\n')

    out_dict = {}

    for signal in [f for f in _tags if f != 'light']: 
        x_flav, y_flav = [f for f in _tags if f != signal]

        x_ratio_tag = _make_log_tag(signal, x_flav)
        y_ratio_tag = _make_log_tag(signal, y_flav)
        search_strings = [x_ratio_tag, y_ratio_tag, tagger]

        flat_eff = _match_hist(eff_dict, search_strings + [signal]).flatten()
        flat_x = _match_hist(rej_dict, search_strings + [x_flav]).flatten()
        flat_y = _match_hist(rej_dict, search_strings + [y_flav]).flatten()

        sys.stdout.write('in {} entries for {}\n'.format(signal, tagger))
        eff_array, x_range, y_range = _get_rejrej_array(
            flat_eff, flat_x, flat_y)

        out_dict[signal] = { 
            'eff':eff_array, 
            'x_range': x_range, 
            'y_range': y_range, 
            'signal':signal, 
            'x_bg':x_flav, 
            'y_bg':y_flav
            }

    return out_dict

def _plot_eff_array(array_dict, use_contour = True, 
                    out_name = 'rejrej.pdf'):     

    eff_array = array_dict['eff']
    x_min, x_max = array_dict['x_range']
    y_min, y_max = array_dict['y_range']

    fig = plt.figure()
    gs = GridSpec(20,21)
    ax = plt.subplot(gs[:,0:-1])
    aspect = float(x_max - x_min) / (y_max - y_min)

    im = plt.imshow(
        eff_array, 
        extent = (x_min,x_max, y_max, y_min), 
        aspect = aspect, 
        interpolation = 'nearest', 
        )

    if use_contour: 
        im.set_cmap('Greys')

        ct = ax.contour(
            eff_array, 
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
