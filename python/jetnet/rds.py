from jetnet import utils
from warnings import warn
def get_allowed_rds_variables(
    input_files, 
    jet_collection = 'AntiKt4TopoEMJetsReTagged', 
    full_dir_name = None, 
    whitelist = None): 
    """
    return a tuple of (double_variables, int_variables), checking the 
    first of input_files for matches. If full_dir_name is given jet_collection
    is ignored. 
    
    Mon Jun 11 11:37:31 CEST 2012: Added some warnings wrt deprecation of
    jet_collection. 
    """

    from ROOT import TFile

    sample_root_file = TFile(input_files[0])

    if jet_collection != 'AntiKt4TopoEMJetsReTagged':
        if full_dir_name: 
            warn("specifying full_dir_name will overwrite jet_collection", 
                 SyntaxWarning, stacklevel = 2)
        else: 
            warn("jet_collection is going to be replaced, use full_dir_name", 
                 FutureWarning, stacklevel = 2)

    if full_dir_name: 
        input_tree_name = full_dir_name + '/PerfTreeAll'
    else: 
        input_tree_name = ( 
            'BTag_%s_JetFitterTagNN/PerfTreeAll' % (jet_collection + 'AOD') )

    sample_tree = sample_root_file.Get(input_tree_name)

    # stupid bug fix 
    # FIXME: fix stupid bug fix
    if sample_tree == None: 
        warn("could not find %s, trying other stuff" % input_tree_name)
        input_tree_name = (
            '%s_JetFitterCharm/PerfTreeAll' % (jet_collection) )
        maybe_tree = sample_root_file.Get(input_tree_name)
        if maybe_tree != None: 
            sample_tree = maybe_tree
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
        warn("no whitelist given, will return all vars", stacklevel = 2)
        
        # for flav in 'buc': 
        #     double_variables.remove('Likelihood_' + flav)
        # for slimvar in ['JetPt','JetEta','mass']: 
        #     double_variables.remove(slimvar)

    return double_variables, int_variables
