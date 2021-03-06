#ifndef PROFILE_2D_H
#define PROFILE_2D_H

#include <string> 
#include <vector>
#include <utility> // pair
#include <map>
#include "TH2D.h"
#include "profile_constants.hh"
#include "profile_common.hh"

namespace magic { 
  const int DEF_2D_BINS = 200; 
} 


class TChain; 
class DoubleBufferMap; 

class FilterHist2D: public TH2D
{
public: 
  FilterHist2D(const LeafInfo& x_info, const LeafInfo& y_info, 
	       const DoubleBufferMap& buffer_locations, 
	       std::vector<ICut*> cuts); 
  ~FilterHist2D(); 
  int fill(); 
private: 
  FilterHist2D(const FilterHist2D&); 
  FilterHist2D& operator=(const FilterHist2D&); 
  
  std::vector<ICut*>  m_cuts; 
  
  double* m_x_buffer; 
  double* m_y_buffer; 

  double* m_x_wt_buffer; 
  double* m_y_wt_buffer; 
 
};


class Hists2D : public std::map<std::string,FilterHist2D*>
{
public: 
  ~Hists2D(); 
}; 

class DoubleBufferMap: public std::map<std::string,double*> 
{
public: 
  ~DoubleBufferMap(); 
};

// typedef std::vector<std::pair<LeafInfo,LeafInfo> > LeafInfoPairs; 
class LeafInfoPairs: public std::vector<std::pair<LeafInfo,LeafInfo> > {}; 

struct ProfileInfo { 
  std::string file_name; 
  std::vector<std::string> hist_names; 
}; 

ProfileInfo pro_2d(std::string file, 
		   std::string tree, 
		   LeafInfoPairs plots, 
		   std::vector<TagInfo> tag_leaves, 
		   std::vector<MaskInfo> masks, 
		   std::string output_file_name, 
		   int max_entries = -1, 
		   const unsigned options = opt::def_opt); 

void cast_int_to_double(std::pair<int*,double*>); 
int set_smart_bins(LeafInfo&); 

#endif // PROFILE_2D_H
