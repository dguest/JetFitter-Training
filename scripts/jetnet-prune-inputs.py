#!/usr/bin/env python2.6
# Author: Daniel Guest (dguest@cern.ch)
"""
runs n-1 variable nn training. 

directories are created with the current directory as the base
"""

import sys, os
import multiprocessing
from optparse import OptionParser, OptionGroup
from copy import deepcopy

from jetnet.dirs import OverwriteError
from jetnet import pynn

class NNTrainingProcess(multiprocessing.Process): 
    def __init__(self, reduced_dataset, output_directory, 
                 normalization = {}, 
                 nodes = None, 
                 flavor_weights = {}, 
                 debug = True, 
                 ): 
        super(NNTrainingProcess,self).__init__()

        self.reduced_dataset = reduced_dataset
        self.output_directory = output_directory
        self.normalization = normalization
        if nodes is None:
            self.nodes = (20, 10)
        else: 
            self.nodes = nodes
        self.flavor_weights = flavor_weights

        self.debug = debug
    
    def run(self): 

        if not os.path.isdir(self.output_directory): 
            if not self.debug: 
                os.mkdir(self.output_directory)
        elif glob.glob(self.output_directory + '/*.root*'): 
            raise OverwriteError('root files found in %s' % 
                                 self.output_directory)

        
        pynn.trainNN(reduced_dataset = self.reduced_dataset, 
                     output_directory = self.output_directory, 
                     n_iterations = 10000, 
                     dilution_factor = 2, 
                     normalization = self.normalization, 
                     nodes = self.nodes, 
                     flavor_weights = self.flavor_weights, 
                     debug = self.debug)


    
def build_trainer_with_missing_vars(reduced_dataset, 
                                    missing_vars): 

    vars_dict = {
        "nVTX"                : ( -0.30, 1.0 / 0.50) , 
        "nTracksAtVtx"        : ( -1.00, 1.0 / 1.60) , 
        "nSingleTracks"       : ( -0.20, 1.0 / 0.50) , 
        "energyFraction"      : ( -0.23, 1.0 / 0.33) , 
        "mass"                : ( - 974, 1.0 / 1600) , 
        "significance3d"      : ( -   7, 1.0 / 14.0) , 
        "discriminatorIP3D"   : ( - 6.3, 1.0 /  6.0) , 
        "cat_pT"              : ( - 3.0, 1.0 /  3.0) , 
        "cat_eta"             : ( - 1.0,        1.0) , 
        }

    for var in missing_vars: 
        new_dict.pop(var)

    print 'work do here' 
        


if __name__ == '__main__': 

    usage = 'usage: %prog <reduced dataset> [options]'
    description = __doc__

    parser = OptionParser(usage = usage, description = description)
    parser.set_defaults(
        test = False, 
        )

    parser.add_option('--test', action = 'store_true')

    (options, args) = parser.parse_args(sys.argv)
    
    n_processors = multiprocessing.cpu_count()

    print 'found %i processors' % n_processors

    nn_training = NNTrainingProcess(
        reduced_dataset = 'tredu', 
        output_directory = 'nnrell', 
        )
    nn_training.start()
    nn_training.join()

