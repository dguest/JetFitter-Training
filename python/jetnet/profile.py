import os, random, array, re, math

def find_leaf_ranges_by_type(rds_file, tree_name = 'SVTree', 
                             max_entries = False): 
    """
    Returns a dict keyed by type. The values of the dict are 
    """
    from ROOT import TH1D, TTree, TFile, TIter, gROOT
    
    root_file = TFile(rds_file)
    if root_file == None: 
        raise IOError('could not load %s' % rds_file)
    sv_tree = root_file.Get(tree_name)

    if sv_tree == None: 
        raise IOError('could not load %s' % tree_name)

    leaf_values = {}
    leaf_types = {}

    leaf_names = []
    leaf_list = sv_tree.GetListOfLeaves()
    for leaf in TIter(leaf_list): 
        name = leaf.GetName()
        leaf_names.append(name)
        leaf_values[name] = []
        leaf_types[name] = leaf.GetTypeName()

    # find ranges
    if not max_entries: max_entries = 20000
    for n, entry in enumerate(sv_tree): 
        if n > max_entries : break 
        for leaf in leaf_values.keys(): 
            value = getattr(entry,leaf)
            leaf_values[leaf].append(value)

    leaf_ranges = {}
    for leaf, v in leaf_values.iteritems(): 
        leaf_ranges[leaf] = (min(v), max(v))
        
    # sort by type
    all_types = set(leaf_types.values())
    leaves_by_type = {t : {} for t in all_types}
    for leaf_name, leaf_range in leaf_ranges.iteritems(): 
        the_type = leaf_types[leaf_name]
        leaves_by_type[the_type][leaf_name] = leaf_range

    return leaves_by_type


def profile_rds(rds_file, tree_name = 'SVTree', 
                max_entries = False): 
    """
    Returns a list of hists. This function is not compiled (slow). 
    """
    from ROOT import TH1D, TTree, TFile, TIter, gROOT
    
    root_file = TFile(rds_file)
    sv_tree = root_file.Get(tree_name)

    leaf_values = {}
    leaf_types = {}

    leaf_names = []
    leaf_list = sv_tree.GetListOfLeaves()
    for leaf in TIter(leaf_list): 
        name = leaf.GetName()
        leaf_names.append(name)
        leaf_values[name] = []
        leaf_types[name] = leaf.GetTypeName()

    # find ranges
    for n, entry in enumerate(sv_tree): 
        if n % 10000 == 0: 
            print n,'entries processed' 

        if n > 20000 : break 

        for leaf in leaf_values.keys(): 
            value = getattr(entry,leaf)

            leaf_values[leaf].append(value)

    leaf_ranges = {}
    for leaf, v in leaf_values.iteritems(): 
        leaf_ranges[leaf] = (min(v), max(v))

    gROOT.cd()

    class FilterHist(TH1D): 
        def __init__(self, name, n_bins, min_val, max_val, 
                     fill_leaf, req_leaf = None): 
            super(FilterHist,self).__init__(
                name, name, n_bins, min_val, max_val ) 

            self.fill_leaf = fill_leaf
            self.req_leaf = req_leaf
 
        def fill(self,entry): 
            if self.req_leaf: 
                if getattr(entry, self.req_leaf) == 0: 
                    return None

            self.Fill( getattr(entry, self.fill_leaf))
        
    hists = {}
    for tag in ['bottom','charm','light', None]: 
        for name, (min_val, max_val) in leaf_ranges.iteritems(): 
            if leaf_types[name] == 'Int_t': 
                max_val += 1
                n_bins = int(max_val - min_val)
            else: 
                n_bins = 100
                
            hist_name = '%s_%s' % ( name , tag) if tag else name
            hists[hist_name] = FilterHist(hist_name,n_bins, min_val, max_val, 
                                          fill_leaf = name, 
                                          req_leaf = tag)

    for n, entry in enumerate(sv_tree): 
        if n % 10000 == 0: 
            print '%i entries processed' % n
            if max_entries and n > max_entries: 
                break

        for leaf, hist in hists.items(): 
            hist.fill(entry)
    
    return hists

def get_mean_rms_values(hists): 
    """
    returns the mean and rms of each hist in 'hists' 
    as a dict keyed by the leaf name
    """
    mean_rms_dict = {}
    for leaf, hist in hists.iteritems(): 
        mean_rms_dict[leaf] = (
            hist.GetMean(), 
            hist.GetRMS(), 
            )

    return mean_rms_dict

def write_mean_rms_textfile(mean_rms_dict, text_file_name = 'mean_rms.txt'): 
    max_chars = max(len(n) for n in mean_rms_dict.keys())
    template = '%*s %10.4g %10.4g\n'
    with open(text_file_name,'w') as norm_file: 
        norm_file.write('#%*s %10s %10s\n' % (
                 max_chars + 1, 'leaf_name', 'mean', 'rms'))
        for name, (mean, rms) in mean_rms_dict.iteritems(): 
            norm_file.write(template % (max_chars + 2, name, mean, rms))

