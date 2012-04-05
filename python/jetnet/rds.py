from jetnet import utils
def get_allowed_rds_variables(input_files, jet_collection, 
                              whitelist = None): 
    """
    return a tuple of double_variables, int_variables, checking the 
    first of input_files for matches
    """


    from ROOT import TFile

    sample_root_file = TFile(input_files[0])
    input_tree_name = ( 
        'BTag_%s_JetFitterTagNN/PerfTreeAll' % (jet_collection + 'AOD') )

    sample_tree = sample_root_file.Get(input_tree_name)
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
