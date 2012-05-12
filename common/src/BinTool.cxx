#include "BinTool.hh"
#include <map> 
#include <vector>
#include <cassert>
// #include <utility>
// #include <cmath>


BinTool::BinTool(const std::vector<double>& v)
{ 
  for (size_t category = 0; category < v.size(); category++){ 
    double bin_upper_bound = v.at(category); 
    this->insert(std::make_pair(bin_upper_bound, category)); 
  }
  
}



int BinTool::get_bin(double value)
{
  std::map<double,int>::const_iterator bin = upper_bound(value); 
  if ( bin == end() ) { 
    // max bin number defined by n points is n if we start with zero
    return size(); 
  }
  return bin->second; 
}

