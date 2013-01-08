import numpy as np
import os, sys, warnings, cPickle, itertools
import glob
from jetnet.cxxprofile import pro2d, profile_fast
import h5py

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

def _get_hist_ranges(file_name, hist_name): 
    """
    """
    from ROOT import TFile
    root_file = TFile(file_name)
    
    hist = root_file.Get(hist_name)
    if not hist: 
        raise IOError('no {} found in {}'.format(hist_name, file_name))
    x_range = (hist.GetXaxis().GetBinLowEdge(1), 
               hist.GetXaxis().GetBinUpEdge(hist.GetNbinsX()))
    y_range = (hist.GetYaxis().GetBinLowEdge(1), 
               hist.GetYaxis().GetBinUpEdge(hist.GetNbinsX()))
    return (x_range, y_range)

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

    eff_array = np.ones((n_out_bins,n_out_bins)) * -1
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
    'antiUMv1': (0,1), 
    'antiBMv1': (-1,0), 
    'logCuMvcomb': (0,1), 
    'logCbMvcomb': (-1,0), 
    'discriminatorMV2': (-1.5,1), 
    'logBcCOMBNN_SVPlus_rapLxy':(-5, 10), 
    'logBuCOMBNN_SVPlus_rapLxy':(-6, 12) , 
    'logCbCOMBNN_SVPlus_rapLxy':(-8, 4), # should just be Bc reversed?
    'logCuCOMBNN_SVPlus_rapLxy':(-5, 8),  
    'logBcJetFitterCOMBNN': (-5, 10), 
    'logBuJetFitterCOMBNN': (-6, 12), 
    'logCbJetFitterCOMBNN': (-8, 4), 
    'logCuJetFitterCOMBNN': (-5, 8) , 
    'logBcSv1Ip3d': (-5, 10), 
    'logBuSv1Ip3d': (-6, 12), 
    'logCbSv1Ip3d': (-8, 4), 
    'logCuSv1Ip3d': (-5, 8) , 
    'logBcSv1Ip3dRapVx': (-5, 10), 
    'logBuSv1Ip3dRapVx': (-6, 12), 
    'logCbSv1Ip3dRapVx': (-8, 4), 
    'logCuSv1Ip3dRapVx': (-5, 8) , 
    'logBcJetFitterCharm': (-5, 10), 
    'logBuJetFitterCharm': (-6, 12), 
    'logCbJetFitterCharm': (-8, 4), 
    'logCuJetFitterCharm': (-5, 8) , 
    }

