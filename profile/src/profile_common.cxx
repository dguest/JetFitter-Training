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

Bitmask::Bitmask(unsigned* address, unsigned required, unsigned veto): 
  m_address(address), 
  m_required(required), 
  m_veto(veto) 
{
}

bool Bitmask::test() const 
{
  bool pass_required = (*m_address & m_required == *m_address); 
  bool has_veto = (*m_address & m_veto); 
  return pass_required && !has_veto; 
}

IntCheck::IntCheck(int* address, int value): 
  m_address(address), 
  m_value(value)
{
}
bool IntCheck::test() const 
{
  return *m_address == m_value; 
}

BitBuffer::~BitBuffer() 
{
  for (iterator itr = begin(); itr != end(); itr++) { 
    delete itr->second; 
  }
}

CutContainer::~CutContainer() 
{
  for (iterator itr = begin(); itr != end(); itr++) { 
    delete itr->second; 
  }
}
