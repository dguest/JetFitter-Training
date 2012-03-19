#include "writeNtuple_common.hh"
#include <vector> 
#include <ostream> 

bool Observers::build_default_values()
{ 
  bool init_doubles = false; 
  if (double_variables.size() == 0) { 
    double_variables.push_back("energyFraction"); 
    double_variables.push_back("significance3d"); 
    init_doubles = true; 
  }
  bool init_ints = false; 
  if (int_variables.size() == 0) { 
    int_variables.push_back("nVTX"); 
    int_variables.push_back("nTracksAtVtx"); 
    int_variables.push_back("nSingleTracks"); 
    
    init_ints = true; 
  }
  return init_ints || init_doubles;
}

std::ostream& operator<<(std::ostream& out, const Observers& v)
{
  typedef std::vector<std::string>::const_iterator SVecItr; 
  out << "discriminators: "; 
  for (SVecItr itr = v.discriminators.begin(); 
       itr != v.discriminators.end(); itr++){ 
    out << *itr << ", "; 
  }
  out << "continuous variables: "; 
  for (SVecItr itr = v.double_variables.begin(); 
       itr != v.double_variables.end(); itr++){ 
    out << *itr << ", "; 
  }
  out << "discrete variables: "; 
  for (SVecItr itr = v.int_variables.begin(); itr != v.int_variables.end(); 
       itr++){ 
    out << *itr << ", "; 
  }
  return out; 
}
