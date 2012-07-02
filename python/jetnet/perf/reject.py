import array, random

# --- utility functions 
def _rejection(signal, background, total_background): 
    if background == 0: 
        return None
    return total_background / background
_rejection.string = 'rejection'

def _bin_rev_iter(hist): 
    n_bins = hist.GetNbinsX()
    for bin_num in xrange(n_bins + 1,1, -1): 
        yield hist.GetBinContent(bin_num)

def _bin_fwd_iter(hist): 
    n_bins = hist.GetNbinsX()
    for bin_num in xrange(1,n_bins + 1): 
        yield hist.GetBinContent(bin_num)

def sig_vs_rej(signal, background, flags = 'r'): 
    """
    makes signal vs rejection plots, uses numpy for faster computation
    by default starts with high bins ('r' flag). 
    """
    import numpy 
    if 'r' in flags: 
        the_itr = _bin_rev_iter
    else: 
        the_itr = _bin_fwd_iter

    sig_array = numpy.fromiter(the_itr(signal),'d')
    bkg_array = numpy.fromiter(the_itr(background),'d')
    
    sig_sum = numpy.cumsum(sig_array)
    bkg_sum = numpy.cumsum(bkg_array)
    
    total_signal = sig_sum[-1]
    total_backtround = bkg_sum[-1]

    nonzero_bkg = bkg_sum.nonzero()

    efficiency = sig_sum[nonzero_bkg] / total_signal
    rejection = total_backtround / bkg_sum[nonzero_bkg]

    return efficiency, rejection

def plots_to_xy(signal, background, y_function = _rejection, rev_itr = True): 
    """
    reads in root histograms for signal and background, returns
    (efficiency, rejection) tuple. 
    """
    if isinstance(background, list): 
        total_background = background[0].Clone(str(random.random()))
        for other_background in background[1:]: 
            total_background.Add(other_background)
        background = total_background
        
    # total_signal = signal.ComputeIntegral() 
    # total_background = background.ComputeIntegral() 
    # print 'sig integral: %f, num entries: %f' % (total_signal, 
    #                                              signal.GetEntries())

    sum_background = 0
    sum_signal = 0

    sig_array = array.array('d')
    bkg_array = array.array('d')

    if rev_itr == True: 
        itr_func = _bin_rev_iter
    else: 
        itr_func = _bin_fwd_iter

    bin_values = zip(itr_func(signal), itr_func(background))
    total_signal = sum(itr_func(signal))
    total_background = sum(itr_func(background))
    for sig_val, bkg_val in bin_values: 
        sum_signal += sig_val
        sum_background += bkg_val

        y_value = y_function(sum_signal, sum_background, total_background)
        if y_value is not None: 
            sig_array.append(sum_signal / total_signal)
            bkg_array.append(y_value)


    assert len(sig_array) == len(bkg_array)
    return sig_array, bkg_array

def plot_dict(root_file, target = 'TH1D'): 
    """
    reads in a root file, returns a dict of all TH1D (or target)
    """
    import ROOT
    from ROOT import TFile

    if isinstance(root_file, str): 
        root_file = TFile(root_file)
    the_dict = {}
    key_list = root_file.GetListOfKeys()
    ROOT.gROOT.cd()
    for key in key_list: 
        if key.GetClassName() == target: 
            hist = key.ReadObj()
            name = key.GetName()
            loc_hist = hist.Clone(str(random.random())) #.Rebin(rebin)
            the_dict[name] = loc_hist
            
    return the_dict


def make_plot_subdicts(input_dict): 
    """
    assuming input_dict is named by var_tag, seperates 
    into {'var':{'tag': object} } dict
    """
    var_dict = {}
    for name,plot in input_dict.iteritems(): 
        name_parts = name.split('_')
        var = '_'.join(name_parts[:-1])
        tag = name_parts[-1]

        if not var in var_dict: 
            var_dict[var] = {tag: plot}
        else: 
            var_dict[var][tag] = plot

    return var_dict
