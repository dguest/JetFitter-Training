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

def _get_bins(file_name, hist_name): 
    
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

def _max_noninf(array): 
    return np.amax(array[np.nonzero(np.isfinite(array))])

def _get_binner(max_val, n_bins = 100): 
    def binner(x): 
        frac = x / max_val * 0.99999999 # never give top bin
        return int(frac * n_bins )
    return binner

def _build_hists(input_file, cache_file = 'rejrej_cache.root', 
                 plot_list = 'rejrej_plots.txt', bins = 600): 
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

def run(in_ntuple = 'perf_ntuple.root', bins = 600): 
    """
    run on in_ntuple, produce bins^2 cuts
    """
    cache_file = 'rejrej_cache.root'
    cache_file, hist_names = _build_hists(in_ntuple, 
                                          cache_file = cache_file, 
                                          bins = bins)

    if not os.path.isfile(cache_file): 
        sys.exit('vuf')

    tagged_hists = [h for h in hist_names if h.split('_')[-1] in _tags]

    integrals = {}
    
    int_pickle = 'cuts_cache.pkl'
    if not os.path.isfile(int_pickle): 
        for h in tagged_hists: 
            integrals[h] = _get_integrals(
                _get_bins(cache_file, h), 
                verbose_string = h + '\t {} of {} rows complete')
        with open(int_pickle,'w') as pkl: 
            cPickle.dump(integrals, pkl)
    else: 
        with open(int_pickle) as pkl: 
            integrals = cPickle.load(pkl)

    _build_plots_from_integrals(integrals, tagger = 'COMBNN_SVPlus')
    _build_plots_from_integrals(integrals, tagger = 'JetFitterCOMBNN')

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

def _build_plots_from_integrals(integrals, tagger = 'COMBNN_SVPlus'): 
    eff_dict = {n : a / a.max() for n, a in integrals.iteritems()}
    
    # rej_dict = {}
    # lots of zeros in the integral array, allow div zero for now
    old_warn_set = np.seterr(divide = 'ignore') 
    # for bg in _tags: 
    #     rej_dict[bg] = integrals[bg].max() / integrals[bg]
    rej_dict = {n : a.max() / a for n, a in integrals.iteritems()}
    np.seterr(**old_warn_set)


    for signal in [f for f in _tags if f != 'light']: 
        x_flav, y_flav = [f for f in _tags if f != signal]

        x_ratio_tag = _make_log_tag(signal, x_flav)
        y_ratio_tag = _make_log_tag(signal, y_flav)
        search_strings = [x_ratio_tag, y_ratio_tag, tagger]

        flat_eff = _match_hist(eff_dict, search_strings + [signal]).flatten()
        flat_x = _match_hist(rej_dict, search_strings + [x_flav]).flatten()
        flat_y = _match_hist(rej_dict, search_strings + [y_flav]).flatten()
        # flat_x = rej_dict[x_flav].flatten()
        # flat_y = rej_dict[y_flav].flatten()

        indices = np.nonzero( (flat_eff > 0.1) & 
                              np.isfinite(flat_x) & 
                              np.isfinite(flat_y)
                              )


        used_x = np.log10(flat_x[indices])
        used_y = np.log10(flat_y[indices])
        used_eff = flat_eff[indices]

        max_x = _max_noninf(used_x)
        max_y = _max_noninf(used_y)

        min_x = np.min(used_x)
        min_y = np.min(used_y)

        if min_y != 0.0 or min_x != 0.0: 
            warnings.warn(
                'may not work with current minx ({}) or miny ({})'.format(
                    min_x, min_y))

        eff_array = np.zeros((100,100))

        get_x_bin = _get_binner(max_x)
        get_y_bin = _get_binner(max_y)
        print 'here be {} slowness'.format(signal)
        for x, y, z in zip(used_x, used_y, used_eff): 
            x_bin = get_x_bin(x)
            y_bin = get_y_bin(y)

            eff_array[y_bin,x_bin] = max(z, eff_array[y_bin,x_bin])
        
        fig = plt.figure()
        gs = GridSpec(20,21)
        ax = plt.subplot(gs[:,0:-1])
        aspect = float(max_x - min_x) / (max_y - min_y)

        im = plt.imshow(
            eff_array, 
            extent = (min_y,max_x, max_y, min_y), 
            aspect = aspect, 
            interpolation = 'nearest', 
            )
            # ).get_axes()

        ax.set_xticks([])
        ax.set_yticks([])
        ax.invert_yaxis()

        cb_ax = plt.subplot(gs[:,-1])
        plt.subplots_adjust(left = 0.25) # FIXME: use OO call here
        # cb = plt.colorbar()
        cb = Colorbar(ax = cb_ax, mappable = im)
        cb.set_label('{} efficiency'.format(signal))
        position = ax.get_position()
        new_aspect = ax.get_aspect()
        # ax.set_position(position)
        

        ax_log = fig.add_subplot(111, frameon = False)
        ax_log.set_xscale('log')
        ax_log.set_yscale('log')
        ax_log.axis((10**min_x, 10**max_x, 10**min_y, 10**max_y))
        ax_log.set_aspect(aspect)
        ax_log.set_xlabel('{} rejection'.format(x_flav))
        ax_log.set_ylabel('{} rejection'.format(y_flav))


        ax_log.set_aspect(new_aspect)
        ax_log.set_position(position)

        plt.savefig('{}_{}.pdf'.format(tagger,signal), bbox_inches = 'tight')
        plt.close()
