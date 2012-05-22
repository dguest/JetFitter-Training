#include "profile_fast.hh"
#include <string> 
#include <stdexcept>
#include <cassert>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
// #include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <cmath> // for rand
#include <algorithm>

#include "TFile.h"
#include "TTree.h"

int profile_fast(std::string file_name, 
		 std::string tree_name, 
		 std::vector<LeafInfo> int_leaves, 
		 std::vector<LeafInfo> double_leaves, 
		 std::vector<std::string> tag_leaves, 
		 std::string output_file_name, 
		 int max_entries, 
		 int n_bins) { 
  
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
    std::string t = tree->GetName(); 
    throw std::runtime_error( (boost::format(err) % t % f).str()); 
  }
  tree->SetBranchStatus("*",0); 

  typedef std::map<std::string,int*> CheckBuffer; 
  CheckBuffer check_buffer; 
  for (std::vector<std::string>::const_iterator itr = tag_leaves.begin(); 
       itr != tag_leaves.end(); itr++){ 
    check_buffer[*itr] = new int; 
  }
  for (std::map<std::string,int*>::const_iterator itr = 
	 check_buffer.begin(); 
       itr != check_buffer.end(); 
       itr++){ 
    tree->SetBranchStatus(itr->first.c_str(),1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    if (error) { 
      throw std::runtime_error("could not find branch " + itr->first); 
    }

  }

  boost::ptr_vector<double> double_buffer; 
  boost::ptr_vector<int> int_buffer; 
  boost::ptr_vector<double> converted_doubles; 
  std::vector<RangeCut> cuts; 

  std::map<std::string,FilterHist*> hists; 

  typedef std::vector<LeafInfo>::const_iterator LeafItr; 

  for (LeafItr leaf_itr = int_leaves.begin(); 
       leaf_itr != int_leaves.end(); 
       leaf_itr++){ 
    int* the_int = new int; 
    int_buffer.push_back(the_int); 
    tree->SetBranchStatus(leaf_itr->name.c_str(),1); 
    bool error = tree->SetBranchAddress(leaf_itr->name.c_str(),the_int); 
    if (error) 
      throw std::runtime_error("could not find branch " + leaf_itr->name); 
    
    double* the_double = new double; 
    converted_doubles.push_back(the_double); 
    for (CheckBuffer::const_iterator check_itr = check_buffer.begin(); 
	 check_itr != check_buffer.end(); check_itr++){ 
      std::string hist_name = leaf_itr->name + "_" + check_itr->first; 
      hists[hist_name] = 
	new FilterHist(n_bins,leaf_itr->min, leaf_itr->max, 
		       the_double, check_itr->second);
    }
    cuts.push_back(RangeCut(the_double,leaf_itr->min, leaf_itr->max)); 
  }

  for (LeafItr leaf_itr = double_leaves.begin(); 
       leaf_itr != double_leaves.end(); 
       leaf_itr++){ 
    double* the_double = new double; 
    double_buffer.push_back(the_double); 
    tree->SetBranchStatus(leaf_itr->name.c_str(),1); 
    bool error = tree->SetBranchAddress(leaf_itr->name.c_str(), the_double); 
    if (error) 
      throw std::runtime_error("could not find branch " + leaf_itr->name); 
    
    for (CheckBuffer::const_iterator check_itr = check_buffer.begin(); 
	 check_itr != check_buffer.end(); check_itr++){ 
      std::string hist_name = leaf_itr->name + "_" + check_itr->first; 
      hists[hist_name] =
	new FilterHist(n_bins,leaf_itr->min, leaf_itr->max, 
		       the_double, check_itr->second);
    }
    cuts.push_back(RangeCut(the_double,leaf_itr->min, leaf_itr->max)); 
  }

  int n_entries = tree->GetEntries(); 
  int n_to_convert = int_buffer.size(); 
  assert(n_to_convert == int(converted_doubles.size())); 

  int n_cut = 0; 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  for (int entry_n = 0; entry_n < max_entries; entry_n++){ 
    if (max_entries >= 0 && int(entry_n) > max_entries) break; 
    tree->GetEntry(entry_n); 
    for (int i = 0; i < n_to_convert; i++){ 
      converted_doubles[i] = int_buffer[i]; 
    }

    if ( !is_in_range(cuts) ) {
      n_cut++; 
      continue; 
    }

    for (std::map<std::string, FilterHist*>::iterator itr = 
	   hists.begin(); itr != hists.end(); itr++){ 
      itr->second->fill(); 
    }
  }

  TFile out_file(output_file_name.c_str(),"recreate"); 

  for (std::map<std::string, FilterHist*>::iterator itr = 
	 hists.begin(); itr != hists.end(); itr++){ 
    itr->second->SetName(itr->first.c_str()); 
    out_file.WriteTObject(itr->second); 
  }

  // clean up
  out_file.Close(); 
  for (std::map<std::string, FilterHist*>::iterator itr = 
	 hists.begin(); itr != hists.end(); itr++){ 
    delete itr->second; 
  }
  
  for (CheckBuffer::iterator itr = check_buffer.begin(); 
       itr != check_buffer.end(); itr++){ 
    delete itr->second; 
  }

  return max_entries - n_cut; 

}

FilterHist::FilterHist(int n_bins, double min, double max, 
		       double* in_buffer, int* check_buffer): 
  TH1D(boost::lexical_cast<std::string>(rand()).c_str(),"",
       n_bins, min, max), 
  m_in_buffer(in_buffer), 
  m_check_buffer(check_buffer)
{
}

FilterHist::~FilterHist() { 
}

FilterHist::FilterHist(const FilterHist&) 
{ 
}

FilterHist& FilterHist::operator=(const FilterHist& in) { 
  return *this; 
}

int FilterHist::fill()
{
  bool pass_check = true; 
  if (m_check_buffer) { 
    pass_check = *m_check_buffer; 
  }
  if (!pass_check) return 0; 

  Fill(*m_in_buffer); 
  return 1; 
}

RangeCut::RangeCut(double* value, double lower, double upper): 
  m_value(value), 
  m_lower(lower), 
  m_upper(upper) 
{
}

bool RangeCut::in_range() const 
{
  bool above = *m_value > m_lower; 
  bool below = *m_value < m_upper; 
  return above && below; 
}

bool is_in_range(const std::vector<RangeCut>& cuts)
{
  for (std::vector<RangeCut>::const_iterator itr = cuts.begin(); 
	 itr != cuts.end(); itr++){ 
    if (!itr->in_range()) { 
      return false; 
    }
  }
  return true; 
}
