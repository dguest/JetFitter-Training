#!/usr/bin/env python2.7

from jetnet import pynn, utils
import sys, argparse

from ROOT import TFile

if __name__ == '__main__': 
    if not ( 3 <= len(sys.argv) <= 4 ): 
        sys.exit('motheruker: <in file> <nn file> [<output file>]')

    input_file = sys.argv[1]
    nn_file = sys.argv[2]
    try: 
        output_file = sys.argv[3]
    except IndexError: 
        output_file = '.'.join(input_file.split('.')[:-1]) + '_aug.root'
        print 'making', output_file
    tree = 'SVTree'

    the_file = TFile(input_file)
    the_tree = the_file.Get(tree)
    all_vars_in_tree = utils.get_leaves_in_tree(the_tree)
    
    pynn.augment_tree(
        in_file = input_file, 
        nn_file = nn_file, 
        out_file = output_file, 
        ints = all_vars_in_tree['Int_t'] , 
        doubles = all_vars_in_tree['Double_t'] , 
        )
    
