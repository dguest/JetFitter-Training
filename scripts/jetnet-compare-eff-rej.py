#!/usr/bin/env python2.6
import sys, os
from jetnet.perf import rejection 

def build_plot(file_name, variable, signal, backgrounds): 
    
    the_file = TFile(file_name)
    signal_hist_name = '_'.join([variable, 'matched', signal])
    the_signal_hist = the_file.Get(signal_hist_name)

    background_names = ['_'.join([variable,'matched',b]) for b in backgrounds]
    background_hists = [the_file.Get(n) for n in background_names]

    graph = rejection.plots_to_rej_vs_eff(
        signal = the_signal_hist, 
        background = background_hists, 
        y_function = rejection.rejection)

    return graph 



def build_comparison(files, signal, background):  
    d_string = 'al'
    t = TROOT
    colors = [t.kOrange, t.kBlue, t.kRed, t.kGreen + 2]
    min_vals = []
    max_vals = []
    all_graphs = []

    canname = '%s tagging %s rejection' % (signal, background)
    canvas = TCanvas(canname.replace(' ','_'), canname, 100,100,600,400)
    legend = TLegend(0.6, 0.6, 0.8, 0.8)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    variables = '%s_over_%s' % (signal[0], background[0])

    for color, file_name in zip(colors, files): 
        rej_plot = build_plot(file_name = file_name, variable = variables, 
                              signal = signal, backgrounds = [background])

    
        rej_plot.GetXaxis().SetLimits(0,1)
        rej_plot.GetXaxis().SetRangeUser(0,1)
        rej_plot.Draw(d_string)
        d_string = 'l'

        rej_plot.SetMarkerColor(color)
        rej_plot.SetLineColor(color)
        rej_plot.SetLineWidth(2)

        the_max = TMath.MaxElement(rej_plot.GetN(),rej_plot.GetY())
        the_min = TMath.MinElement(rej_plot.GetN(),rej_plot.GetY())

        min_vals.append(the_min)
        max_vals.append(the_max)
        all_graphs.append(rej_plot)

        # add legend
        leg_name = file_name.split(os.sep)[0]
        legend.AddEntry(rej_plot,leg_name,'l')

    # ---- titles
    efficiency_name = '%s efficiency' % signal
    rejection_name = '%s rejection' % background
    title = ';'.join(['',efficiency_name, rejection_name])
    all_graphs[0].SetTitle(title)

    # ---- range 
    the_min = max(min(min_vals),1)
    the_max = max(max_vals)
    the_range = the_max - the_min
    y_mar = 0.1
    from math import log, exp
    all_graphs[0].SetMaximum(the_max*exp( y_mar*log(the_range)))
    all_graphs[0].SetMinimum(the_min*exp(-y_mar*log(the_range)))
    gPad.SetLogy()


    legend.Draw()
    canvas.graphs = all_graphs
    canvas.legend = legend

    return canvas


if __name__ == '__main__': 
    
    if len(sys.argv) == 1: 
        sys.exit( 'usage: %s <file paths>...' % 
                  os.path.basename(sys.argv[0]))

    from ROOT import TFile, TMath, TROOT, TCanvas, TLegend, gPad
    import AtlasStyle

    all_plots = []
    for signal in ['bottom','charm']: 
        for background in ['light','bottom','charm']: 
            if background == signal: continue 

            canvas = build_comparison(files = sys.argv[1:], 
                                      signal = signal, 
                                      background = background)
            all_plots.append(canvas)
                                          

    save_dest = raw_input('enter directory name to save plots: ')
    if save_dest: 
        os.mkdir(save_dest)
        for plot in all_plots:
            s_path = os.path.join(save_dest,plot.GetName() )
            for extension in ['.pdf','.png']: 
                plot.Print(s_path + extension)
