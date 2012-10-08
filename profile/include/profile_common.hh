#ifndef PROFILE_COMMON_H
#define PROFILE_COMMON_H

#include <string> 
#include <map>
#include <vector> 

struct LeafInfo
{
  std::string name; 
  std::string wt_name; 
  double max; 
  double min; 
  int n_bins; 
  std::vector<double> bin_bounds; 
};

struct TagInfo
{
  std::string name; 
  std::string leaf_name; 
  int value; 
};
 
struct MaskInfo
{
  std::string name; 
  std::string leaf_name;
  unsigned accept_mask; 
  unsigned veto_mask; 
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

class ICut
{
public: 
  virtual ~ICut() {}
  virtual bool test() const = 0; 
  virtual void piggyback(ICut*) = 0; 
}; 

class Bitmask : public ICut
{
public: 
  Bitmask(unsigned* address, unsigned required, unsigned veto = 0); 
  ~Bitmask(); 
  void piggyback(ICut*); 
  bool test() const; 
private: 
  unsigned* m_address; 
  unsigned m_required; 
  unsigned m_veto; 

  ICut* m_piggyback; 
}; 

class IntCheck: public ICut
{
public: 
  IntCheck(int* address, int value = 1); 
  ~IntCheck(); 
  void piggyback(ICut*); 
  bool test() const; 
private: 
  int* m_address; 
  int m_value; 

  ICut* m_piggyback; 
}; 

class BitBuffer: public std::map<std::string, unsigned*> 
{ 
public: 
  ~BitBuffer(); 
}; 

class CutContainer: public std::map<std::string, ICut*>
{
public: 
  ~CutContainer(); 
}; 

#endif // profile_common_h
