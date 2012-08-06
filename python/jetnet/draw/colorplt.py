#!/usr/bin/env python2.7
"""
script to print 2d plots colored by the values of three histograms. 
"""

__author__ = 'Dan Guest'
__contact__ = 'dguest@cern.ch'

from ROOT import TFile, TH1D
import numpy as np
import matplotlib.pyplot as plt
import re

# root_file = TFile('flavor_wt_hists.root')
# flavors = ['charm','bottom','light']

def _get_n_bins(hist): 
    x_bins = hist.GetNbinsX()
    y_bins = hist.GetNbinsY()
    return x_bins, y_bins

def _are_bins_equal(hist): 
    bin_range = xrange(1,hist.GetNbinsX() + 1)
    bin_ups = [hist.GetXaxis().GetBinUpEdge(b) for b in bin_range]
    bin_divs = [hist.GetBinLowEdge(1) ] + bin_ups
    ave_bin_diff = (bin_divs[-1] - bin_divs[0]) / (len(bin_divs) - 1)
    devs_from_linnear = []
    for i, b_val in enumerate(bin_divs): 
        dev = b_val - ave_bin_diff * i - bin_divs[0]
        devs_from_linnear.append(dev)

    return sum(devs_from_linnear) < ave_bin_diff / 1e4
    

def _get_bin_array(red_hist, green_hist, blue_hist): 

    hists = [red_hist, green_hist, blue_hist]
    
    bin_counts = [_get_n_bins(h) for h in hists]

    all_equal = (bin_counts[0] == bin_counts[1] == bin_counts[2])
    assert all_equal, 'oh dear!'

    assert _are_bins_equal(red_hist), "bins don't seem equal"
        

    x_bins, y_bins = _get_n_bins(red_hist)

    bin_array = []
    for y_bin in xrange(1,y_bins + 1): 
        x_row = []
        bin_array.append(x_row)
        for x_bin in xrange(1,x_bins + 1): 
            x_row.append( 
                [h.GetBinContent(x_bin, y_bin) for h in hists])
                          
    np_array = np.array(bin_array)

    # it this fixes the reversed y axis, should probably figure out less of 
    # a hack to do this
    np_array = np_array[::-1,:,:]   

    return np_array

def _get_bin_ranges(hist2d): 
    x_axis = hist2d.GetXaxis()
    x_max = x_axis.GetBinUpEdge(x_axis.GetNbins())
    x_min = x_axis.GetBinLowEdge(1)
    
    y_axis = hist2d.GetYaxis()
    y_max = y_axis.GetBinUpEdge(y_axis.GetNbins())
    y_min = y_axis.GetBinLowEdge(1)

    return ( (x_min,x_max), (y_min,y_max))


_replacements = [
    ('Eta',r' $\eta$'), 
    ('Pt' ,r' $p_\mathrm{T}$'), 
    ('logCb',r'$\log (w_\mathrm{c} / w_\mathrm{b})$'), 
    ('logCu',r'$\log (w_\mathrm{c} / w_\mathrm{u})$'), 
    ('NewTune',r''), 
    ('JetFitterCOMBNN','COMB'), 
    ('COMBNN','COMB'), 
    ('SVPlus','SVP'), 
    ('Lxy',r'$L_{xy}$'), 
    ('rap',r'$\varphi$'), 
    ('$$','$ $'), 
    ('$_','$ '), 
    ]

_regex_subs = [ 
    (r'(?<=[A-Za-z])_(?=[$A-Za-z]+)',r' '), 
    ]

def _texify(string, replacements = _replacements): 
    for search, rep in replacements: 
        string = string.replace(search,rep)

    for s, r in _regex_subs: 
        string = re.sub(s,r,string)

    return string 

def _make_tags(plots, translate_label = _texify): 
    tags = []
    x_labels = []
    y_labels = []
    for pl in plots: 
        y_label, x_label = '_'.join(pl.split('_')[:-1]).split('_vs_')
        x_labels.append(x_label)
        y_labels.append(y_label)
        tags.append(pl.split('_')[-1])

    if all(x_labels[0] == x for x in x_labels): 
        xlab = x_labels[0]
    else: 
        xlab = 'x'

    if all(y_labels[0] == x for x in y_labels): 
        ylab = y_labels[0]
    else: 
        ylab = 'y'

    return translate_label(xlab), translate_label(ylab), tags

_flavors = ['charm','bottom','light']
_def_plots = ['JetEta_vs_JetPt_{}'.format(t) for t in _flavors]

def print_color_plot(root_file_name, plots = _def_plots, 
                     normalize_by_tag = False, logz = False, 
                     legloc = 'best', out_file_name = '', 
                     tag_plots = _make_tags): 
    """
    makes a color plot, prints as a pdf... work in progress
    """


    root_file = TFile(root_file_name)

    h = [root_file.Get(p) for p in plots]
    np_array  = _get_bin_array(*h)

    if logz: 
        np_array = np_array + 1
        np_array = np.log(np_array)

    array_max_vals = [np_array[:,:,i].max() for i in xrange(3)]

    if normalize_by_tag: 
        for i,the_max in enumerate(array_max_vals): 
            np_array[:,:,i] = np_array[:,:,i] / the_max
    else: 
        overall_max = max(array_max_vals)
        np_array = np_array / overall_max

    first_plot = h[0]
    pt_range, eta_range = _get_bin_ranges(first_plot)
        
    fig = plt.figure()
    pt_span = pt_range[1] - pt_range[0]
    eta_span = eta_range[1] - eta_range[0]
    all_extent = (pt_range[0], pt_range[1],
                  eta_range[0],eta_range[1])
    all_aspect = pt_span / eta_span * 2/3
    plot = plt.imshow(np_array, 
                      extent = all_extent, 
                      aspect = all_aspect, 
                      interpolation='nearest')
    
    xlab,ylab,tags = tag_plots(plots)
    
    plt.xlabel(xlab, fontsize = 24)
    plt.xticks(fontsize = 16)
    plt.ylabel(ylab, fontsize = 24)
    plt.yticks(fontsize = 16)

    empty = np.array([])
    for tag, color in zip(tags, 'rgb'): 
        plt.plot(empty, color + '-', label = tag , lw = 10)
    
    plt.legend(loc = legloc)
    plt.axis(all_extent)

    if not out_file_name: 
        out_file_name = '.'.join(root_file.GetName().split('.')[:-1]) 

    plt.savefig(out_file_name + '.pdf', 
                bbox_inches = 'tight')

