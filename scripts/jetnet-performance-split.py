#!/usr/bin/env python2.6
"""
Author: Daniel Guest (dguest@cern.ch)
"""

from jetnet.perf import splittree, rejection 
import ROOT
from ROOT import TFile, gROOT
import sys, os, glob
import multiprocessing as proc

gROOT.SetBatch()
ROOT.PyConfig.IgnoreCommandLineOptions = True

values = [15,30,50,80,120]

def perf_tree_entries(file_path): 
    """
    used to filter out files with empty performance trees
    """
    f = TFile(file_path)
    t = f.Get('performance')
    return t.GetEntriesFast()

def proc_func(ds): 
    print 'working on %s' % ds
    rejection.make_plots_from(ds, max_events = 100000)

if __name__ == '__main__': 
    if len(sys.argv) != 2: 
        sys.exit('enter a file to run over')
    input_dataset = sys.argv[1]
    if not os.path.isfile(input_dataset): 
        sys.exit("what the hell is '%s'?" % input_dataset)

    output_dir = 'split_by_pt'
    if not os.path.isdir(output_dir): 
        os.mkdir(output_dir)
    existing_files = glob.glob(output_dir + '/*.root')
    if not existing_files: 
        datasets = splittree.split_tree(input_dataset, values = values, 
                                        out_file_dir = output_dir)
    else: 
        datasets = existing_files

    n_cpu = proc.cpu_count()

    worker_pool = proc.Pool(n_cpu)
    
    worker_pool.map(proc_func, filter(perf_tree_entries, datasets))

    # for subset in filter(perf_tree_entries,datasets): 
    #     print 'working on %s' % subset
    #     rejection.make_plots_from(subset, max_events = 10000)
