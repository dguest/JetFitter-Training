#include "profile_common.hh"
#include <vector> 

CheckBuffer::~CheckBuffer() { 
  for (iterator itr = begin(); itr != end(); itr++){ 
    delete itr->second; 
  }
}

RangeCut::RangeCut(double* value, double lower, double upper): 
  m_value(value), 
  m_lower(lower), 
  m_upper(upper) 
{
}

bool RangeCut::in_range() const 
{
  bool above = *m_value >= m_lower; 
  bool below = *m_value <= m_upper; 
  return above && below; 
}

bool is_in_range(const std::vector<RangeCut>& cuts)
{
  for (std::vector<RangeCut>::const_iterator itr = cuts.begin(); 
	 itr != cuts.end(); itr++){ 
    if (!itr->in_range()) { 
      return false; 
    }
  }
  return true; 
}
