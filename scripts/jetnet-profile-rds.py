#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)

"""
make profile hists for files in <reduced dataset> 
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from jetnet import profile
import os, sys
from warnings import warn
from optparse import OptionParser, OptionGroup


if __name__ == '__main__': 

    usage = 'usage: %prog <reduced dataset> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('-o', dest = 'output_file', 
                      default = None, 
                      help = 'name of output file, defaults to '
                      '<reduced dataset>_profile.root')

    (options, args) = parser.parse_args(sys.argv[1:])
    
    if len(args) != 1: 
        sys.exit(parser.get_usage())

    reduced_dataset = args[0]

    if options.output_file is None: 
        profile_path = os.path.splitext(reduced_dataset)[0] + '_profile.root'
        if os.path.exists(profile_path): 
            sys.exit('%s already exists' % profile_path)
    else: 
        rds_path = options.output_file

    profile.make_profile_file(reduced_dataset, 
                              profile_file = profile_path)

