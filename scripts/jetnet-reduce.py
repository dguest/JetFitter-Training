#!/usr/bin/env python2.7
# Author: Daniel Guest (dguest@cern.ch)

"""
reduce dataset consisting of files in <file list> 
"""

# hide this godawful abomination of a framework
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True

from jetnet import pyprep
import os, sys
from warnings import warn
from optparse import OptionParser, OptionGroup


if __name__ == '__main__': 

    usage = 'usage: %prog <file list> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False
        )

    parser.add_option('--test', action = 'store_true')
    parser.add_option('-o', dest = 'output_file', 
                      default = None, 
                      help = 'name of output file, defaults to '
                      '<file list>.root')

    (options, args) = parser.parse_args(sys.argv[1:])
    
    if len(args) != 1: 
        sys.exit(parser.get_usage())

    input_file_list = args[0]
    input_files = []
    with open(input_file_list) as file_list: 
        for line in file_list: 
            input_files.append(line.strip())

    if options.output_file is None: 
        rds_path = os.path.splitext(input_file_list)[0] + '.root'
        if os.path.exists(rds_path): 
            sys.exit('%s already exists' % rds_path)
    else: 
        rds_path = options.output_file

    double_variables = [
        'energyFraction', 
        'significance3d', 
        'meanTrackRapidity', 
        'maxTrackRapidity', 
        'minTrackRapidity', 
        ]
    int_variables = [ 
        'nVTX', 
        'nTracksAtVtx', 
        'nSingleTracks', 
        ]
    observer_discriminators = ['IP2D','IP3D','SV1','COMB']
    jet_collection = 'AntiKt4TopoEMJets'

    pyprep.prep_ntuple(input_files = input_files, 
                       double_variables = double_variables, 
                       int_variables = int_variables, 
                       observer_discriminators = observer_discriminators, 
                       jet_collection = jet_collection, 
                       output_file = rds_path, 
                       debug = options.test)

