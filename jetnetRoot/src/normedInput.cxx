#include "normedInput.hh"
#include <boost/ptr_container/ptr_vector.hpp>
#include <string> 

#include <TTree.h> 
#include <TFile.h>
#include <TChain.h> 

#include <cassert>

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
					    TChain* reduced_dataset)
{ 
  assert (false); 
}
