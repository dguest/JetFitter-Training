#!/usr/bin/env python2.6

import sys, os
from jetnet.perf import performance as p

if __name__ == '__main__': 

    inputs = sys.argv[1:]

    if not len(inputs) == 2: 
        sys.exit('usage: %s <file> <signal>' % 
                 os.path.split(sys.argv[0])[-1])

    hist_file = inputs[0]

    signal = inputs[1]
    for do_normalize in [False, True]: 
        results = p.b_tag_plots(hist_file, normalize = do_normalize, 
                                signal = signal)

        raw_input('press ENTER')

        out_spec = '_normalized' if do_normalize else ''

        output_name = '%s_perf_result%s' % (signal,out_spec)

        for output_format in ['.pdf','.png']: 
            results[0].Print(output_name + output_format)

    sys.exit(0)

