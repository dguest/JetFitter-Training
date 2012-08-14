import numpy as np
import os, sys, warnings, cPickle, itertools
import glob
from jetnet.cxxprofile import pro2d, profile_fast

"""
routine to draw b-rejection vs c- or l-rejection plots
"""

_tags = ['bottom','charm','light']


###### TODO: move these into RejRejPlot ######

def _get_bins(file_name, hist_name): 
    
    from ROOT import TFile
    root_file = TFile(file_name)
    
    hist = root_file.Get(hist_name)
    if not hist: 
        raise IOError('no {} found in {}'.format(hist_name, file_name))

    x_bins = hist.GetNbinsX()
    y_bins = hist.GetNbinsY()
    
    bins = np.zeros((x_bins,y_bins))

    for y_bin in xrange(y_bins): 
        for x_bin in xrange(x_bins): 
            bins[x_bin,y_bin] = hist.GetBinContent(x_bin + 1,y_bin + 1)

    return bins

def _get_integrals_fast(bins): 
    """
    much faster and simpler way to compute integrals, uses numpy cumsum
    """

    if min(bins.shape) == 1: 
        flatsum = np.cumsum(bins.flat[::-1])[::-1]
        sums = np.subtract(*np.meshgrid(flatsum,flatsum))
        assert sums.shape[0] == sums.shape[1]
        return sums

    else: 
        y_sums = np.cumsum(bins[:,::-1], axis = 1)[:,::-1]
        sums = np.cumsum(y_sums[::-1,:], axis = 0)[::-1,:]
        return sums


def _max_noninf(array): 
    return np.amax(array[np.nonzero(np.isfinite(array))])

def _loop_over_entries(x_bins, y_bins, used_eff, n_out_bins): 
    """
    builds max efficiency at rejection by looping over all points in 
    cut grid. For small grids this should be faster. 
    """
    sys.stdout.write('finding eff at rejection... ')
    sys.stdout.flush()

    valid_x = (x_bins >= 0) & (x_bins < n_out_bins) 
    valid_y = (y_bins >= 0) & (y_bins < n_out_bins)

    valid_indices = np.flatnonzero(valid_x & valid_y)

    x_bins = x_bins[valid_indices]
    y_bins = y_bins[valid_indices]
    used_eff = used_eff[valid_indices]

    eff_array = np.zeros((n_out_bins,n_out_bins))
    for x_bin, y_bin, z in zip(x_bins, y_bins, used_eff): 
        # y_bin comes first because that's what imshow wants... 
        eff_array[y_bin,x_bin] = max(z, eff_array[y_bin,x_bin])

    sys.stdout.write('done\n')
    return eff_array

def _get_rejrej_array(flat_eff, flat_x, flat_y, x_range=None, y_range=None): 
    indices = np.nonzero( (flat_eff > 0.005) & 
                          np.isfinite(flat_x) & 
                          np.isfinite(flat_y)
                          )

    used_x = np.log10(flat_x[indices])
    used_y = np.log10(flat_y[indices])
    used_eff = flat_eff[indices]

    if not x_range: 
        # allow 1% safety margin on max value
        max_x = _max_noninf(used_x) * 1.0001
        min_x = np.min(used_x)
    else: 
        min_x, max_x = x_range

    if not y_range: 
        max_y = _max_noninf(used_y) * 1.0001
        min_y = np.min(used_y)
    else: 
        min_y, max_y = y_range

    n_out_bins = 100

    x_bin_values = np.linspace(min_x, max_x, n_out_bins)
    x_bins = np.digitize(used_x, bins = x_bin_values) - 1 # no underflow

    y_bin_values = np.linspace(min_y, max_y, n_out_bins)
    y_bins = np.digitize(used_y, bins = y_bin_values) - 1 # no underflow

    make_eff_array = _loop_over_entries # the other method seems slower

    eff_array = make_eff_array(x_bins, y_bins, used_eff, n_out_bins)
    
    return eff_array, (min_x, max_x), (min_y, max_y)


# TODO: autogenerate this? 
_tagger_bounds = {
    'discriminatorMV1': (0,1), 
    'logBcCOMBNN_SVPlus_rapLxy':(-5, 10), 
    'logBuCOMBNN_SVPlus_rapLxy':(-6, 12) , 
    'logCbCOMBNN_SVPlus_rapLxy':(-8, 4), # should just be Bc reversed?
    'logCuCOMBNN_SVPlus_rapLxy':(-5, 8),  
    'logBcJetFitterCOMBNN': (-5, 10), 
    'logBuJetFitterCOMBNN': (-6, 12), 
    'logCbJetFitterCOMBNN': (-8, 4), 
    'logCuJetFitterCOMBNN': (-5, 8) , 
    }


