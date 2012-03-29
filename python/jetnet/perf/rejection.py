import os, sys, math, re, array
from math import exp, log
from random import random
from ROOT import TTree, TH1D, TFile, TCanvas, TROOT, gPad, TGraph, TLegend

class Q: 
    """
    basically an enum type which (hopefully) macthes the ntuple truth info
    """
    DOWN, UP, STRANGE, CHARM, BOTTOM, TOP = range(1,7)
    LIGHT = DOWN


class PerfPlot(TH1D): 
    def __init__(self, name, x_map, bins = 100, x_min = -0.1, x_max = 1.1, 
                 wt_map = lambda x: 1): 
        super(PerfPlot,self).__init__(str(random()),name,bins, x_min, x_max)
        self.name = name
        self.x_map = x_map
        self.wt_map = wt_map

    def fill(self,fill_buffer): 
        fill_weight = self.wt_map(fill_buffer)
        x_value = self.x_map(fill_buffer)
        if fill_weight is not None and x_value is not None: 
            self.Fill(x_value, 
                      fill_weight)


def get_map_with_flavor( map_function, name = None, flavor = Q.CHARM): 
    def the_map(fill_buffer): 
        if fill_buffer.cat_flavour != flavor: 
            return None
        
        return map_function(fill_buffer)

    if name is not None: 
        the_map.name = name

    return the_map

# --- FOM functions

def b_over_l(f): 
    return f.NNb / (f.NNu + f.NNb)

def c_over_l(f): 
    return f.NNc / (f.NNu + f.NNc)

def b_over_c(f): 
    return f.NNb / (f.NNc + f.NNb)

def c_over_b(f): 
    return f.NNc / (f.NNc + f.NNb)

def b_over_all(f): 
    return f.NNb / (f.NNc + f.NNb + f.NNu)

def c_over_all(f): 
    return f.NNc / (f.NNc + f.NNb + f.NNu)

def get_square_of(n): 
    n_pad_x = int(math.floor(math.sqrt(n)))
    n_pad_y = int(math.ceil( float(n) / float(n_pad_x)))
    return n_pad_x, n_pad_y


t = TROOT
color_dict = { 
    'charm': t.kOrange, 
    'light': t.kBlue, 
    'bottom': t.kRed, 
    'over c': t.kOrange, 
    'over l': t.kBlue, 
    'over b': t.kRed, 
    'over all': t.kGreen + 2, 
    }
color_key_re = re.compile('|'.join(color_dict.keys()))
    
def define_plots(): 
    plot_function_list = [
        (b_over_l, 'b over l'), 
        (c_over_l, 'c over l'), 
        (b_over_c, 'b over c'), 
        (c_over_b, 'c over b'), 
        (c_over_all, 'c over all'), 
        (b_over_all, 'b over all'), 
        ]
    
    quark_flavor_list = [ 
        (Q.BOTTOM, 'bottom'), 
        (Q.CHARM, 'charm'), 
        (Q.LIGHT, 'light'), 
        ]

    all_plots = []
    plots_by_function = []
    for function, func_name in plot_function_list: 
        this_function_plots = []
        for truth_q, truth_name in quark_flavor_list: 
            
            x_axis_name = '%s, %s matched' % (func_name, truth_name)
            the_map = get_map_with_flavor(
                map_function = function, 
                name = x_axis_name, 
                flavor = truth_q)

            the_plot = PerfPlot(name = the_map.name, x_map = the_map)
            the_plot.truth_name = truth_name
            the_plot.func_name = func_name
            this_function_plots.append(the_plot)
            all_plots.append(the_plot)

        plots_by_function.append(this_function_plots)

    return plots_by_function

def save_plots(plots_by_function, file_name): 
    root_file = TFile(file_name, 'recreate')
    
    for plist in plots_by_function: 
        for plot in plist: 
            plot_name = '%s matched %s' % (plot.func_name, plot.truth_name)
            plot.SetName(plot_name.replace(' ','_'))
            root_file.WriteTObject(plot)

def get_plots(file_name): 
    plots_by_function = {}
    root_file = TFile(file_name)
    key_list = root_file.GetListOfKeys()
    for key in key_list: 
        hist = key.ReadObj()
        function, truth = hist.GetName().replace('_',' ').split('matched')
        hist.truth_name = truth.strip()
        hist.func_name = function.strip()
        try:
            plots_by_function[function].append(hist)
        except KeyError: 
            plots_by_function[function] = [hist]
        

    ret_value = plots_by_function.values()
    return ret_value, root_file

