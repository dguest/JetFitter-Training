"""
provides a python interface to the ntup_from_d3pd function 
"""
from _ctuple_maker import _ntup_from_d3pd

def ntup_from_d3pd(input_files, input_tree, jet_collection, output_file, 
                   limits={}, flags=''): 
    """
    very thin python wrapper for the ntuple builder. 

    Limits is a dict of (string, float) key / value pairs. Keys: 
        -min_pt
        -max_pt
        -max_abs_eta

    Flags: 
        u: skip taus
        f: fabrizios silly tagger (mv1 plus mv1c)
        m: save extra mv1 branches (mv1 and mv1c)
    """
    _ntup_from_d3pd(input_files, input_tree, jet_collection, 
                    output_file, limits, flags)
