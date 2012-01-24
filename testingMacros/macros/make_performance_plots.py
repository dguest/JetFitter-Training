#!/usr/bin/env python2.6
import os, sys, math, re
from math import exp, log
from random import random
from ROOT import TTree, TH1D, TFile, TCanvas, TROOT, gPad
import AtlasStyle

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
    'bottom': t.kRed
    }
color_key_re = re.compile('|'.join(color_dict.keys()))
    


def define_plots(): 
    plot_function_list = [
        (b_over_l, 'b over l'), 
        (c_over_l, 'c over l'), 
        (b_over_c, 'b over c'), 
        (c_over_b, 'c over b'), 
        # (c_over_all, 'c over all'), 
        # (c_over_all, 'c over all'), 
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

def fill_plots(plots_by_function, file_name): 

    root_file = TFile(file_name)
    root_tree = root_file.Get('performance')
    
    n_entries_total = root_tree.GetEntriesFast()

    print '%i entries in tree' % n_entries_total

    for entry_n, entry in enumerate(root_tree): 

        for plot_list in plots_by_function: 
            for plot in plot_list: 
                plot.fill(entry)

        # print 'entry: %i' % entry_n
        # print 'flavor: %i' % entry.cat_flavour
        # print the_map.name
        # print the_map(entry)

        if entry_n % 20000 == 0: 
            print '%i of %i entries processed' % (entry_n, n_entries_total)
        
        # if entry_n > 100000: break 


def draw_plots(plots_by_function): 

    perf_canvas = TCanvas('performance','performance',
                          50,50,600*2, 400*2)

    n_pad_x, n_pad_y = get_square_of(len(plots_by_function))
    perf_canvas.Divide(n_pad_x, n_pad_y)

    for pad, plot_list in enumerate(plots_by_function,1): 
        perf_canvas.cd(pad)

        min_vals = []
        max_vals = []

        num_name, denom_name = plot_list[0].func_name.split('over')
        x_label = '#frac{%s}{%s + %s}' % (num_name, num_name, denom_name)
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

        # plot_list[0].SetMaximum(the_max + 0.1*the_range)
        # plot_list[0].SetMinimum(the_min - 0.1*the_range)

        plot_list[0].SetMaximum(the_max*exp( 0.1*log(the_range)))
        plot_list[0].SetMinimum(the_min*exp(-0.1*log(the_range)))
        gPad.SetLogy()
        


    # for plot in all_plots: 
    #     plot.Draw()
    raw_input('PRESS ENTER')


if __name__ == '__main__': 
    
    if len(sys.argv) != 2: 
        sys.exit('usage: %s <testing ntuple>' % 
                 os.path.basename(sys.argv[0]))

    cache_file_name = 'cache.root'

    if not os.path.isfile(cache_file_name): 

        plots_by_function = define_plots()
        fill_plots(plots_by_function, sys.argv[1])
        save_plots(plots_by_function,'cache.root')

    else: 
        plots_by_function, root_file = get_plots('cache.root')


    draw_plots(plots_by_function)



