import numpy as np
import math 

def make_log_ticks(min_tick, max_tick): 
    """
    returns the ticks between min_tick and max_tick as (minor, major, labels)
    """
    round_min_tick = 10**math.floor(math.log10(min_tick))
    round_max_tick = 10**math.ceil(math.log10(max_tick))
    values = np.linspace(np.power(10,round_min_tick),
                         np.power(10,round_max_tick), num = 500)
    print values
    values = np.log10(values)
    print values
    spread = max_tick - min_tick
    good_index = np.flatnonzero(np.diff(values) > spread / 20) + 1
    if not 0 in good_index: 
        good_index = np.append(good_index,[0])
    values = values[good_index]
    values = values[np.flatnonzero( (min_tick < values) & 
                                    (values < max_tick))]
    labels = ['$10^{{ {} }}$'.format(np.log10(x)) for x in values]
    
    minors = np.logspace(np.log10(round_min_tick),
                         np.log10(round_max_tick), num = 49)
    minors = minors[np.flatnonzero(np.diff(minors) > (spread / 100.0))]
    minors = minors[np.flatnonzero( (min_tick < minors) & 
                                    (minors < max_tick))]
    return minors, values, labels
