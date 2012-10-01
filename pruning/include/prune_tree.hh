#ifndef PRUNE_TREE
#define PRUNE_TREE

#include <string> 
#include <map>
#include <vector> 
#include <set> 
#include <utility> //pair

struct SubTreeIntInfo 
{ 
  std::string name; 
  int value; 
}; 
struct SubTreeDoubleInfo 
{
  std::string name; 
  double low; 
  double high; 
}; 
struct SubTreeUnsignedInfo
{
  std::string name; 
  unsigned required; 
  unsigned veto; 
}; 

struct Cuts
{
  std::vector<SubTreeIntInfo> ints; 
  std::vector<SubTreeDoubleInfo> doubles; 
  std::vector<SubTreeUnsignedInfo> bits; 
}; 

int simple_prune(std::string file_name, 
		 std::string tree_name, 
		 Cuts cuts, 
		 std::set<std::string> subset, 
		 std::string output_file_name, 
		 int max_entries = -1, 
		 const unsigned options = 0);

class SubTreeCut
{
public: 
  virtual ~SubTreeCut() {}
  virtual bool check() const = 0; 
  virtual std::string name() = 0; 
}; 

class SubTreeIntCut: public SubTreeCut
{
public: 
  SubTreeIntCut(std::string name, int value, int* ptr); 
  ~SubTreeIntCut() {}
  bool check() const; 
  std::string name(); 
private: 
  std::string m_variable; 
  int m_value; 
  int* m_ptr; 
}; 

class SubTreeDoubleCut: public SubTreeCut
{
public: 
  SubTreeDoubleCut(std::string name, double low, double up, double* ptr);
  ~SubTreeDoubleCut() {}
  bool check() const; 
  std::string name(); 
private: 
  std::string m_variable; 
  double m_upper; 
  double m_lower; 
  double* m_ptr; 
}; 

class SubTreeBitmaskCut: public SubTreeCut
{
public: 
  SubTreeBitmaskCut(std::string name, 
		    unsigned required, unsigned veto, 
		    unsigned* ptr);
  ~SubTreeBitmaskCut() {}
  bool check() const; 
  std::string name(); 
private: 
  std::string m_variable; 
  unsigned m_required; 
  unsigned m_veto; 
  unsigned* m_ptr; 
}; 

class IntBuffer: public std::map<std::string, int*>
{
public: 
  ~IntBuffer(); 
}; 

class UnsignedBuffer: public std::map<std::string, unsigned*> 
{
public: 
  ~UnsignedBuffer(); 
}; 

class DoubleBuffer: public std::map<std::string, double*>
{
public: 
  ~DoubleBuffer(); 
}; 

class SubTreeCuts: public std::vector<SubTreeCut*>
{
public: 
  ~SubTreeCuts(); 
}; 

namespace opt 
{
  const unsigned verbose = 1u << 0; 
}

#endif // PRUNE_TREE