def write_norm_textfile(mean_rms_dict, text_file_name = 'norm_values.txt'): 
    max_chars = max(len(n) for n in mean_rms_dict.keys())
    template = '%*s %10.4g %10.4g\n'
    with open(text_file_name,'w') as norm_file: 
        norm_file.write('#%*s %10s %10s\n' % (
                 max_chars + 1, 'leaf_name', 'offset', 'scale'))
        for name, (mean, rms) in mean_rms_dict.iteritems(): 
            norm_file.write(template % (max_chars + 2, name, mean, rms))


def build_mean_rms_tree(mean_rms_dict, tree_name = ''): 
    """
    if you want to use a tree to store information, use this, 
    but, well, fuck root... seriously 
    """
    from ROOT import TTree
    if not tree_name: 
        tree_name = str(random.random())

    the_tree = TTree(tree_name, tree_name)
    array.array('c','nonono')
    print "fuck this shit, I'm going textfile"
    
        
def make_profile_file(reduced_dataset, profile_file = None, 
                      max_entries = False, tree = 'SVTree', 
                      pt_range = None, force_n_bins = False): 
    """
    new and improved: now uses cxxprofile, which should be a lot faster
    """
    if profile_file is None: 
        rds_dir = os.path.dirname(reduced_dataset)
        profile_file = os.path.join(rds_dir,'profiled.root')

    from cxxprofile import profile_fast
    range_dict = find_leaf_ranges_by_type(reduced_dataset, tree_name = tree)

    # hack to deal with empty leafs, could be dangerous
    bad_ranges = []
    for type_name, type_dict in range_dict.iteritems(): 
        for var, var_range in type_dict.iteritems(): 
            if any(math.isnan(x) for x in var_range): 
                bad_ranges.append((type_name,var))

    for type_name, var in bad_ranges: 
        print 'removing', var
        del range_dict[type_name][var]

    tags = ['charm','bottom','light']

    ints = []
    for var, var_range in range_dict['Int_t'].iteritems(): 
        if var in tags: continue
        ints.append( (var,var_range[0],var_range[1]) )

    doubles = []
    for var, var_range in range_dict['Double_t'].iteritems(): 
        if pt_range and var == 'JetPt': 
            var_range = pt_range
        span = var_range[1] - var_range[0]
        low = var_range[0] - 0.1 * span 
        high = var_range[1] + 0.1 * span 
        d_tuple = (var, low, high)
        if force_n_bins: 
            d_tuple = (var, force_n_bins,var_range[0], var_range[1])
        doubles.append( d_tuple )


    # cxx routine is programed to take -1 as all
    if not max_entries: max_entries = -1

    n_pass, n_fail = profile_fast(
        in_file = reduced_dataset, tree = tree, out_file = profile_file, 
        ints = ints, doubles = doubles, tags = tags, 
        max_entries = max_entries)
    
    return n_pass, n_fail

def read_back_profile_file(profile_file): 
    from ROOT import TFile, TIter, gROOT
    root_file = TFile(profile_file)
    list_of_keys = root_file.GetListOfKeys()
    hists = {}
    gROOT.cd()                  # clone into memory
    for key_itr in TIter(list_of_keys): 
        key_name = key_itr.GetName()
        hist = key_itr.ReadObj().Clone(str(random.random()))
        hists[key_name] = hist

    return hists


def build_mean_rms_from_profile(profile_file, 
                                text_file_name = 'mean_rms.txt'): 
    profile_hists = read_back_profile_file(profile_file)
    mean_rms_dict = get_mean_rms_values(profile_hists)
    write_mean_rms_textfile(mean_rms_dict, text_file_name)

def make_normalization_file(profile_file, 
                            normalization_file = 'normalization.txt', 
                            whitelist = None): 
    """
    make a normalization text file from profile_file. 
    """
    mean_rms_dict = get_mean_rms_values(
        read_back_profile_file(profile_file))

    if whitelist: 
        for key in mean_rms_dict.keys(): 
            if key not in whitelist: 
                del mean_rms_dict[key]
    else: 
        # -- we don't care about flavors here 
        flavor_tags = set(['light','charm','bottom'])
        for key in mean_rms_dict.keys(): 
            keyparts_set = set(key.split('_'))
            overlap = keyparts_set & flavor_tags
            if overlap: 
                del mean_rms_dict[key]

    max_chars = max(len(n) for n in mean_rms_dict.keys())
    template = '%-*s % -10.4g % -10.4g\n'
    with open(normalization_file,'w') as norm_file: 
        for name, (mean, rms) in mean_rms_dict.iteritems(): 
            offset = -mean 
            scale = 1.0 / rms
            this_var_info = template % (max_chars, name, offset, scale)
            norm_file.write(this_var_info)
