#ifndef PROFILE_CONSTANTS_H
#define PROFILE_CONSTANTS_H

#include <string> 

namespace opt { 
  const unsigned show_progress = 1u << 0; 

  const unsigned def_opt       = 0; 
}

namespace magic { 
  const int MAX_AUTO_BINS = 100000; 
} 

#endif // PROFILE_CONSTANTS_H
