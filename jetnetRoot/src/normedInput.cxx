#include "normedInput.hh"
#include "nnExceptions.hh"

#include <boost/ptr_container/ptr_vector.hpp>
#include <string> 

#include <TTree.h> 
#include <TFile.h>
#include <TObjArray.h>
#include <TLeaf.h>

#include <cassert>
#include <iostream>

// -- stuff for virtual base class
std::ostream& operator<<(std::ostream& out, const NormedInputBase& n)
{
  n.print_to(out); 
  return out; 
}


int InputVariableContainer::write_to_file(TFile* file, 
					  std::string tree_name) const
{
  TTree* out_tree = new TTree(tree_name.c_str(), tree_name.c_str()); 
  std::string name; 
  float offset; 
  float scale; 
  out_tree->Branch("name", &name); 
  out_tree->Branch("offset", &offset); 
  out_tree->Branch("scale", &scale); 
  for (int node_number = 0; node_number < size(); node_number++){ 
    name = at(node_number).get_name(); 
    offset = at(node_number).get_offset(); 
    scale = at(node_number).get_scale(); 
    out_tree->Fill(); 
  }
  file->WriteTObject(out_tree); 
  return 0; 
}

int InputVariableContainer::build_from_tree(TTree* info_tree, 
					    TTree* reduced_dataset)
{ 
  if (size() > 0) { 
    std::cerr << "ERROR: tried to build normed inputs in nonempty "
      "container\n"; 
    throw LoadNormalizationException(); 
  }

  std::string name; 
  float offset; 
  float scale; 
  info_tree->SetBranchStatus("*",1); 
  info_tree->SetBranchAddress("name",   &name); 
  info_tree->SetBranchAddress("offset", &offset); 
  info_tree->SetBranchAddress("scale",  &scale); 

  // TODO: replace with boost::scoped_ptr 
  TObjArray* list_of_leaves = reduced_dataset->GetListOfLeaves(); 
  int n_leaves = list_of_leaves->GetSize(); 

  int n_inputs = info_tree->GetEntries(); 
  for (int input_n = 0; input_n < n_inputs; input_n++){ 
    info_tree->GetEntry(input_n); 
    
    std::string leaf_type_name = ""; 
    for (int leaf_n = 0; leaf_n < n_leaves; leaf_n++){ 
      TLeaf* the_leaf = dynamic_cast<TLeaf*>(list_of_leaves->At(leaf_n)); 
      assert(the_leaf); 
      std::string leaf_name = the_leaf->GetName(); 
      if (leaf_name == name){ 
	leaf_type_name = the_leaf->GetTypeName(); 
	break; 
      }
    }
    if (leaf_type_name.size() == 0){ 
      throw MissingLeafException(name, reduced_dataset->GetName()); 
    }

    if (leaf_type_name == "Int_t") { 
      NormedInput<int>* the_input = 
	new NormedInput<int>(name, offset, scale);
      the_input->set_tree(reduced_dataset); 
      push_back(the_input);
    } 
    else if (leaf_type_name == "Double_t"){ 
      NormedInput<double>* the_input = 
	new NormedInput<double>(name, offset, scale); 
      the_input->set_tree(reduced_dataset); 
      push_back(the_input); 
    }
    else{ 
      std::cerr << "ERROR: I don't know what to do with " << leaf_type_name
		<< "\n"; 
      assert(false); 
    }

  }

}


// --- exceptions

MissingLeafException::MissingLeafException(std::string leaf_name, 
					   std::string chain_name): 
  _leaf_name(leaf_name), 
  _chain_name(chain_name)
{ }; 

std::string MissingLeafException::leaf_name() const
{
  return _leaf_name; 
}

std::string MissingLeafException::chain_name() const
{
  return _chain_name; 
} 
