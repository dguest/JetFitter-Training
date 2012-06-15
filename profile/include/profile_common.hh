#ifndef PROFILE_COMMON_H
#define PROFILE_COMMON_H

#include <string> 
#include <map>
#include <vector> 

struct LeafInfo
{
  std::string name; 
  double max; 
  double min; 
  int n_bins; 
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

class CheckBuffer: public std::map<std::string,int*> 
{
public: 
  ~CheckBuffer(); 
};

bool is_in_range(const std::vector<RangeCut>&); 


#endif // profile_common_h
