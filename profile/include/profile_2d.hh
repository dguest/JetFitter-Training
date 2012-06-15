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

class FilterHist2D: public TH2D
{
public: 
  FilterHist2D(int x_bins, double x_min, double x_max, double* x_buffer,
	       int y_bins, double y_min, double y_max, double* y_buffer, 
	       int* check_buffer = 0); 
  ~FilterHist2D(); 
  int fill(); 
private: 
  FilterHist2D(const FilterHist2D&); 
  FilterHist2D& operator=(const FilterHist2D&); 
  double* m_x_buffer; 
  double* m_y_buffer; 
  int* m_check_buffer; 
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

std::pair<int,int> pro_2d(std::string file, 
			  std::string tree, 
			  LeafInfoPairs plots, 
			  std::vector<std::string> tag_leaves, 
			  std::string output_file_name, 
			  int max_entries = -1, 
			  const unsigned options = opt::def_opt); 

#endif // PROFILE_2D_H
