#!/usr/bin/env python2.6
import AtlasStyle
from jetnet.perf import rejection as re
import sys, os


def make_performance_plots(ntuple_file_name, do_batch = True): 
    from os.path import splitext, basename, dirname
    save_dir = '%splots_%s' % (dirname(ntuple_file_name), 
                             splitext(ntuple_file_name)[0])


    all_canvas = re.make_plots_from(ntuple_file_name)

    
    if not do_batch: 
        raw_input('press enter')
    
    formats = ['.pdf','.png']
    if save_dir: 
        if not os.path.isdir(save_dir): 
            os.makedirs(save_dir)

        for ext in formats: 
            for plot in all_canvas: 
                fullname = plot.GetName() + ext
                fullpath = os.path.join(save_dir,fullname)
                print 'printing to %s' % fullpath
                plot.Print(fullpath)


if __name__ == '__main__': 
    
    if len(sys.argv) != 2: 
        sys.exit('usage: %s <testing ntuple>' % 
                 os.path.basename(sys.argv[0]))

    file_name = sys.argv[1]
    if file_name.split('.')[-1] == 'root': 
        make_performance_plots(ntuple_file_name)
        sys.exit(0)
        
    files = []
    with open(file_name) as the_list: 
        for line in the_list: 
            files.append(line.strip())

    try: 
        array_id = int(os.environ['PBS_ARRAYID'])
        the_file = files[array_id]
        print 'making plots for %s' % the_file
        make_performance_plots(the_file)
    except KeyError: 
        for the_file in files: 
            make_performance_plots(the_file)
