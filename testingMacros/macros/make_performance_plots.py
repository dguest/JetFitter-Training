#!/usr/bin/env python2.6
import AtlasStyle
from jetnet.perf import rejection as re
import sys, os

if __name__ == '__main__': 
    
    if len(sys.argv) != 2: 
        sys.exit('usage: %s <testing ntuple>' % 
                 os.path.basename(sys.argv[0]))

    from os.path import splitext, basename, dirname
    ntuple_file_name = sys.argv[1]
    save_dir = '%splots_%s' % (dirname(ntuple_file_name), 
                             splitext(ntuple_file_name)[0])


    all_canvas = re.make_plots_from(ntuple_file_name)

    

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


