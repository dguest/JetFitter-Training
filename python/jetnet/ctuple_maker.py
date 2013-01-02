"""
provides a python interface to the ntup_from_d3pd function 
"""
from _ctuple_maker import _ntup_from_d3pd

def ntup_from_d3pd(input_files, input_tree, jet_collection, output_file): 
    """
    very thin python wrapper for the ntuple builder
    """
    _ntup_from_d3pd(input_files, input_tree, jet_collection, output_file)
