#!/usr/bin/env python2.6

import sys, os
from ROOT import TFile, TH1F, TCanvas, TROOT


def sig_over_background(root_file, signal = 'charm', background = 'light'): 
    search_string = '_%s_over_%s_' % (signal, background)

    matches = []
    
    if isinstance(root_file,str): 
        root_file = TFile(root_file)

    for key in root_file.GetListOfKeys(): 
        obj = key.ReadObj()
        if search_string in obj.GetName(): 
            matches.append(obj)

    t = TROOT
    color_dict = { 
        'charms': t.kOrange, 
        'lights': t.kBlue, 
        'bottoms': t.kRed
        }

    labels = (
        '',
        '#frac{%s}{%s + %s}' % (signal,signal,background), 
        'entries'
        )
    canvas = TCanvas(search_string,';'.join(labels),100,100,600,400)
    draw_string = ''

    for hist in matches: 
        
        color = color_dict[hist.GetName().split('_')[0]]
        hist.SetLineColor(color)
        hist.Draw(draw_string)
        draw_string = 'same'

    canvas.file = root_file
    return canvas

if __name__ == '__main__': 

    inputs = sys.argv[1:]

    if not len(inputs) == 2: 
        sys.exit('usage: %s, <signal> <background>' % 
                 os.path.split(sys.argv[0])[-1])

    hist_file = 'all_plots.root'

    sig_name = sys.argv[1]
    background_name = sys.argv[2]

    sig_over_background(hist_file, sig_name, background_name)

    raw_input('press enter to continue')
