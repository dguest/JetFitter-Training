#!/usr/bin/env python2.6

import sys, os
from ROOT import TFile, TH1F, TCanvas, TROOT

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

def sig_over_background(root_file, signal = 'charm', background = 'light'): 
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
    
    canvas = TCanvas(search_string,';'.join(labels),100,100,600,400)
    draw_string = ''

    if not test_matches + train_matches: 
        raise NoMatchError('ugh',signal = signal, background = background)
                           

    for hist in test_matches: 

        print 'drawing %s' % hist.GetName()
        color = color_dict[hist.GetName().split('_')[0]]
        hist.SetTitle(';'.join(labels))
        hist.SetLineColor(color)
        hist.Draw(draw_string)
        draw_string = 'same'

    canvas.file = root_file
    return canvas

if __name__ == '__main__': 

    import AtlasStyle
    AtlasStyle.SetAtlasStyle()

    inputs = sys.argv[1:]

    if not len(inputs) == 2: 
        sys.exit('usage: %s, <signal> <background>' % 
                 os.path.split(sys.argv[0])[-1])

    hist_file = 'all_plots.root'

    sig_name = sys.argv[1]
    background_name = sys.argv[2]

    result = sig_over_background(hist_file, sig_name, background_name)

    raw_input('press enter to continue')