class RejRejPlot(object): 
    """
    Keeps track of the data (and cache) files associated with a given 
    rejecton plot.

    Adds an entry for the needed root histograms to hist_list when created, 
    under the 'requested' entry. 

    When compute() is called, will look for the  entry under 'built' in
    the same pickle. 

    -Does not build hists from ntuple. 
    -Does not make plots. 
    -Does not store lots of info internally. 

    Uses: _get_rejrej_array, _get_bins, _get_integrals_fast

    """
    def __init__(self, tagger = 'JetFitterCOMBNN', signal = 'charm', 
                 bins = 2000, x_range = None, y_range = None, 
                 window_discrim = False, cache = 'cache', 
                 hist_list = 'hist_list.pkl', 
                 parent_ntuple = 'perf_ntuple.root'): 

        win_dir = 'window' if window_discrim else 'twocut'
        bins_name = 'bins{}'.format(bins)
        integrals_dir = os.path.join(cache, tagger, win_dir, bins_name )
        cache_path = os.path.join(integrals_dir, signal)
        if not os.path.isdir(cache_path): 
            os.makedirs(cache_path)

        self._cache_path = cache_path
        self._tagger = tagger
        self._signal = signal 
        self._x_range = x_range
        self._y_range = y_range
        self._window_discrim = window_discrim

        rejrej_pickle = os.path.join(cache_path, 'rejrej.pkl')
        self._rejrej_pickle = rejrej_pickle

        remain_flavs = [f for f in _tags if f != signal]
        self._x_flav, self._y_flav = remain_flavs

        # bins things
        integrals_name = 'integrals_{bins}bins.pkl'.format(bins = bins)
        integral_pkl = os.path.join(integrals_dir, integrals_name)
        self._integrals_pickle = integral_pkl
        self._bins = bins

        root_file_name = 'root_{}_hists.root'.format(
            '1d' if window_discrim else '2d')
        self._root_cache = os.path.join(cache, root_file_name)


        self.trigger_recalc = False

        root_hists_listing_pkl = os.path.join(cache, hist_list)
        self._root_hists_listing_pkl = root_hists_listing_pkl
        hist_location = self._check_hist_list()
        if not hist_location: 
            self.trigger_recalc = True
        

    def compute(self):
        """
        Build the plot. Returns the path to the .pkl file where it's stored.
        
        Make sure you've called HistBuilder on the hist_list. 
        """
        if not os.path.isfile(self._rejrej_pickle): 
            self._build_rejrej()
        else: 
            mismatch = self._check_plot_mismatch()
            if mismatch > 1e-3: 
                print 'found range mismatch in {}, rebinning'.format(
                    self._rejrej_pickle)
                self._build_rejrej()

        return self._rejrej_pickle

    def _check_plot_mismatch(self): 
        
        with open(self._rejrej_pickle) as pkl: 
            old_plot = cPickle.load(pkl)
        ranges = [ 
            [self._x_range, old_plot['x_range']], 
            [self._y_range, old_plot['y_range']], 
            ]
        
        total_diff = 0
        total_sum = 0
        for r in ranges: 
            if r[0] is None: 
                continue
            for pt in zip(*r): 
                if (pt[0] is None) != (pt[1] is None): 
                    return True
                diff = (max(pt) - min(pt))**2
                add = sum(pt)**2
                total_diff += diff
                total_sum += add

        if not total_diff: 
            return False
        return float(total_diff) / float(total_sum) 


    def _check_hist_list(self): 
        list_file = self._root_hists_listing_pkl

        if os.path.isfile(list_file): 
            with open(list_file) as pkl: 
                hist_listing = cPickle.load(pkl)
        else: 
            hist_listing = { 
                'requested': set(), 
                'built': {}, 
                }
            
        this_hist_index = (self._tagger, self._bins, self._signal, 
                           self._window_discrim)
        if this_hist_index in hist_listing['built']: 
            return hist_listing['built'][this_hist_index]
        
        if not this_hist_index in hist_listing['requested']: 
            hist_listing['requested'].add(this_hist_index)

        with open(list_file,'w') as pkl: 
            cPickle.dump(hist_listing, pkl)
        return False

    def _build_rejrej(self): 
        if not os.path.isfile(self._integrals_pickle): 
            self._build_integrals()
        print 'building rejrej plot for', self._tagger
        
        with open(self._integrals_pickle) as pkl: 
            integrals = cPickle.load(pkl)
        
        sig = self._signal
        eff = integrals[sig] / integrals[sig].max()

        flav_x, flav_y = [t for t in _tags if t != sig]

        old_warn_set = np.seterr(divide = 'ignore') 
        rej_x = integrals[flav_x].max() / integrals[flav_x]
        rej_y = integrals[flav_y].max() / integrals[flav_y]
        np.seterr(**old_warn_set)
        
        eff = eff.flatten()
        rej_x = rej_x.flatten()
        rej_y = rej_y.flatten()
        
        eff_array, x_range, y_range = _get_rejrej_array(
            eff,rej_x, rej_y, 
            x_range=self._x_range, y_range=self._y_range)

        out_dict = { 
            'eff': eff_array, 
            'x_range': x_range, 
            'y_range': y_range, 
            'signal': sig, 
            'x_bg': flav_x, 
            'y_bg': flav_y, 
            'tagger': self._tagger, 
            'bins': self._bins, 
            }
        
        with open(self._rejrej_pickle,'w') as pkl: 
            print 'saving {}'.format(self._rejrej_pickle)
            cPickle.dump(out_dict,pkl)
    
    def _build_integrals(self): 
        """
        this should read in a root file and create the integrals cache
        """

        the_root_file, hists = self._check_hist_list()
            
        # bins = _get_bins(

        int_dict = {}
        for hist in hists: 
            print 'loading {} in {}'.format(hist, the_root_file)
            bins = _get_bins(the_root_file, hist)
            integral = _get_integrals_fast(bins)
            
            tag = hist.split('_')[-1]
            assert tag in _tags, '{} not found in {}'.format(tag, _tags)
            int_dict[tag] = integral

        with open(self._integrals_pickle,'w') as pkl: 
            print 'saving {}'.format(self._integrals_pickle)
            cPickle.dump(int_dict, pkl)


