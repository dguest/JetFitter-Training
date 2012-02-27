#include "normedInput.hh"
#include "nnExceptions.hh"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <string> 

#include <TTree.h> 
#include <TFile.h>
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

  InputVariableInfo in_var; 
  // ROOT wants a pointer to stl objects
  boost::scoped_ptr<std::string> name_holder(new std::string); 
  std::string* name_holder_raw = name_holder.get(); 
  info_tree->SetBranchStatus("*",1); 
  info_tree->SetBranchAddress("name",   &name_holder_raw ); 
  info_tree->SetBranchAddress("offset", &in_var.offset ); 
  info_tree->SetBranchAddress("scale",  &in_var.scale ); 

  int n_inputs = info_tree->GetEntries(); 
  for (int input_n = 0; input_n < n_inputs; input_n++){ 
    info_tree->GetEntry(input_n); 
    
    in_var.name = *name_holder; 
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
  return 0; 
}

int InputVariableContainer::set_hardcoded_defaults(TTree* reduced_dataset)
{ 
  typedef InputVariableInfo I; 
  I nVTX               = {"nVTX"                , -0.30, 1.0 / 0.50} ; 
  I nTracksAtVtx       = {"nTracksAtVtx"        , -1.00, 1.0 / 1.60} ; 
  I nSingleTracks      = {"nSingleTracks"       , -0.20, 1.0 / 0.50} ; 
  I energyFraction     = {"energyFraction"      , -0.23, 1.0 / 0.33} ; 
  I mass               = {"mass"                , - 974, 1.0 / 1600} ; 
  I significance3d     = {"significance3d"      , -   7, 1.0 / 14.0} ; 
  I discriminatorIP3D  = {"discriminatorIP3D"   , - 6.3, 1.0 /  6.0} ; 
  I cat_pT             = {"cat_pT"              , - 3.0, 1.0 /  3.0} ; 
  I cat_eta            = {"cat_eta"             , - 1.0,        1.0} ; 

  add_variable(nVTX                , reduced_dataset); 
  add_variable(nTracksAtVtx        , reduced_dataset); 
  add_variable(nSingleTracks       , reduced_dataset); 
  add_variable(energyFraction      , reduced_dataset); 
  add_variable(mass                , reduced_dataset); 
  add_variable(significance3d      , reduced_dataset); 
  add_variable(discriminatorIP3D   , reduced_dataset); 
  add_variable(cat_pT              , reduced_dataset); 
  add_variable(cat_eta             , reduced_dataset); 

  return 0; 
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
