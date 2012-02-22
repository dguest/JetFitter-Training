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
  // *************** the tree needs to read the address of a std string
  InputVariableInfo in_var = {"",0,0}; 
  info_tree->SetBranchStatus("*",1); 
  info_tree->SetBranchAddress("name",   &(in_var.name) ); 
  info_tree->SetBranchAddress("offset", &(in_var.offset) ); 
  info_tree->SetBranchAddress("scale",  &(in_var.scale) ); 

  int n_inputs = info_tree->GetEntries(); 
  for (int input_n = 0; input_n < n_inputs; input_n++){ 
    info_tree->GetEntry(input_n); 
    
    add_variable(in_var, reduced_dataset); 
 
  }
  return 0; 

}

int InputVariableContainer::add_variable(const InputVariableInfo& in_var, 
					 TTree* reduced_dataset){ 
  // TODO: replace with boost::scoped_ptr 
  TLeaf* the_leaf = reduced_dataset->GetLeaf(in_var.name.c_str()); 
  if (the_leaf == 0){ 
    throw MissingLeafException(in_var.name, reduced_dataset->GetName()); 
  }
  std::string leaf_type_name = the_leaf->GetTypeName(); 

  if (leaf_type_name == "Int_t") { 
    push_back(new NormedInput<int>(in_var, reduced_dataset));
  } 
  else if (leaf_type_name == "Double_t"){ 
    push_back(new NormedInput<double>(in_var, reduced_dataset)); 
  }
  else{ 
    std::cerr << "ERROR: I don't know what to do with " << leaf_type_name
	      << "\n"; 
    assert(false); 
  }

  return 0; 
}

int InputVariableContainer::add_variable(const std::string& name, 
					 TTree* reduced_dataset){ 
  InputVariableInfo info = { name, 0, 0}; 
  add_variable(info, reduced_dataset); 
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
