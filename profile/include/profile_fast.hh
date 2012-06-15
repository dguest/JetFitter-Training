#ifndef PROFILE_FAST_H
#define PROFILE_FAST_H

#include <string> 
#include <vector>
#include <utility> // pair
#include <map>
#include "TH1D.h"
#include "profile_constants.hh"
#include "profile_common.hh"

class TChain; 

class FilterHist: public TH1D
{
public: 
  FilterHist(int n_bins, double min, double max, 
	     double* in_buffer, int* check_buffer = 0); 
  ~FilterHist(); 
  int fill(); 
private: 
  FilterHist(const FilterHist&); 
  FilterHist& operator=(const FilterHist&); 
  double* m_in_buffer; 
  int* m_check_buffer; 
};


class Hists : public std::map<std::string,FilterHist*>
{
public: 
  ~Hists(); 
}; 




std::pair<int,int> profile_fast(std::string file, 
				std::string tree, 
				std::vector<LeafInfo> int_leaves, 
				std::vector<LeafInfo> double_leaves, 
				std::vector<std::string> tag_leaves, 
				std::string output_file_name, 
				int max_entries = -1, 
				const unsigned options = opt::def_opt); 

#endif // PROFILE_FAST_H
