#include "profile_fast.hh"
#include "profile_common.hh"
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

#include "TFile.h"
#include "TTree.h"

std::pair<int,int> profile_fast(std::string file_name, 
				std::string tree_name, 
				std::vector<LeafInfo> int_leaves, 
				std::vector<LeafInfo> double_leaves, 
				std::vector<std::string> tag_leaves, 
				std::string output_file_name, 
				int max_entries, 
				const unsigned options) { 
  
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
    std::string t = tree_name.c_str(); 
    throw std::runtime_error( (boost::format(err) % t % f).str()); 
  }
  tree->SetBranchStatus("*",0); 

  // typedef std::map<std::string,int*> CheckBuffer; 
  CheckBuffer check_buffer; 
  for (std::vector<std::string>::const_iterator itr = tag_leaves.begin(); 
       itr != tag_leaves.end(); itr++){ 
    check_buffer[*itr] = new int; 
  }
  for (CheckBuffer::const_iterator itr = 
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

  int n_entries = tree->GetEntries(); 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  // typedef std::map<std::string,FilterHist*> Hists; 
  Hists hists; 

  typedef std::vector<LeafInfo>::const_iterator LeafItr; 

  for (LeafItr leaf_itr = int_leaves.begin(); 
       leaf_itr != int_leaves.end(); 
       leaf_itr++){ 

    // TODO: not implemented, but should be
    assert(leaf_itr->wt_name.size() == 0); 

    int* the_int = new int; 
    int_buffer.push_back(the_int); 
    tree->SetBranchStatus(leaf_itr->name.c_str(),1); 
    bool error = tree->SetBranchAddress(leaf_itr->name.c_str(),the_int); 
    if (error) 
      throw std::runtime_error("could not find branch " + leaf_itr->name); 
    
    double* the_double = new double; 
    converted_doubles.push_back(the_double); 

    int n_bins = leaf_itr->n_bins; 
    double plot_min = leaf_itr->min; 
    double plot_max = leaf_itr->max; 
    if (n_bins == -2) { 
      n_bins = int(round(plot_max - plot_min + 1)); 
      plot_min -= 0.5; 
      plot_max += 0.5; 
    }
    else if (n_bins == -1) { 
      n_bins = std::min(max_entries / 100, magic::MAX_AUTO_BINS); 
    }

    hists[leaf_itr->name] = new FilterHist
      (n_bins, plot_min, plot_max, the_double); 
    for (CheckBuffer::const_iterator check_itr = check_buffer.begin(); 
	 check_itr != check_buffer.end(); check_itr++){ 
      std::string hist_name = leaf_itr->name + "_" + check_itr->first; 
      hists[hist_name] = 
	new FilterHist(n_bins,plot_min, plot_max, 
		       the_double, check_itr->second);
    }
    cuts.push_back(RangeCut(the_double,plot_min, plot_max)); 
  }

  for (LeafItr leaf_itr = double_leaves.begin(); 
       leaf_itr != double_leaves.end(); 
       leaf_itr++){ 

    // TODO: not implemented, but should be
    assert(leaf_itr->wt_name.size() == 0); 

    double* the_double = new double; 
    double_buffer.push_back(the_double); 
    tree->SetBranchStatus(leaf_itr->name.c_str(),1); 
    bool error = tree->SetBranchAddress(leaf_itr->name.c_str(), the_double); 
    if (error) 
      throw std::runtime_error("could not find branch " + leaf_itr->name); 

    int n_bins = leaf_itr->n_bins; 
    if (n_bins < 0) 
      n_bins = std::min(max_entries / 100, magic::MAX_AUTO_BINS); 

    hists[leaf_itr->name] = new FilterHist
      (n_bins, leaf_itr->min, leaf_itr->max, the_double); 
    for (CheckBuffer::const_iterator check_itr = check_buffer.begin(); 
	 check_itr != check_buffer.end(); check_itr++){ 
      std::string hist_name = leaf_itr->name + "_" + check_itr->first; 
      hists[hist_name] =
	new FilterHist(n_bins,leaf_itr->min, leaf_itr->max, 
		       the_double, check_itr->second);
    }
    cuts.push_back(RangeCut(the_double,leaf_itr->min, leaf_itr->max)); 
  }

  int n_to_convert = int_buffer.size(); 
  assert(n_to_convert == int(converted_doubles.size())); 

  bool show_progress = options & opt::show_progress; 
  int one_percent = max_entries / 100; 

  int n_cut = 0; 
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
    for (int i = 0; i < n_to_convert; i++){ 
      converted_doubles[i] = int_buffer[i]; 
    }

    if ( !is_in_range(cuts) ) {
      n_cut++; 
      continue; 
    }

    for (Hists::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
      itr->second->fill(); 
    }
  }
  if (show_progress) std::cout << "\n"; 

  TFile out_file(output_file_name.c_str(),"recreate"); 

  for (Hists::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
    itr->second->SetName(itr->first.c_str()); 
    out_file.WriteTObject(itr->second); 
  }

  // clean up
  return std::make_pair(max_entries - n_cut, n_cut); 
}

Hists::~Hists() {
  for (iterator itr = begin(); itr != end(); itr++){ 
    delete itr->second; 
  }
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

