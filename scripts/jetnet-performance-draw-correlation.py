#!/usr/bin/env python2.6

"""
draws correlation plots from test output ntuple

Author: Daniel Guest (dguest@cern.ch)
"""

from optparse import OptionParser, OptionGroup
import sys, random

if __name__ == '__main__': 
    
    usage = 'usage: %prog <correlation plots> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        quiet = False, 
        )

    parser.add_option('-o', dest = 'output_file', 
                      default = 'cor_plots.root')
    parser.add_option('-q', dest = 'quiet')
    parser.add_option('-b', dest = 'batch')

    (options, args) = parser.parse_args(sys.argv[1:])

    if not len(args) == 1: 
        sys.exit(parser.get_usage())

    import ROOT 
    from ROOT import TH2D, TIter, TFile, TCanvas

    if options.batch: 
        ROOT.gROOT.SetBatch()

    flavors = ['bottom', 'charm', 'light']

    root_file = TFile(args[0])
    ROOT.gROOT.cd()

    keys = root_file.GetListOfKeys()
    denom_hists = dict()
    num_hists = dict( (s, dict()) for s in flavors)
    for key in TIter(keys): 
        name = key.GetName()
        hist = key.ReadObj().Clone(str(random.random()))

        subnames = name.split('_')
        axes = '_'.join(subnames[:3])
        try: 
            tag = subnames[3]
            num_hists[tag][axes] = hist
        except IndexError: 
            denom_hists[axes] = hist


    import AtlasStyle
    ROOT.gStyle.SetPalette(1)

    for axes, denom in denom_hists.iteritems(): 

        for tag in flavors: 
            ratio = num_hists[tag][axes].Clone(str(random.random()))
            ratio.Divide(denom)
            ratio.SetTitle(tag)
            
            canvas = TCanvas(tag,tag,100,100,600,400)
            ROOT.gPad.SetRightMargin(0.2)
            ratio.Draw('colz')
            if not options.batch: 
                raw_input('press enter')
    
