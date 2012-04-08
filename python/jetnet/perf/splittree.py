from ROOT import TIter, TFile, TTree
from array import array
import os

def split_tree(in_file, values, variable = 'JetPt', 
               out_file_name = '%s_%s-%s.root', 
               out_file_dir = ''): 
    """
    splits tree into subtrees, divided at values

    returns the relative paths of the output trees
    """
    padded_values = sorted(values)
    padded_values.append(None)
    out_trees = []
    out_files = []
    lower_limit = None
    for upper_limit in padded_values:
        lower_name = str(lower_limit) if lower_limit else 'under'
        upper_name = str(upper_limit) if upper_limit else 'up'
        file_name = out_file_name % (variable, lower_name, upper_name)
        full_file_name = os.path.join(out_file_dir, file_name)
        the_file = TFile(full_file_name,'recreate')
        the_tree = TTree('performance','performance')
        out_files.append(the_file)
        the_tree.range = (lower_limit, upper_limit)
        out_trees.append(the_tree)
        lower_limit = upper_limit

    datatype_to_buffer_char = { 
        'Int_t': 'i', 
        'Double_t':'d', 
        }
    datatype_to_root_type = { 
        'Int_t': 'I', 
        'Double_t':'D', 
        }


    root_file = TFile(in_file)
    tree = root_file.Get('performance')

    class Buffer(object): pass

    leaf_list = []
    for leaf in TIter(tree.GetListOfLeaves()):
        name = leaf.GetName()
        type_name = leaf.GetTypeName()
        data_char = datatype_to_buffer_char[type_name]
        root_char = datatype_to_root_type[type_name]
        the_array = array(data_char,[0])
        for out in out_trees: 
            # print 'building branch:', name + '/' + root_char
            out.Branch(name,the_array, name + '/' + root_char)

        the_buffer = Buffer()
        the_buffer.name = name
        the_buffer.value = the_array
        leaf_list.append(the_buffer)

    print '%i entries in tree to read' % tree.GetEntries()
    for n, entry in enumerate(tree): 
        if n % 10000 == 0: 
            print n, 'entries processed' 

        if n == 100000: break 

        cat_var = getattr(entry,variable)
        leaves_copied = False
        for output in out_trees: 
            minval, maxval = output.range
            if minval is None: minval = cat_var - 1
            if maxval is None: maxval = cat_var + 1
            if minval <= cat_var < maxval: 
                if not leaves_copied: 
                    for leaf in leaf_list: 
                        leaf.value[0] = getattr(entry,leaf.name)
                    leaves_copied = True
                output.Fill()
                    
    for out_file, out_tree in zip(out_files, out_trees): 
        out_file.WriteTObject(out_tree)
            
    return [f.GetName() for f in out_files]
