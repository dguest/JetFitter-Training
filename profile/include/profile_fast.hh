#ifndef PROFILE_FAST_H
#define PROFILE_FAST_H

#include <string> 
#include <vector>
#include "TH1D.h"

class TChain; 

struct LeafInfo
{
  std::string name; 
  double max; 
  double min; 
};

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

int profile_fast(std::string file, 
		 std::string tree, 
		 std::vector<LeafInfo> int_leaves, 
		 std::vector<LeafInfo> double_leaves, 
		 std::string output_file_name); 

#endif // PROFILE_FAST_H