def fill_plots(plots_by_function, root_file, cut_list = [], 
               max_events = None): 

    if isinstance(root_file, str): 
        root_file = TFile(root_file)
    root_tree = root_file.Get('performance')
    
    n_entries_total = root_tree.GetEntriesFast()

    print '%i entries in tree' % n_entries_total

    n_cuts = 0
    
    def fails_cut(entry): 
        for cut in cut_list: 
            if cut(entry): 
                n_cuts += 1
                return True
        return False


    for entry_n, entry in enumerate(root_tree): 

        if max_events is not None: 
            if entry_n > max_events: 
                break 

        if fails_cut(entry): 
            continue

        for plot_list in plots_by_function: 
            for plot in plot_list: 
                plot.fill(entry)


        if entry_n % 20000 == 0: 
            print '%i of %i entries processed' % (entry_n, n_entries_total)
        
        # if entry_n > 100000: break 

    print 'cut %i events' % n_cuts

def bin_rev_iter(hist): 
    n_bins = hist.GetNbinsX()
    for bin_num in xrange(n_bins + 1,1, -1): 
        yield hist.GetBinContent(bin_num)

# --- y axis functions 
def sig_over_bg(signal, background, total_background): 
    if background == 0: 
        return None
    return signal / background
sig_over_bg.string = 'sig / bkg' 

def significance(signal, background, total_background): 
    if signal + background == 0: 
        return None
    return signal / math.sqrt(signal + background)
significance.string = '#frac{s}{#sqrt{s + b}}'

def rejection(signal, background, total_background): 
    if background == 0: 
        return None
    return total_background / background
rejection.string = 'rejection'

def plots_to_rej_vs_eff(signal, background, y_function = sig_over_bg): 

    if isinstance(background, list): 
        total_background = background[0].Clone(str(random()))
        for other_background in background[1:]: 
            total_background.Add(other_background)
        background = total_background
        
    total_signal = signal.GetEntries() 
    total_background = background.GetEntries() 

    sum_background = 0
    sum_signal = 0

    sig_array = array.array('d')
    bkg_array = array.array('d')

    bin_values = zip(bin_rev_iter(signal), bin_rev_iter(background))
    for sig_val, bkg_val in bin_values: 
        sum_signal += sig_val
        sum_background += bkg_val

        y_value = y_function(sum_signal, sum_background, total_background)
        if y_value is not None: 
            sig_array.append(sum_signal / total_signal)
            bkg_array.append(y_value)


    assert len(sig_array) == len(bkg_array)


    graph = TGraph(len(sig_array), sig_array, bkg_array)
    return graph

def test_graphs(plots_by_function): 
    signal = None
    background = []
    for fom_group in plots_by_function: 
        print '--- new fom group ---'
        for plot in fom_group: 
            print '%s, %s, %s' % (plot.GetName(), 
                                  plot.func_name, plot.truth_name)

        the_graph = 'none'

def draw_plots(plots_by_function, logy = True, canvas_name = 'performance'): 

    perf_canvas = TCanvas(canvas_name,canvas_name,
                          50,50,600*2, 400*2)


    n_pad_x, n_pad_y = get_square_of(len(plots_by_function))
    perf_canvas.Divide(n_pad_x, n_pad_y)

    for pad, plot_list in enumerate(plots_by_function,1): 
        perf_canvas.cd(pad)

        min_vals = []
        max_vals = []

        num_name, denom_name = plot_list[0].func_name.split('over')
        x_label = '#frac{%s}{%s + %s}' % (num_name, num_name, denom_name)
        if 'all' in denom_name: 
            x_label = '#frac{%s}{all}' % (num_name)
        new_plot_title = ';'.join(['',x_label,'n Jets'])
        plot_list[0].SetTitle(new_plot_title)
        plot_list[0].Draw()
        for plot in plot_list[1:]: 
            plot.Draw('same')

        for plot in plot_list: 
            color = color_dict[color_key_re.findall(plot.truth_name)[0]]
            plot.SetLineColor(color)
            min_vals.append(plot.GetBinContent(plot.GetMinimumBin()))
            max_vals.append(plot.GetBinContent(plot.GetMaximumBin()))

        the_min = max(min(min_vals),1)
        the_max = max(max_vals)
        the_range = the_max - the_min
        y_mar = 0.1

        if logy: 
            plot_list[0].SetMaximum(the_max*exp( y_mar*log(the_range)))
            plot_list[0].SetMinimum(the_min*exp(-y_mar*log(the_range)))
            gPad.SetLogy()
        else: 
            plot_list[0].SetMaximum(the_max + y_mar*the_range)
            plot_list[0].SetMinimum(the_min - y_mar*the_range)

    return perf_canvas

