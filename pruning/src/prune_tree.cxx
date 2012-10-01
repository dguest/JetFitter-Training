#include "prune_tree.hh"
#include <string> 
#include <stdexcept>
#include <cassert>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <cmath> // for rand
#include <utility> // pair
#include <algorithm>
#include <iostream>
#include <set> 

#include "TFile.h"
#include "TTree.h"
#include "TLeaf.h"
#include "TObjArray.h"

int simple_prune(std::string file_name, 
		 std::string tree_name, 
		 Cuts cuts_info, 
		 std::set<std::string> subset, 
		 std::string output_file_name, 
		 int max_entries, 
		 const unsigned options){ 
  
  TFile file(file_name.c_str()); 
  if (file.IsZombie() || !file.IsOpen() ) { 
    std::string err = " cannot be opened"; 
    throw std::runtime_error(file.GetName() + err); 
  }
  srand(0); 
  
  TTree* tree = dynamic_cast<TTree*>(file.Get(tree_name.c_str())); 
  if (!tree) { 
    std::string err = "%s in %s cannot be opened"; 
    std::string f = file.GetName(); 
    std::string t = tree_name; 
    throw std::runtime_error( (boost::format(err) % t % f).str()); 
  }
  tree->SetBranchStatus("*",0); 

  // ---- set all branches ----
  // Double and IntBuffer own the pointers to the leafs
  IntBuffer int_buffer; 	
  DoubleBuffer double_buffer; 
  UnsignedBuffer unsigned_buffer; 
  std::vector<std::string> untypable_branches; 

  TObjArray* leaflist = tree->GetListOfLeaves(); 
  int n_leaves = leaflist->GetEntries(); 

  for (int leaf_n = 0; leaf_n < n_leaves; leaf_n++) { 
    TLeaf* leaf = static_cast<TLeaf*>(leaflist->At(leaf_n)); 
    assert(leaf); 
    std::string leaf_type = leaf->GetTypeName(); 
    std::string leaf_name = leaf->GetName(); 
    if (leaf_type == "Double_t"){ 
      double_buffer[leaf_name] = new double; 
    }
    else if (leaf_type == "Int_t") { 
      int_buffer[leaf_name] = new int; 
    }
    else if (leaf_type == "UInt_t") { 
      unsigned_buffer[leaf_name] = new unsigned; 
    }
    else{ 
      untypable_branches.push_back(leaf_name); 
    }
  }
  
  for (IntBuffer::const_iterator itr = 
	 int_buffer.begin(); 
       itr != int_buffer.end(); 
       itr++){ 
    tree->SetBranchStatus(itr->first.c_str(),1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    if (error) { 
      throw std::runtime_error("could not find branch " + itr->first); 
    }
  }

  for (DoubleBuffer::const_iterator itr = 
	 double_buffer.begin(); 
       itr != double_buffer.end(); 
       itr++){ 
    tree->SetBranchStatus(itr->first.c_str(),1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    if (error) { 
      throw std::runtime_error("could not find branch " + itr->first); 
    }
  }
  for (UnsignedBuffer::const_iterator itr = 
	 unsigned_buffer.begin(); 
       itr != unsigned_buffer.end(); 
       itr++){ 
    tree->SetBranchStatus(itr->first.c_str(),1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    if (error) { 
      throw std::runtime_error("could not find branch " + itr->first); 
    }
  }

  // --- define cuts ---

  SubTreeCuts cuts; 		// owns the cuts in it
  for (std::vector<SubTreeIntInfo>::const_iterator 
	 itr = cuts_info.ints.begin(); 
       itr != cuts_info.ints.end(); 
       itr++) { 
    int* buffer = int_buffer[itr->name]; 
    if (!buffer) { 
      throw std::runtime_error("could not find branch" + itr->name); 
    }
    cuts.push_back(new SubTreeIntCut(itr->name, itr->value, buffer)); 
  }
  for (std::vector<SubTreeDoubleInfo>::const_iterator
	 itr = cuts_info.doubles.begin(); 
       itr != cuts_info.doubles.end(); 
       itr++) { 
    double* buffer = double_buffer[itr->name]; 
    if (!buffer) { 
      throw std::runtime_error("could not find branch" + itr->name); 
    }
    cuts.push_back(new SubTreeDoubleCut(itr->name, 
					itr->low, itr->high, 
					buffer) ); 
  }
  for (std::vector<SubTreeUnsignedInfo>::const_iterator
	 itr = cuts_info.bits.begin(); 
       itr != cuts_info.bits.end(); itr++) { 
    unsigned* buffer = unsigned_buffer[itr->name]; 
    if (!buffer) { 
      throw std::runtime_error("could not find branch" + itr->name); 
    }
    cuts.push_back(new SubTreeBitmaskCut(itr->name, 
					 itr->required, itr->veto, 
					 buffer) ); 
  }

  // --- define outputs ----
  TFile out_file(output_file_name.c_str(), "recreate"); 
  if (!out_file.IsOpen() || out_file.IsZombie()) { 
    throw std::runtime_error("can't create " + output_file_name); 
  }
  TTree out_tree("SVTree","SVTree"); 

  
  for (IntBuffer::const_iterator itr = int_buffer.begin(); 
       itr != int_buffer.end(); 
       itr++) { 
    if (subset.size() == 0 || subset.count(itr->first))
      out_tree.Branch(itr->first.c_str(), itr->second); 
  }
  for (DoubleBuffer::const_iterator itr = double_buffer.begin(); 
       itr != double_buffer.end(); 
       itr++) { 
    if (subset.size() == 0 || subset.count(itr->first))
      out_tree.Branch(itr->first.c_str(), itr->second); 
  }


  // loop over the tree now
  int n_entries = tree->GetEntries(); 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  bool show_progress = options & opt::verbose; 
  int one_percent = max_entries / 100; 

  for (int entry_n = 0; entry_n < max_entries; entry_n++){ 
    if (max_entries >= 0 && int(entry_n) > max_entries) break; 
    if (show_progress && 
	(entry_n % one_percent == 0 || entry_n + 1 == n_entries)) { 
      std::cout << boost::format
	("\r%.1fM of %.1fM entries processed (%.0f%%)") 
	% (float(entry_n) / 1e6 ) % (float(max_entries) / 1e6 ) 
	% (float(entry_n) * 100 / float(max_entries) ); 
      std::cout.flush(); 
    }
    tree->GetEntry(entry_n); 

    bool pass_all_cuts = true; 
    for (SubTreeCuts::const_iterator itr = cuts.begin(); 
	 itr != cuts.end(); 
	 itr++) { 
      if (!(*itr)->check()) { 
	pass_all_cuts = false; 
	break; 
      }
    }
    if (pass_all_cuts) { 
      out_tree.Fill(); 
    }

  }
  if (show_progress) std::cout << "\n"; 

  out_file.WriteTObject(&out_tree); 

  return 0; 

}

SubTreeDoubleCut::SubTreeDoubleCut(std::string name, 
				   double low, 
				   double up, 
				   double* ptr): 
  m_variable(name), 
  m_upper(up), 
  m_lower(low), 
  m_ptr(ptr)
{
}

bool SubTreeDoubleCut::check() const
{
  return (*m_ptr < m_upper) && (*m_ptr >= m_lower); 
}
std::string SubTreeDoubleCut::name() 
{
  return m_variable; 
}

SubTreeIntCut::SubTreeIntCut(std::string name, 
			     int value, 
			     int* ptr): 
  m_variable(name), 
  m_value(value), 
  m_ptr(ptr)
{
}
bool SubTreeIntCut::check() const
{
  return *m_ptr == m_value; 
}
std::string SubTreeIntCut::name()
{
  return m_variable; 
}

SubTreeBitmaskCut::SubTreeBitmaskCut(std::string name, 
				     unsigned required, 
				     unsigned veto, 
				     unsigned* ptr): 
  m_variable(name), 
  m_required(required), 
  m_veto(veto), 
  m_ptr(ptr)
{
}
bool SubTreeBitmaskCut::check() const
{
  unsigned value = *m_ptr; 
  bool has_all_required = ((m_required & value) == value); 
  bool has_any_veto = (m_veto & value); 
  return has_all_required && ~has_any_veto; 
}
std::string SubTreeBitmaskCut::name()
{
  return m_variable; 
}



IntBuffer::~IntBuffer() 
{
  for (const_iterator itr = begin(); itr != end(); itr++) { 
    delete itr->second; 
  }
}
DoubleBuffer::~DoubleBuffer() 
{
  for (const_iterator itr = begin(); itr != end(); itr++) { 
    delete itr->second; 
  }
}
UnsignedBuffer::~UnsignedBuffer() 
{
  for (const_iterator itr = begin(); itr != end(); itr++) { 
    delete itr->second; 
  }
}

SubTreeCuts::~SubTreeCuts() 
{
  for (const_iterator itr = begin(); itr != end(); itr++) { 
    delete *itr; 
  }
}

// CheckedTree::CheckedTree(std::string name): 
//   m_tree(new TTree(name.c_str(),"")), 
//   m_wrote(false)
// {
// }

// CheckedTree::~CheckedTree() 
// {
//   // I think the file disctructor takes care of deleting the tree...
//   if (!m_wrote)
//     delete m_tree; 
// }

// void CheckedTree::AddObserver(std::pair<std::string, int*> observer) 
// {
//   m_tree->Branch(observer->first.c_str(), observer->second); 
// }
// void CheckedTree::AddObserver(std::pair<std::string, double*> observer) 
// {
//   m_tree->Branch(observer->first.c_str(), observer->second); 
// }

// void CheckedTree::addCuts(SubTreeCut& cut) 
// { 
  
//   m_cuts.push_back(cut); 
// }

// void CheckedTree::write_to(TFile& file)
// {
//   m_wrote = true; 
//   file.WriteTObject(m_tree); 
// }

// void CheckedTree::fill()
// { 
//   for (std::vector<SubTreeIntCut>::const_iterator itr = m_int_cuts.begin(); 
//        itr != int_cuts.end(); 
//        itr++) { 
    
//   }
// }
