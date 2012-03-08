import os, random, array

def profile_rds(rds_file, tree_name = 'SVTree'): 
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
    
        
def make_profile_file(reduced_dataset, profile_file = None): 
    from ROOT import TFile
    if profile_file is None: 
        rds_dir = os.path.dirname(reduced_dataset)
        profile_file = os.path.join(rds_dir,'profiled.root')

    hists = profile_rds(reduced_dataset)
    save_file = TFile(profile_file, 'recreate')

    for name, hist in hists.items(): 
        print 'saving %s, %i entries' % (name, hist.GetEntries())
    
        save_file.WriteTObject(hist)

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