def draw_graphs(plots_by_function, logy = True, variable_list = None, 
                y_function = sig_over_bg, canvas_name = 'rej_vs_eff'): 

    perf_canvas = TCanvas(canvas_name,canvas_name,
                          50,50,600,400)

    d_string = 'al'

    all_graphs = []
    min_vals = []
    max_vals = []
    legend = TLegend(0.6, 0.6, 0.9,0.9)
    legend.SetFillStyle(0)
    legend.SetBorderSize(0)

    for plot_list in plots_by_function:
    
        plot_variable = plot_list[0].func_name
        num_name, denom_name = plot_variable.split('over')
        x_label = '#frac{%s}{%s + %s}' % (num_name, num_name, denom_name)
            
        new_plot_title = ';'.join(['',x_label,'n Jets'])
        if variable_list: 
            if plot_variable not in variable_list: 
                continue

        signal_char, bg_char = plot_list[0].func_name.split(' over ')

        signal_hist = None
        background_list = []
        for plot in plot_list: 
            truth_char = plot.truth_name[0]
            if truth_char == signal_char: 
                assert signal_hist is None
                signal_hist = plot
            elif truth_char == bg_char or bg_char == 'all': 
                background_list.append(plot)

            else: 
                print 'not including %s in %s' % (plot.truth_name, 
                                                  plot.func_name)

                print signal_char, bg_char

        rej_plot = plots_to_rej_vs_eff(signal_hist, background_list, 
                                       y_function = y_function)

        rej_plot.GetXaxis().SetLimits(0,1)
        rej_plot.GetXaxis().SetRangeUser(0,1)
        rej_plot.Draw(d_string)
        d_string = 'l'

        color = color_dict[color_key_re.findall(signal_hist.func_name)[0]]
        rej_plot.SetMarkerColor(color)
        rej_plot.SetLineColor(color)
        rej_plot.SetLineWidth(2)

        from ROOT import TMath
        # the_max = TMath.GeomMean(rej_plot.GetN(),rej_plot.GetY())
        the_max = TMath.MaxElement(rej_plot.GetN(),rej_plot.GetY())
        the_min = TMath.MinElement(rej_plot.GetN(),rej_plot.GetY())

        min_vals.append(the_min)
        max_vals.append(the_max)
        all_graphs.append(rej_plot)

        # add legend
        func_name = signal_hist.func_name
        sig_name, bg_name = func_name.split(' over ')
        if bg_name == 'all': 
            leg_entry = 'nn_{%s} / nn_{all}' % sig_name
        else: 
            leg_entry = 'nn_{%s} / nn_{%s + %s}' % (
                sig_name, sig_name, bg_name)

        legend.AddEntry(rej_plot,leg_entry,'l')

    the_min = min(min_vals)
    # the_max = (sum(max_vals) / len(max_vals)) * 3
    the_max = max(max_vals) 
    the_range = the_max - the_min
    y_mar = 0.1
    print the_min, '--', the_max

    try: 
        y_title = y_function.string 
    except AttributeError: 
        y_title = 'unknown something' 

    all_graphs[0].SetTitle(';'.join(['','efficiency',y_title]))

    if logy: 
        all_graphs[0].SetMaximum(the_max*exp( y_mar*log(the_range)))
        all_graphs[0].SetMinimum(the_min*exp(-y_mar*log(the_range)))
        gPad.SetLogy()
    else: 
        all_graphs[0].SetMaximum(the_max + y_mar*the_range)
        all_graphs[0].SetMinimum(the_min - y_mar*the_range)

    legend.Draw()
    perf_canvas.graphs = all_graphs
    perf_canvas.legend = legend
    return perf_canvas


# ---- cuts 

def jet_pt_cut(entry): 
    pass_cut = 50 < entry.JetPt < 180
    return not pass_cut

def jet_eta_cut(entry): 
    pass_cut = abs(entry.JetEta) < 1.5
    return not pass_cut

# --- run function 

def make_plots_from(ntuple_file_name, cut_list = [jet_eta_cut], 
                    max_events = None):
    

    cache_file_name = '%s_cache%s' % os.path.splitext(ntuple_file_name)

    ntuple_file = TFile(ntuple_file_name)
    cache_file = None

    if not os.path.isfile(cache_file_name): 

        plots_by_function = define_plots()
        fill_plots(plots_by_function, ntuple_file, cut_list = cut_list, 
                   max_events = max_events)
        save_plots(plots_by_function,cache_file_name)

    else: 
        plots_by_function, cache_file = get_plots(cache_file_name)


    perf_canvas = draw_plots(plots_by_function)
    perf_canvas.file = cache_file
    
    # raw_input('press enter')

    eff_graphs = [ 
        ('c_tagging',['c over all','c over b','c over l']), 
        ('b_tagging',['b over all','b over c','b over l']), 
        ]
    rej_vs_eff_list = []
    for name, plots in eff_graphs:
        
        rej_vs_eff = draw_graphs(plots_by_function, 
                                 variable_list = plots, 
                                 y_function = rejection, 
                                 logy = True, 
                                 canvas_name = name)

        rej_vs_eff_list.append(rej_vs_eff)

    all_canvas = [perf_canvas] + rej_vs_eff_list


    return all_canvas
    # raw_input('press enter')
