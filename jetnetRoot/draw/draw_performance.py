#!/usr/bin/env python2.6

import sys, os
from ROOT import TFile, TH1F, TCanvas, TROOT, TLegend, gPad

class NoMatchError(LookupError): 
    def __init__(self, problem, signal = '', background = '', 
                 root_file = ''): 
        self.problem = problem
        self.signal = signal
        self.background = background

    def __str__(self): 
        if signal and background: 
            return 'could not find %s over %s' % ( 
                self.signal, self.background)
        else: 
            return self.problem 

def sig_over_background(root_file, signal = 'charm', background = 'light', 
                        canvas = None, pad = None, sample = 'test'): 
    search_string = '_%s_over_%s_' % (signal, background)

    train_matches = []
    test_matches = []
    
    if isinstance(root_file,str): 
        root_file = TFile(root_file)

    for key in root_file.GetListOfKeys(): 
        obj = key.ReadObj()
        if search_string in obj.GetName(): 
            if 'train' in obj.GetName(): 
                train_matches.append(obj)
            elif 'test' in obj.GetName(): 
                test_matches.append(obj)

    t = TROOT
    color_dict = { 
        'charms': t.kOrange, 
        'lights': t.kBlue, 
        'bottoms': t.kRed
        }

    labels = (
        'graph',
        '#frac{%s}{%s + %s}' % (signal,signal,background), 
        'entries'
        )
    
    if canvas is None: 
        canvas = TCanvas(search_string,';'.join(labels),100,100,600,400)

    if pad: 
        canvas.cd(pad)
    draw_string = ''

    if not test_matches + train_matches: 
        raise NoMatchError('ugh',signal = signal, background = background)
                           
    legend = TLegend(0.6,0.7,0.9,0.9)
    legend.SetFillStyle(0)
    legend.SetBorderSize(0)
    
    for hist in test_matches: 

        print 'drawing %s' % hist.GetName()
        sig_name = hist.GetName().split('_')[0]
        color = color_dict[sig_name]
        hist.SetTitle(';'.join(labels))
        hist.SetLineColor(color)
        hist.Draw(draw_string)
        draw_string = 'same'

        legend.AddEntry(hist, sig_name, 'l')

    gPad.SetLogy()
    legend.Draw()
    try: 
        canvas.files.append(root_file)
    except AttributeError: 
        canvas.files = [root_file]
    canvas.file = root_file
    canvas.legend = legend
    return canvas

def b_tag_plots(hist_file): 
    plots = [
        ('bottom','charm'), 
        ('bottom','light')
        ]

    import AtlasStyle
    AtlasStyle.SetAtlasStyle()


    canvas = TCanvas('performance','performance',100,100,600*2,400*2)
    canvas.Divide(2,2)

    results = []
    for pad_index, (signal, background) in enumerate(plots): 
        pad = pad_index + 1
        r = sig_over_background(root_file = hist_file, signal = signal, 
                                background = background, canvas = canvas, 
                                pad = pad)
        results.append(r)

    return results

if __name__ == '__main__': 

    inputs = sys.argv[1:]

    hist_file = 'all_plots.root'

    if len(inputs) == 1 and 'all' in inputs: 
        results = b_tag_plots(hist_file)
        raw_input('press ENTER')

        sys.exit(0)

    if not len(inputs) == 2: 
        sys.exit('usage: %s <signal> <background>' % 
                 os.path.split(sys.argv[0])[-1])

    import AtlasStyle
    AtlasStyle.SetAtlasStyle()

    sig_name = sys.argv[1]
    background_name = sys.argv[2]

    result = sig_over_background(hist_file, sig_name, background_name)

    raw_input('press enter to continue')
