import numpy as np
import math 

def make_log_ticks(min_tick, max_tick): 
    """
    returns the ticks between min_tick and max_tick as (minor, major, labels)
    """
    low_10_pow = math.floor(math.log10(min_tick))
    high_10_pow = math.ceil(math.log10(max_tick))
    pow_10_range = high_10_pow - low_10_pow
    values = np.linspace(low_10_pow, high_10_pow, 
                         num = int(pow_10_range + 1) ) 
    log_min = math.log10(min_tick)
    log_max = math.log10(max_tick)

    mapped_range = (log_max - log_min) 
    slop = mapped_range / pow_10_range
    transform = np.poly1d([ mapped_range / pow_10_range,  
    major_frac = np.array(
    return log_min, log_max

    spread = max_tick - min_tick
    good_index = np.flatnonzero(np.diff(values) > spread / 20) + 1
    if not 0 in good_index: 
        good_index = np.append(good_index,[0])
    values = values[good_index]
    values = values[np.flatnonzero( (min_tick < values) & 
                                    (values < max_tick))]
    labels = ['$10^{{ {} }}$'.format(np.log10(x)) for x in values]
    
    minors = np.linspace(np.exp(round_min_tick),
                         np.exp(round_max_tick), num = 49)
    minors = np.log(minors)
    print minors
    minors = minors[np.flatnonzero(np.diff(minors) > (spread / 100.0))]
    minors = minors[np.flatnonzero( (min_tick < minors) & 
                                    (minors < max_tick))]
    return minors, values, labels
