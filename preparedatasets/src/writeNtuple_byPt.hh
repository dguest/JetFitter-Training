#ifndef WRITE_NTUPLE_BYPT_H
#define WRITE_NTUPLE_BYPT_H

#include <string> 
#include <vector> 
#include "ntuple_defaults.hh"
#include "writeNtuple_common.hh"

int writeNtuple_byPt(SVector input_files, 
		     Observers observers, 
		     std::string jetCollectionName = defopt::JCN, 
		     std::string output_file = defopt::OFN, 
		     std::string suffix = "AOD");

class FlavorCountPtEta
{
public: 
  FlavorCountPtEta(size_t n_pt, size_t n_eta); 
  void increment(size_t cat_pt, size_t cat_eta); 
  void compute(); 
private: 
  size_t _n_pt; 
  size_t _n_eta; 
  bool _is_computed; 
  std::vector< std::vector<double> > _pt_eta_count; 
  std::vector< std::vector<double> > _pt_eta_weight; 
  std::vector<double> _pt_count; 
  std::vector<double> _pt_max_weights; 
}; 
  
#endif // WRITE_NTUPLE_BYPT_H