class DisplayCut(object): 
    """
    Keeps track of one set of cuts which will be mapped onto the 
    rejrej plane. 
    """
    def __init__(self, cut1, cut2): 
        self._cut_1 = cut1
        self._cut_2 = cut2

        self._xyz = None
        self._cut_ranges = None

        self.point_type = 'mo'

    def place(self, sig, bg_x, bg_y, cut_1_range, cut_2_range): 
        """
        calculates x,y,z coordinates (rej x, rej y, eff)
        
        NOTE: make sure the eff, rej_x, rej_y arrays are integrated
        """
        assert bg_x.shape == bg_y.shape
        npts_1, npts_2 = bg_x.shape

        c1_bin_bounds = np.linspace(*cut_1_range, num=(npts_1 + 1))
        c1_bin = np.digitize([self._cut_1], c1_bin_bounds) - 1

        c2_bin_bounds = np.linspace(*cut_2_range, num=(npts_2 + 1))
        c2_bin = np.digitize([self._cut_2], c2_bin_bounds) - 1

        if any(b < 0 for b in [c1_bin, c2_bin]): 
            raise ValueError("can't put a cut in the underflow bin")
              
        eff = float(sig[c1_bin, c2_bin] / sig.max())

        def get_rej(bkg_array): 
            array_val = bkg_array.max() / bkg_array[c1_bin, c2_bin]
            return float(array_val)
        rej_x, rej_y = [get_rej(ar) for ar in [bg_x, bg_y]]

        self._xyz = rej_x, rej_y, eff
        self._cut_ranges = (cut_1_range, cut_2_range)

    @property
    def xyz(self): 
        if self._xyz is None: 
            raise AttributeError("you haven't calculated xyz yet")
        return self._xyz
    
    @property
    def cut1(self): 
        return self._cut_1

    @property
    def cut2(self): 
        return self._cut_2

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

    _default_point_string = '({cut1},{cut2}) $\epsilon$:{eff:.3g}'

    def __init__(self, tagger = 'JetFitterCOMBNN', signal = 'charm', 
                 bins = 2000, x_range = None, y_range = None, 
                 window_discrim = False, cache = 'cache', 
                 hist_list = 'hist_list.pkl', 
                 parent_ntuple = 'perf_ntuple.root', 
                 debug=False): 

        self._debug = debug
        win_dir = 'window' if window_discrim else 'twocut'
        bins_name = 'bins{}'.format(bins)
        cache_path = os.path.join(cache, tagger, win_dir, bins_name, signal)
        if not os.path.isdir(cache_path): 
            os.makedirs(cache_path)

        self._cache_path = cache_path
        self._tagger = tagger
        self._signal = signal 
        self._x_range = x_range
        self._y_range = y_range
        self._window_discrim = window_discrim
        self._cuts_to_display = []

        self._id = (tagger, bins, signal, window_discrim)

        rejrej_pickle = os.path.join(cache_path, 'rejrej.pkl')
        self._rejrej_pickle = rejrej_pickle

        remain_flavs = [f for f in _tags if f != signal]
        self._x_flav, self._y_flav = remain_flavs

        # bins things
        integrals_name = 'integrals_{bins}bins.h5'.format(bins = bins)
        integral_pkl = os.path.join(cache_path, integrals_name)
        self._integrals_cache = integral_pkl
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
            return self._rejrej_pickle

        try: 
            mismatch = self._check_plot_mismatch()
            if mismatch > 1e-3: 
                print 'found range mismatch in {}, rebinning'.format(
                    self._rejrej_pickle)
                self._build_rejrej()
        except KeyError: 
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
        if self._debug: 
            print 'booked {} to {}'.format(this_hist_index, list_file)

    def _build_rejrej(self): 
        if not os.path.isfile(self._integrals_cache): 
            self._build_integrals()
        print 'building rejrej plot for', self._tagger

        integrals, ranges = self._get_integrals_and_ranges()

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
            'window_cut': self._window_discrim, 
            'cuts_to_display': self._cuts_to_display, 
            }
        
        with open(self._rejrej_pickle,'w') as pkl: 
            print 'saving {}'.format(self._rejrej_pickle)
            cPickle.dump(out_dict,pkl)

    def add_display_cut(self, x_cut, y_cut, 
                        plot_string=_default_point_string, 
                        ann_opts={}): 
        """
        adds a point to the rejrej plot. 
        FIXME: the x_cut, y_cut names are a misnomer: these are actually
        cut1 and cut2
        """
        integrals, ranges = self._get_integrals_and_ranges()

        sig = self._signal
        flav_x, flav_y = [t for t in _tags if t != sig]

        the_cut = DisplayCut(x_cut, y_cut)
        the_cut.plot_string = plot_string
        the_cut.ann_opts = ann_opts
        the_cut.place(integrals[sig], integrals[flav_x], integrals[flav_y], 
                      *ranges[sig])
        self._cuts_to_display.append(the_cut)

        out_dict = {'cuts_to_display': self._cuts_to_display}
        if os.path.isfile(self._rejrej_pickle): 
            with open(self._rejrej_pickle) as pkl: 
                out_dict = cPickle.load(pkl)
            out_dict['cuts_to_display'] = self._cuts_to_display
            
        with open(self._rejrej_pickle,'w') as pkl: 
            print 'updating', self._rejrej_pickle
            cPickle.dump(out_dict,pkl)

    def _get_integrals_and_ranges(self): 

        if not os.path.isfile(self._integrals_cache): 
            self._build_integrals()

        integrals = {}
        ranges = {}

        with h5py.File(self._integrals_cache) as data: 
            for name in data: 
                integrals[name] = np.array(data[name])
                atr = data[name].attrs
                x_range = (atr['x_min'], atr['x_max'])
                y_range = (atr['y_min'], atr['y_max'])
                ranges[name] = (x_range, y_range)

        return integrals, ranges

    def get_cut1_with_rejection(self, rejection=2.0): 
        """
        calculates the cut where we get a specified rejection 

        assumes no cut on the second variable
        """

        integrals, ranges = self._get_integrals_and_ranges()

        sig = self._signal
        flav_x, flav_y = [t for t in _tags if t != sig]

        eff = integrals[sig][:,0] / integrals[sig][:,0].max()
        bg_eff = integrals[flav_x][:,0] / integrals[flav_x][:,0].max()

        bg_offset = bg_eff - (1.0 / rejection)
        crossover_bins = np.where(bg_offset[:-1] * bg_offset[1:] < 0.0 )
        if len(crossover_bins) == 0: 
            raise ValueError('rejection value of {} can\'t be found'.format(
                    rejection))
        elif len(crossover_bins) > 1: 
            raise ValueError("found multiple crossover points: {}".format(
                    crossover_bins))
        
        n_bins = len(bg_eff)
        crossover_values = np.linspace(*ranges[flav_x][0], 
                                        num=(n_bins + 1))[1:-1]
        
        value = crossover_values[crossover_bins[0][0]]
        return value

    def get_cut2_with_rejection(self, rejection=2.0, cut1_value=0): 
        """
        calculates the cut where we get a specified rejection 

        assumes no cut on the second variable
        """

        integrals, ranges = self._get_integrals_and_ranges()

        sig = self._signal
        flav_x, flav_y = [t for t in _tags if t != sig]

        n_bins_x = integrals[flav_x].shape[0]
        cut1_bin_converter = BinValueCnv(n_bins_x,ranges[flav_x][0])
        x_bin = cut1_bin_converter.bin_from_value(cut1_value)

        eff = integrals[sig][x_bin,:] / integrals[sig][x_bin,:].max()
        bg_eff = np.squeeze(integrals[flav_y][x_bin,:] / 
                  integrals[flav_y][x_bin,:].max())

        bg_offset = bg_eff - (1.0 / rejection)
        crossover_bins = np.where(bg_offset[:-1] * bg_offset[1:] < 0.0 )
        if len(crossover_bins) == 0: 
            raise ValueError('rejection value of {} can\'t be found'.format(
                    rejection))
        elif len(crossover_bins) > 1: 
            raise ValueError("found multiple crossover points: {}".format(
                    crossover_bins))
        
        n_bins = len(bg_eff)
        crossover_values = np.linspace(*ranges[flav_y][1], 
                                        num=(n_bins + 1))[1:-1]
        return crossover_values[crossover_bins[0][0]]
        
    
    def _build_integrals(self): 
        """
        this should read in a root file and create the integrals cache
        """

        the_root_file, hists = self._check_hist_list()
            
        int_dict = {}
        ranges_dict = {}
        for hist in hists: 
            print 'loading {} in {}'.format(hist, the_root_file)
            try: 
                bins = _get_bins(the_root_file, hist)
            except IOError as error: 
                with open(self._root_hists_listing_pkl) as pkl: 
                    l = cPickle.load(pkl)
                del l['built'][self._id]
                with open(self._root_hists_listing_pkl,'w') as pkl: 
                    cPickle.dump(l,pkl)
                new_message = '{}, removed entry in {}, try rerunning'.format(
                    error.message, self._root_hists_listing_pkl)
                raise IOError(new_message)

            integral = _get_integrals_fast(bins)
            
            tag = hist.split('_')[-1]
            assert tag in _tags, '{} not found in {}'.format(tag, _tags)
            int_dict[tag] = integral

            ranges_dict[tag] = _get_hist_ranges(the_root_file, hist)


        with h5py.File(self._integrals_cache,'w') as hfile: 
            for name, hist in int_dict.iteritems(): 
                hfile.create_dataset(name, data=hist, compression='gzip')
                x_range, y_range = ranges_dict[name]

                hfile[name].attrs['x_min'] = x_range[0]
                hfile[name].attrs['x_max'] = x_range[1]

                hfile[name].attrs['y_min'] = y_range[0]
                hfile[name].attrs['y_max'] = y_range[1]


