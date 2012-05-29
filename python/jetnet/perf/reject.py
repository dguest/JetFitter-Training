import array, random
import ROOT

# --- utility functions 
def rejection(signal, background, total_background): 
    if background == 0: 
        return None
    return total_background / background
rejection.string = 'rejection'

def bin_rev_iter(hist): 
    n_bins = hist.GetNbinsX()
    for bin_num in xrange(n_bins + 1,1, -1): 
        yield hist.GetBinContent(bin_num)

def bin_fwd_iter(hist): 
    n_bins = hist.GetNbinsX()
    for bin_num in xrange(1,n_bins + 1): 
        yield hist.GetBinContent(bin_num)



def plots_to_xy(signal, background, y_function = rejection, rev_itr = True): 
    """
    reads in root histograms for signal and background, returns
    (efficiency, rejection) tuple. 
    """
    if isinstance(background, list): 
        total_background = background[0].Clone(str(random.random()))
        for other_background in background[1:]: 
            total_background.Add(other_background)
        background = total_background
        
    total_signal = signal.GetEntries() 
    total_background = background.GetEntries() 

    sum_background = 0
    sum_signal = 0

    sig_array = array.array('d')
    bkg_array = array.array('d')

    if rev_itr == True: 
        itr_func = bin_rev_iter
    else: 
        itr_func = bin_fwd_iter

    bin_values = zip(itr_func(signal), itr_func(background))
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
