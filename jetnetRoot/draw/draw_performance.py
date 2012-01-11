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
                        canvas = None, pad = None, sample = 'test', 
                        normalize = False): 
    search_string = '_%s_over_%s_' % (signal, background)

    matches = []
    
    if isinstance(root_file,str): 
        root_file = TFile(root_file)

    for key in root_file.GetListOfKeys(): 
        obj = key.ReadObj()
        if search_string in obj.GetName(): 
            if sample in obj.GetName(): 
                matches.append(obj)

    t = TROOT
    color_dict = { 
        'charms': t.kOrange, 
        'lights': t.kBlue, 
        'bottoms': t.kRed
        }

    labels = (
        'graph',
        '#frac{%s}{%s + %s}' % (signal,signal,background), 
        '%s entries' % sample
        )
    
    if canvas is None: 
        canvas = TCanvas(search_string,';'.join(labels),100,100,600,400)

    if pad: 
        canvas.cd(pad)
    draw_string = ''

    if not matches: 
        raise NoMatchError('ugh',signal = signal, background = background)
                           
    legend = TLegend(0.6,0.7,0.9,0.9)
    legend.SetFillStyle(0)
    legend.SetBorderSize(0)
    
    for hist in matches: 

        print 'drawing %s' % hist.GetName()
        sig_name = hist.GetName().split('_')[0]
        color = color_dict[sig_name]
        hist.SetTitle(';'.join(labels))
        hist.SetLineColor(color)
        if normalize == True: 
            hist.DrawNormalized(draw_string)
        else: 
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

def b_tag_plots(hist_file, normalize = False, signal = 'bottom'): 
    plots = []

    all_backgrounds = ['light','charm','bottom']
    all_backgrounds.remove(signal)

    for background in all_backgrounds: 
        for sample in ['train','test']: 
            plots.append( (signal, background, sample) )

    import AtlasStyle
    AtlasStyle.SetAtlasStyle()


    canvas = TCanvas('performance','performance',100,100,600*2,400*2)
    canvas.Divide(2,2)

    results = []
    for pad_index, (signal, background, sample) in enumerate(plots): 
        pad = pad_index + 1
        r = sig_over_background(root_file = hist_file, signal = signal, 
                                background = background, canvas = canvas, 
                                sample = sample, normalize = normalize, 
                                pad = pad)
        results.append(r)

    return results

if __name__ == '__main__': 

    inputs = sys.argv[1:]

    hist_file = 'all_plots.root'

    if len(inputs) == 1: 
        signal = inputs[0]
        for do_normalize in [False, True]: 
            results = b_tag_plots(hist_file, normalize = do_normalize, 
                                  signal = signal)

            raw_input('press ENTER')

            out_spec = '_normalized' if do_normalize else ''

            output_name = '%s_perf_result%s' % (signal,out_spec)

            for output_format in ['.pdf','.png']: 
                results[0].Print(output_name + output_format)

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
