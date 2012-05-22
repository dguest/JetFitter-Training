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

class RangeCut { 
public: 
  RangeCut(double* value, double lower, double upper); 
  bool in_range() const; 
private: 
  double* m_value; 
  double m_lower; 
  double m_upper; 
}; 

bool is_in_range(const std::vector<RangeCut>&); 

int profile_fast(std::string file, 
		 std::string tree, 
		 std::vector<LeafInfo> int_leaves, 
		 std::vector<LeafInfo> double_leaves, 
		 std::string output_file_name, 
		 int max_entries = -1, 
		 int n_bins = 500); 

#endif // PROFILE_FAST_H
