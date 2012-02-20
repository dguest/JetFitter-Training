

def profile_rds(rds_file): 
    from ROOT import TH1D, TTree, TFile, TIter, gROOT
    
    root_file = TFile(rds_file)
    sv_tree = root_file.Get('SVTree')

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

    # for leaf, value_list in leaf_ranges.items(): 
    #     print leaf, min(value_list), '--', max(value_list)
        
    leaf_ranges = {}
    for leaf, v in leaf_values.iteritems(): 
        leaf_ranges[leaf] = (min(v), max(v))

    gROOT.cd()
    hists = {}
    for name, (min_val, max_val) in leaf_ranges.iteritems(): 
        if leaf_types[name] == 'Int_t': 
            n_bins = int(max_val - min_val) 
        else: 
            n_bins = 100
        hists[name] = TH1D(name,name,n_bins, min_val, max_val)

    for n, entry in enumerate(sv_tree): 
        if n % 10000 == 0: 
            print '%i entries processed' % n

        for leaf, hist in hists.items(): 
            value = getattr(entry,leaf)
            hist.Fill(value)
    
    return hists
        
def make_profile_file(reduced_dataset, profile_file = None): 
    if profile_file is None: 
        profile_file = os.path.dirname(reduced_dataset) + 'profiled.root'
    hists = profile.profile_rds(reduced_dataset)
    save_file = TFile(profile_file, 'recreate')

    for name, hist in hists.items(): 
        print 'saving %s, %i entries' % (name, hist.GetEntries())
    
        save_file.WriteTObject(hist)

