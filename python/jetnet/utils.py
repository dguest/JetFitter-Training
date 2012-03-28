

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
