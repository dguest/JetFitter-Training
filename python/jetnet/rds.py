from jetnet import utils
def get_allowed_rds_variables(
    input_files, 
    jet_collection = 'AntiKt4TopoEMJetsReTagged', 
    full_dir_name = None, 
    whitelist = None): 
    """
    return a tuple of (double_variables, int_variables), checking the 
    first of input_files for matches. If full_dir_name is given jet_collection
    is ignored. 
    """

    from ROOT import TFile

    sample_root_file = TFile(input_files[0])

    if full_dir_name: 
        input_tree_name = full_dir_name + '/PerfTreeAll'
    else: 
        input_tree_name = ( 
            'BTag_%s_JetFitterTagNN/PerfTreeAll' % (jet_collection + 'AOD') )

    sample_tree = sample_root_file.Get(input_tree_name)

    # stupid bug fix 
    # FIXME: fix stupid bug fix
    if sample_tree == None: 
        input_tree_name = (
            '%s_JetFitterCharm/PerfTreeAll' % (jet_collection) )
        sample_tree = sample_root_file.Get(input_tree_name)
        if sample_tree == None: 
            raise IOError("could not find %s" % input_tree_name)
        
    leaves_dict = utils.get_leaves_in_tree(sample_tree)

    if whitelist: 
        double_variables = whitelist
        int_variables = whitelist

        double_variables = [
            x for x in double_variables if x in leaves_dict['Double_t'] ]

        int_variables = [
            x for x in int_variables if x in leaves_dict['Int_t'] ]


    else: 
        int_variables = leaves_dict['Int_t']
        int_variables.remove('Flavour')
        double_variables = leaves_dict['Double_t']
        double_variables.remove('Discriminator')
        for flav in 'buc': 
            double_variables.remove('Likelihood_' + flav)
        for slimvar in ['JetPt','JetEta','mass']: 
            double_variables.remove(slimvar)

    return double_variables, int_variables
