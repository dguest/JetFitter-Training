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
  m_veto(veto), 
  m_piggyback(0)
{
}

Bitmask::~Bitmask() { 
  if (m_piggyback) { 
    delete m_piggyback; 
  }
}

void Bitmask::piggyback(ICut* piggyback) { 
  if (m_piggyback) { 
    m_piggyback->piggyback(piggyback); 
  }
  else { 
    m_piggyback = piggyback; 
  }
}

bool Bitmask::test() const 
{
  bool pass_required = ( (*m_address & m_required) == m_required); 
  bool has_veto = (*m_address & m_veto); 
  bool pass = (pass_required && !has_veto); 
  bool pass_piggyback = true; 
  if (m_piggyback) { 
    pass_piggyback = m_piggyback->test();
  }
  return pass && pass_piggyback; 
}

IntCheck::IntCheck(int* address, int value): 
  m_address(address), 
  m_value(value), 
  m_piggyback(0)
{
}

IntCheck::~IntCheck() {
  if (m_piggyback) { 
    delete m_piggyback; 
  }
}
void IntCheck::piggyback(ICut* piggyback) { 
  if (m_piggyback) { 
    m_piggyback->piggyback(piggyback); 
  }
  else{ 
    m_piggyback = piggyback; 
  }
}

bool IntCheck::test() const 
{
  bool pass = (*m_address == m_value); 
  bool pass_piggyback = true; 
  if (m_piggyback) { 
    pass_piggyback = m_piggyback->test(); 
  }
  return pass && pass_piggyback; 
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
