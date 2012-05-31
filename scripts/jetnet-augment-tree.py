#!/usr/bin/env python2.7

from jetnet import pynn, utils
import sys

if __name__ == '__main__': 
    if len(sys.argv) != 3: 
        sys.exit('motheruker: <in file> <nn file>')

    input_file = sys.argv[1]
    nn_file = sys.argv[2]
    output_file = 'testout.root'
    tree = 'SVTree'

    the_file = TFile(input_file)
    the_tree = the_file.Get(tree)
    all_vars_in_tree = utils.get_leaves_in_tree(the_tree)
    
    pynn.augment_tree(
        in_file = in_file, 
        nn_file = nn_file, 
        out_file = output_file, 
        ints = all_vars_in_tree['Int_t'] , 
        doubles = all_vars_in_tree['Double_t'] , 
        max_entries = 500, 
        )
    
