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
#include "TLeaf.h"

ProfileInfo pro_2d(std::string file_name, 
		   std::string tree_name, 
		   LeafInfoPairs plots, 
		   std::vector<TagInfo> tag_leaves, 
		   std::vector<MaskInfo> masks, 
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

  // TODO: unify these cuts with the bit cuts below by replacing with IntCut
  CutContainer cuts; 
  CheckBuffer check_buffer; 
  for (std::vector<TagInfo>::const_iterator itr = tag_leaves.begin(); 
       itr != tag_leaves.end(); itr++){ 
    if (!check_buffer.count(itr->leaf_name)) { 
      check_buffer[itr->leaf_name] = new int; 
    }
    int* address = check_buffer[itr->leaf_name]; 
    if (cuts.count(itr->name) ) { 
      cuts[itr->name]->piggyback(new IntCheck(address, itr->value)); 
    }
    else{ 
      cuts[itr->name] = new IntCheck(address, itr->value); 
    }

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

  BitBuffer bit_buffer; 
  for (std::vector<MaskInfo>::const_iterator itr = masks.begin(); 
       itr != masks.end(); itr++) { 
    if (!bit_buffer.count(itr->leaf_name)) { 
      bit_buffer[itr->leaf_name] = new unsigned; 
    }
    unsigned* address = bit_buffer[itr->leaf_name]; 

    if (cuts.count(itr->name) ) { 
      cuts[itr->name]->piggyback(new Bitmask(address, 
					    itr->accept_mask, 
					    itr->veto_mask)); 
    }
    else { 
      cuts[itr->name] = new Bitmask(address, itr->accept_mask, itr->veto_mask);
    }
  }
  for (BitBuffer::const_iterator itr = bit_buffer.begin(); 
       itr != bit_buffer.end(); itr++) { 
    tree->SetBranchStatus(itr->first.c_str(), 1); 
    bool error = tree->SetBranchAddress(itr->first.c_str(), itr->second); 
    if (error) { 
      throw std::runtime_error("could not find bitmask branch " + 
			       itr->first); 
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
    if (!double_buffer.count(itr->first.wt_name) && 
	itr->first.wt_name.size() != 0)
      double_buffer[itr->first.wt_name] = new double; 
    if (!double_buffer.count(itr->second.wt_name) &&
	itr->second.wt_name.size() != 0)
      double_buffer[itr->second.wt_name] = new double; 
  }

  CheckBuffer int_buffer; 
  std::vector<std::pair<int*,double*> > ints_to_double_cast; 
  // set all (double) branches
  for (DoubleBufferMap::const_iterator itr = 
	 double_buffer.begin(); 
       itr != double_buffer.end(); 
       itr++){ 
    TLeaf* the_leaf = tree->GetLeaf(itr->first.c_str()); 
    if (!the_leaf) 
      throw std::runtime_error("could not find branch " + itr->first); 

    tree->SetBranchStatus(itr->first.c_str(),1); 

    bool error = false; 

    std::string type = the_leaf->GetTypeName();
    if (type == "Int_t") { 
      int* int_ptr = 0; 
      if (check_buffer.count(itr->first)) { 
	int_ptr = check_buffer.find(itr->first)->second; 
      }
      else if (int_buffer.count(itr->first)) { 
	int_ptr = check_buffer.find(itr->first)->second; 
      }
      else { 
	int_ptr = new int; 
	int_buffer[itr->first] = int_ptr; 
      }
      ints_to_double_cast.push_back(std::make_pair(int_ptr, itr->second)); 
      error = tree->SetBranchAddress(itr->first.c_str(),int_ptr); 
    }
    else if (type == "Double_t") { 
      error = tree->SetBranchAddress(itr->first.c_str(),itr->second); 
    }
    else {
      throw std::runtime_error("branch " + itr->first + 
			       " is of unsupported type " + type); 
    }
    if (error) 
      throw std::runtime_error("problem setting branch " + itr->first); 

  }

  std::vector<ICut*> no_cuts; 

  // std::vector<RangeCut> cuts; 

  int n_entries = tree->GetEntries(); 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  Hists2D hists; 

  for (LeafInfoPairs::iterator leaf_itr = plots.begin(); 
       leaf_itr != plots.end(); 
       leaf_itr++){ 

    set_smart_bins(leaf_itr->first); 
    set_smart_bins(leaf_itr->second); 

    std::string x_var_name = leaf_itr->first.name; 
    std::string y_var_name = leaf_itr->second.name; 

    if (leaf_itr->first.wt_name.size() != 0) { 
      x_var_name.append("_" + leaf_itr->first.wt_name); 
    }
    if (leaf_itr->second.wt_name.size() != 0) { 
      y_var_name.append("_" + leaf_itr->second.wt_name); 
    }

    std::string hist_name = y_var_name + "_vs_" + x_var_name; 

    //TODO: compound cuts should have been solved by piggybacking
    // remove the cut vector (we only need a pointer to one cut)
    std::map<std::string, ICut*> used_cuts; 
    used_cuts.insert(std::make_pair<std::string, ICut*>("",0)); 
    for (CutContainer::const_iterator itr = cuts.begin(); 
	 itr != cuts.end(); itr++) { 
      used_cuts.insert(*itr); 
    }
    for (CutContainer::const_iterator cut_itr = used_cuts.begin(); 
	 cut_itr != used_cuts.end(); cut_itr++) { 

      // we would combine cuts here... 
      std::vector<ICut*> cut_vector; 
      if (cut_itr->second) { 
	cut_vector.push_back(cut_itr->second); 
      }

      std::string cut_hist_name = hist_name; 
      if (cut_itr->first.size() > 0) { 
	cut_hist_name.append("_" + cut_itr->first); 
      }

      if (hists.count(cut_hist_name) ) { 
	throw std::runtime_error("attempted to redefine " + cut_hist_name); 
      }
      hists[cut_hist_name] = new FilterHist2D(leaf_itr->first, 
					      leaf_itr->second, 
					      double_buffer, cut_vector); 
    }
  }


  bool show_progress = options & opt::show_progress; 
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

    std::for_each(ints_to_double_cast.begin(), ints_to_double_cast.end(), 
		  cast_int_to_double); 

    for (Hists2D::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
      itr->second->fill(); 
    }
  }
  if (show_progress) std::cout << "\n"; 

  ProfileInfo return_info; 
  return_info.file_name = output_file_name; 
  TFile out_file(output_file_name.c_str(),"recreate"); 

  for (Hists2D::iterator itr = hists.begin(); itr != hists.end(); itr++){ 
    itr->second->SetName(itr->first.c_str()); 
    out_file.WriteTObject(itr->second); 
    return_info.hist_names.push_back(itr->first); 
  }

  return return_info; 
}

Hists2D::~Hists2D() {
  for (iterator itr = begin(); itr != end(); itr++){ 
    delete itr->second; 
  }
}

FilterHist2D::FilterHist2D(const LeafInfo& x, const LeafInfo& y, 
			   const DoubleBufferMap& buffer_locations, 
			   std::vector<ICut*> cuts): 
  TH2D(boost::lexical_cast<std::string>(rand()).c_str(),"",
       x.n_bins, x.min, x.max, y.n_bins, y.min, y.max), 
  m_cuts(cuts), 
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

  // rebinning for variable bin sizes
  // written while ROOT documentation was down 
  if (x.bin_bounds.size() != 0 || y.bin_bounds.size() != 0) { 
    assert(x.bin_bounds.size() == unsigned(x.n_bins + 1) || 
	   x.bin_bounds.size() == 0); 
    assert(y.bin_bounds.size() == unsigned(y.n_bins + 1) || 
	   y.bin_bounds.size() == 0); 
    double* x_bins = new double[x.n_bins + 1]; 
    double* y_bins = new double[y.n_bins + 1]; 

    for (int iii = 0; iii < x.n_bins + 1; iii++) { 
      if (x.bin_bounds.size() > 0) { 
	x_bins[iii] = x.bin_bounds.at(iii); 
      }
      else { 
	double inc = (x.max - x.min) / x.n_bins; 
	x_bins[iii] = x.min + inc * iii; 
      }
    }

    for (int iii = 0; iii < y.n_bins + 1; iii++) { 
      if (y.bin_bounds.size() > 0) { 
	y_bins[iii] = y.bin_bounds.at(iii); 
      }
      else { 
	double inc = (y.max - y.min) / y.n_bins; 
	y_bins[iii] = y.min + inc * iii; 
      }
    }

    SetBins(x.n_bins, x_bins, y.n_bins, y_bins); 

    delete x_bins; 
    delete y_bins; 
  }

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

  for (std::vector<ICut*>::const_iterator itr = m_cuts.begin(); 
       itr != m_cuts.end(); itr++) { 
    const ICut& cut = **itr; 
    if (!cut.test()) { 
      return 0; 
    }
  }

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

void cast_int_to_double(std::pair<int*,double*> the_pair) { 
  *the_pair.second = *the_pair.first; 
}

int set_smart_bins(LeafInfo& info) { 
  if (info.n_bins > 0) return info.n_bins; 
  
  if (info.n_bins == -1 || info.n_bins == 0) { 
    info.n_bins = magic::DEF_2D_BINS; 
    return info.n_bins; 
  }

  if (info.n_bins == -2) { 
    info.n_bins = int(round(info.max - info.min + 1)); 
    info.min -= 0.5; 
    info.max += 0.5; 
    return info.n_bins; 
  }
  
  throw std::runtime_error("don't know how to intemperate " + 
			   boost::lexical_cast<std::string>(info.n_bins) ); 
  
}