class HistBuilder(object): 
    """
    Builds hists requested in requests_pickle, which is loaded by
    the constructor. 
    Updates the pickle with locations and names of the built histograms. 
    
    The 'built' entry of the requests pickle contains a dictionary of 
    (file, {flavor:name}) tuples. 
    """

    def __init__(self,requests_pickle, 
                 tagger_bounds = _tagger_bounds, 
                 permissive=False): 
        try: 
            self._permissive = permissive
            self._initialize(requests_pickle, tagger_bounds)
        except BadTaggerError as bad: 
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
                    x_prefix = 'log' + x_chars
                    short_x = t.replace(x_prefix,'')
                    if x_prefix in t and short_x == short_tagger: 
                        x_matches.append(t)

                    y_prefix = 'log' + y_chars
                    short_y = t.replace(y_prefix,'')
                    if y_prefix in t and short_y == short_tagger: 
                        y_matches.append(t)

                if len(x_matches) != 1 or len(y_matches) != 1: 

                    raise BadTaggerError(
                        "taggers {} {} match {} {}".format(
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
        p, f = profile_fast(input_ntuple, tree = 'SVTree', 
                            out_file = onedim_out_file, 
                            doubles = onedim_input_list, 
                            tags = _tags, 
                            show_progress = True)
        print '{} in window, {} outside'.format(p,f)
            
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

class BinValueCnv(object): 
    """
    no undeflow or overflow bins
    """
    def __init__(self, n_bins, bin_ranges): 
        self._n_bins = n_bins
        self._low = bin_ranges[0]
        self._high = bin_ranges[1]
        
    def low_value_from_bin(self, bin_number): 
        bin_divisions = np.linspace(self._low, self._high, 
                                    num=(self._n_bins + 1))
        return bin_divisions[bin_number]
    def bin_from_value(self, value): 
        bin_bounds = np.linspace(self._low, self._high, 
                                 num=(self._n_bins + 1))
        the_bin = np.digitize([value], bin_bounds) - 1

        if the_bin < 0: 
            raise ValueError("can't put a cut in the underflow bin")
        return the_bin

class BadTaggerError(LookupError): 
    """
    Exception used by HistBuilder when it can't construct a requested hist. 
    Will be caught temporarily and used to remove the offending entry. 
    """
    def __init__(self, message, tagger_tuple = None, where = 'requested'): 
        super(BadTaggerError,self).__init__(message)
        self.id = tagger_tuple
        self.where = where
