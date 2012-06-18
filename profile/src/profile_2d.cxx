#include "profile_2d.hh"
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


std::pair<int,int> pro_2d(std::string file_name, 
			  std::string tree_name, 
			  LeafInfoPairs plots, 
			  std::vector<std::string> tag_leaves, 
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
    std::string t = tree->GetName(); 
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

  DoubleBufferMap double_buffer; 

  for (LeafInfoPairs::const_iterator itr = plots.begin(); 
       itr != plots.end(); itr++){ 
    // reserve buffers for value leafs
    if (!double_buffer.count(itr->first.name) )
      double_buffer[itr->first.name] = new double; 
    if (!double_buffer.count(itr->second.name) )
      double_buffer[itr->second.name] = new double; 
    
    // and for wt leafs 
    if (!double_buffer.count(itr->first.wt_name) )
      double_buffer[itr->first.name] = new double; 
    if (!double_buffer.count(itr->second.wt_name) )
      double_buffer[itr->second.name] = new double; 
  }

  // set all (double) branches
  for (DoubleBufferMap::const_iterator itr = 
	 double_buffer.begin(); 
       itr != double_buffer.end(); 
       itr++){ 
    tree->SetBranchStatus(itr->first.c_str(),1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    if (error) { 
      throw std::runtime_error("could not find branch " + itr->first); 
    }

  }

  // std::vector<RangeCut> cuts; 

  int n_entries = tree->GetEntries(); 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  Hists2D hists; 

  // TODO: this could be simplified if I made the FilterHist2D constructor 
  //       accept two copies of LeafInfo
  for (LeafInfoPairs::const_iterator leaf_itr = plots.begin(); 
       leaf_itr != plots.end(); 
       leaf_itr++){ 

    int x_bins = leaf_itr->first.n_bins; 
    int y_bins = leaf_itr->second.n_bins; 
    if (x_bins < 0) x_bins = magic::DEF_2D_BINS; 
    if (y_bins < 0) y_bins = magic::DEF_2D_BINS; 

    std::string hist_name = leaf_itr->second.name + "_vs_" + 
      leaf_itr->first.name; 

    hists[hist_name] = new FilterHist2D(leaf_itr->first, leaf_itr->second, 
					double_buffer); 

    for (CheckBuffer::const_iterator check_itr = check_buffer.begin(); 
	 check_itr != check_buffer.end(); check_itr++){ 
      std::string filt_hist_name = leaf_itr->second.name + "_vs_" + 
      leaf_itr->first.name + "_" + check_itr->first; 
      hists[filt_hist_name] = new FilterHist2D
	(leaf_itr->first, leaf_itr->second, 
	 double_buffer,check_itr->second); 
    }
  }


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

    for (Hists2D::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
      itr->second->fill(); 
    }
  }
  if (show_progress) std::cout << "\n"; 

  TFile out_file(output_file_name.c_str(),"recreate"); 

  for (Hists2D::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
    itr->second->SetName(itr->first.c_str()); 
    out_file.WriteTObject(itr->second); 
  }


  return std::make_pair(max_entries - n_cut, n_cut); 
}

Hists2D::~Hists2D() {
  for (iterator itr = begin(); itr != end(); itr++){ 
    delete itr->second; 
  }
}

FilterHist2D::FilterHist2D(const LeafInfo& x, const LeafInfo& y, 
			   const DoubleBufferMap& buffer_locations, 
			   int* check_buffer): 
  TH2D(boost::lexical_cast<std::string>(rand()).c_str(),"",
       x.n_bins, x.min, x.max, y.n_bins, y.min, y.max), 
  m_check_buffer(check_buffer), 
  m_x_wt_buffer(0), 
  m_y_wt_buffer(0)
{
  assert(buffer_locations.count(x.name)); 
  assert(buffer_locations.count(y.name)); 
  assert(x.wt_name.size() == 0 || buffer_locations.count(x.wt_name)); 
  assert(y.wt_name.size() == 0 || buffer_locations.count(y.wt_name)); 

  m_x_buffer = buffer_locations.find(x.name)->second; 
  m_y_buffer = buffer_locations.find(y.name)->second; 

  if (x.wt_name.size() != 0) { 
    m_x_wt_buffer = buffer_locations.find(x.wt_name)->second; 
  }
  if (y.wt_name.size() != 0) { 
    m_y_wt_buffer = buffer_locations.find(y.wt_name)->second; 
  }

}

FilterHist2D::FilterHist2D(int x_bins, double x_min, double x_max, 
			   double* x_buffer,
			   int y_bins, double y_min, double y_max, 
			   double* y_buffer, 
			   int* check_buffer, 
			   double* x_wt_buffer, 
			   double* y_wt_buffer):  
  TH2D(boost::lexical_cast<std::string>(rand()).c_str(),"",
       x_bins,x_min, x_max, y_bins, y_min, y_max), 
  m_x_buffer(x_buffer), 
  m_y_buffer(y_buffer), 
  m_check_buffer(check_buffer), 
  m_x_wt_buffer(x_wt_buffer), 
  m_y_wt_buffer(y_wt_buffer)
{
}

FilterHist2D::~FilterHist2D() { 
}

FilterHist2D::FilterHist2D(const FilterHist2D&) 
{ 
}

FilterHist2D& FilterHist2D::operator=(const FilterHist2D& in) { 
  return *this; 
}

int FilterHist2D::fill()
{
  bool pass_check = true; 
  if (m_check_buffer) { 
    pass_check = *m_check_buffer; 
  }
  if (!pass_check) return 0;

  // if there are weights, use this
  if (m_x_wt_buffer || m_y_wt_buffer) { 
    double x_wt = m_x_wt_buffer ? *m_x_wt_buffer : 1.0; 
    double y_wt = m_y_wt_buffer ? *m_y_wt_buffer : 1.0; 
    double wt = x_wt * y_wt; 
    Fill(*m_x_buffer,*m_y_buffer, wt); 
    return 0; 
  }

  // unweighted way to do things
  Fill(*m_x_buffer,*m_y_buffer); 
  return 0; 
}

DoubleBufferMap::~DoubleBufferMap() { 
  for (iterator itr = begin(); itr != end(); itr++){ 
    delete itr->second; 
  }
}
