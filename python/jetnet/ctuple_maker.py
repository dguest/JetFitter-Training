"""
provides a python interface to the ntup_from_d3pd function 
"""
from _ctuple_maker import _ntup_from_d3pd

def ntup_from_d3pd(input_files, input_tree, jet_collection, output_file, 
                   limits={}): 
    """
    very thin python wrapper for the ntuple builder. 

    Limits is a dict of (string, float) key / value pairs. Keys: 
        -min_pt
        -max_pt
        -max_abs_eta
    """
    _ntup_from_d3pd(input_files, input_tree, jet_collection, 
                    output_file, limits)
