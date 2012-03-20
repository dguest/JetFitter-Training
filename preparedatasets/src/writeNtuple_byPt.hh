#ifndef WRITE_NTUPLE_BYPT_H
#define WRITE_NTUPLE_BYPT_H

#include <string> 
#include <vector> 
#include "ntuple_defaults.hh"
#include "writeNtuple_common.hh"

int writeNtuple_byPt(SVector input_files, 
		     Observers observers, 
		     std::vector<double> pt_cat_vec, 
		     std::string jetCollectionName = defopt::JCN, 
		     std::string output_dir = "nothing",
		     std::string suffix = "AOD");

class FlavorCountPtEta
{
public: 
  FlavorCountPtEta(size_t n_pt, size_t n_eta, double tolerance = 1); 
  void increment(size_t cat_pt, size_t cat_eta); 
  void compute(); 
  double get_weight(size_t cat_pt, size_t cat_eta) const; 
private: 
  size_t _n_pt; 
  size_t _n_eta; 
  double _tolerance; 
  bool _is_computed; 
  std::vector< std::vector<double> > _pt_eta_count; 
  std::vector< std::vector<double> > _pt_eta_weight; 
  std::vector<double> _pt_count; 
  std::vector<double> _pt_max_weights; 
}; 
  
#endif // WRITE_NTUPLE_BYPT_H
