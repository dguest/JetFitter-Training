#!/usr/bin/env python2.6

"""
makes correlation plots from test output ntuple

Author: Daniel Guest (dguest@cern.ch)
"""

from optparse import OptionParser, OptionGroup
import sys

if __name__ == '__main__': 
    
    usage = 'usage: %prog <performance ntuple> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False, 
        quiet = False, 
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('--max-entries', 
                      default = None)
    parser.add_option('-o', dest = 'output_file', 
                      default = 'cor_plots.root')
    parser.add_option('-q', dest = 'quiet')

    (options, args) = parser.parse_args(sys.argv[1:])

    if not len(args) == 1: 
        sys.exit(parser.get_usage())

    import ROOT 
    from jetnet.perf import correlation 

    ROOT.gROOT.SetBatch()
    ROOT.PyConfig.IgnoreCommandLineOptions = True

    ntuple_name = args[0]
    max_entries = None 
    if options.max_entries: 
        max_entries = int(options.max_entries)

    correlation.make_tag_wt_correlations(
        ntuple_name, 
        max_entries = max_entries, 
        output_name = options.output_file, 
        quiet = options.quiet
        )

