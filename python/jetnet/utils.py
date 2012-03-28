

def get_leaves_in_tree(tree): 
    """
    returns a type-keyed dict of variable lists
    """

    from ROOT import  TIter

    leaves_dict = {}
    leaf_list = tree.GetListOfLeaves()
    for leaf in TIter(leaf_list): 
        name = leaf.GetName()
        type_name = leaf.GetTypeName()
        if not type_name in leaves_dict.keys(): 
            leaves_dict[type_name] = [name]
        else: 
            leaves_dict[type_name].append(name)

    return leaves_dict

def get_allowed_rds_variables(input_files, jet_collection, 
                              int_variables = None, 
                              double_variables = None): 
    """
    return a tuple of double_variables, int_variables, checking the 
    first of input_files for matches
    """


    from ROOT import TFile

    sample_root_file = TFile(input_files[0])
    input_tree_name = ( 
        'BTag_%s_JetFitterTagNN/PerfTreeAll' % (jet_collection + 'AOD') )

    sample_tree = sample_root_file.Get(input_tree_name)
    leaves_dict = get_leaves_in_tree(sample_tree)

    if not double_variables: 
        double_variables = [
            'energyFraction', 
            'significance3d',         
            'meanTrackRapidity', 
            'maxTrackRapidity', 
            'minTrackRapidity', 
            'minTrackPtRel', 
            'meanTrackPtRel', 
            'maxTrackPtRel', 
            'leadingVertexPosition', 
            ]
        double_variables = [
            x for x in double_variables if x in leaves_dict['Double_t'] ]

    if not int_variables: 
        int_variables = [ 
            'nVTX', 
            'nTracksAtVtx', 
            'nSingleTracks', 
            ]
        int_variables = [
            x for x in int_variables if x in leaves_dict['Int_t'] ]

    return double_variables, int_variables
