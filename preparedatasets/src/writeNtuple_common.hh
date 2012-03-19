#ifndef WRITE_NTUPLE_COMMON_H
#define WRITE_NTUPLE_COMMON_H

#include <ostream>
#include <vector>

typedef std::vector<std::string> SVector; 

class LoadOfficialDSException {};

struct Observers { 
  SVector discriminators; 
  SVector double_variables; 
  SVector int_variables; 
  bool build_default_values(); 
}; 

std::ostream& operator<<(std::ostream&, const Observers&); 

#endif // WRITE_NTUPLE_COMMON_H