class HistBuilder(object): 
    """
    Builds hists requested in requests_pickle, which is loaded by
    the constructor. 
    Updates the pickle with locations and names of the built histograms. 
    
    The 'built' entry of the requests pickle contains a dictionary of 
    (file, {flavor:name}) tuples. 
    """

    def __init__(self,requests_pickle, 
                 tagger_bounds = _tagger_bounds): 
        try: 
            self._initialize(requests_pickle, tagger_bounds)
        except HistBuilder.BadTaggerError as bad: 
            with open(requests_pickle,'r') as pkl: 
                pk_dict = cPickle.load(pkl)
                
            pk_dict['requested'].remove(bad.id)
            with open(requests_pickle,'w') as pkl: 
                cPickle.dump(pk_dict,pkl)
            raise

    def _initialize(self, requests_pickle, tagger_bounds): 
        with open(requests_pickle) as pkl: 
            status_dict = cPickle.load(pkl)

        self._requests_pickle = requests_pickle
            
        requested = status_dict['requested']
        built = set(status_dict['built'].keys())
    
        unbuilt = requested - built
        
        flavor_to_char = {
            'light':'u', 
            'charm':'c', 
            'bottom':'b'
            }
    
        onedim_inputs = {}
        twodim_inputs = {}
        for id_tuple in unbuilt: 
            print 'booking {} to build'.format(id_tuple)
            short_tagger, bins, signal, window_discrim = id_tuple

            all_taggers = tagger_bounds.keys()
            rej_flavors = [f for f in _tags if f != signal]
            signal_char = flavor_to_char[signal]
    
            if window_discrim: 
                tagger_matches = []
                for t in all_taggers: 
                    if not short_tagger in t: 
                        continue
                    elif 'logBu' in t or 'discriminator' in t: 
                        tagger_matches.append(t)
    
                if len(tagger_matches) != 1: 
                    raise BadTaggerError("taggers {} all match {}".format(
                            tagger_matches, short_tagger), id_tuple)
                full_tagger = tagger_matches[0]
                bounds = tagger_bounds[full_tagger]
                tagger_tuple = (full_tagger, bins, bounds[0],bounds[1])
                onedim_inputs[id_tuple] = tagger_tuple
            else: 
                rej_chars = [flavor_to_char[f] for f in rej_flavors]
                x_chars = signal_char.upper() + rej_chars[0]
                y_chars = signal_char.upper() + rej_chars[1]
                
                x_matches = []
                y_matches = []
                for t in all_taggers: 
                    if 'log' + x_chars in t and short_tagger in t: 
                        x_matches.append(t)
                    if 'log' + y_chars in t and short_tagger in t: 
                        y_matches.append(t)
                if len(x_matches) != 1 or len(y_matches) != 1: 
                    raise BadTaggerError(
                        "taggers {} {} match {}".format(
                            x_matches, y_matches, short_tagger, signal), 
                        id_tuple)
                x_tagger = x_matches[0]
                y_tagger = y_matches[0]
                x_bounds = tagger_bounds[x_tagger]
                y_bounds = tagger_bounds[y_tagger]
                x_tagger_tuple = (x_tagger, bins, x_bounds[0], x_bounds[1])
                y_tagger_tuple = (y_tagger, bins, y_bounds[0], y_bounds[1])
                twodim_inputs[id_tuple] = (x_tagger_tuple, y_tagger_tuple) 
                                      
        self._onedim_inputs = onedim_inputs
        self._twodim_inputs = twodim_inputs
    
    def build_1d_hists(self, input_ntuple, out_dir = ''): 
        """
        Call profiling routines to build the histogram files. 
        Record the hists as built in the requests_tuple
        """
        if not self._onedim_inputs: 
            return False
        built_1d_hists = {}
        onedim_input_list = []
        if not out_dir: 
            out_dir = os.path.split(self._requests_pickle)[0]

        cache_files = glob.glob(out_dir + '/*')
        onedim_out_file = ''
        for x in xrange(100): 
            f_name = 'onedim_cache{}.root'.format(x)
            onedim_out_file = os.path.join(out_dir, f_name)
            if not onedim_out_file in cache_files: 
                break
        assert onedim_out_file, 'could not name output file'

        for id_tuple, inputs in self._onedim_inputs.items(): 
            onedim_input_list.append(inputs)
            hist_base_name = inputs[0] 
            all_hists = [hist_base_name + '_' + t for t in _tags]
            built_1d_hists[id_tuple] = (onedim_out_file, all_hists)
        
        print 'profiling 1d hists'
        profile_fast(input_ntuple, tree = 'SVTree', 
                     out_file = onedim_out_file, 
                     doubles = onedim_input_list, 
                     tags = _tags, 
                     show_progress = True)
            
        with open(self._requests_pickle) as pkl: 
            status_dict = cPickle.load(pkl)

        status_dict['built'].update(built_1d_hists)
        with open(self._requests_pickle,'w') as pkl: 
            cPickle.dump(status_dict, pkl)

    def build_2d_hists(self, input_ntuple, out_dir = ''): 
        """
        Call profiling routines to build the histogram files. 
        Record the hists as built in the requests_tuple
        """
        if not self._twodim_inputs: 
            return False
        built_hists = {}
        input_list = []
        if not out_dir: 
            out_dir = os.path.split(self._requests_pickle)[0]

        cache_files = glob.glob(out_dir + '/*')

        out_file = ''
        for x in xrange(100): 
            f_name = 'twodim_cache{}.root'.format(x)
            out_file = os.path.join(out_dir, f_name)
            if not out_file in cache_files: 
                break
        assert out_file, 'could not name output file'

        for id_tuple, inputs in self._twodim_inputs.items(): 
            input_list.append(inputs)
            first_hist_name = inputs[1][0]
            second_hist_name = inputs[0][0]
            compound_hist_name = first_hist_name + '_vs_' + second_hist_name
            all_hists = [compound_hist_name + '_' + t for t in _tags]
            built_hists[id_tuple] = (out_file, all_hists)
        
        print 'profiling 2d hists'
        reported_out, reported_hists = pro2d(
            input_ntuple, tree = 'SVTree', 
            out_file = out_file, 
            plots = input_list, 
            tags = _tags, 
            show_progress = True)
        
            
        with open(self._requests_pickle) as pkl: 
            status_dict = cPickle.load(pkl)

        status_dict['built'].update(built_hists)
        with open(self._requests_pickle,'w') as pkl: 
            cPickle.dump(status_dict, pkl)

class BadTaggerError(LookupError): 
    """
    Exception used by HistBuilder when it can't construct a requested hist. 
    Will be caught temporarily and used to remove the offending entry. 
    """
    def __init__(self, message, tagger_tuple = None): 
        super(BadTaggerError,self).__init__(message)
        self.id = tagger_tuple
