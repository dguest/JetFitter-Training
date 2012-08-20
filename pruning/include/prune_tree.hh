#ifndef PRUNE_TREE
#define PRUNE_TREE

#include <string> 
#include <map>
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

int simple_prune(std::string file_name, 
		 std::string tree_name, 
		 std::vector<SubTreeIntInfo> int_cuts, 
		 std::vector<SubTreeDoubleInfo> double_cuts, 
		 std::string output_file_name, 
		 int max_entries = -1, 
		 const unsigned options = 0);

class SubTreeCut
{
public: 
  virtual SubTreeCut() = 0; 
  virtual ~SubTreeCut() {}
  virtual bool check() = 0; 
  virtual std::string name() = 0; 
}; 

class SubTreeIntCut: public SubTreeCut
{
public: 
  SubTreeIntCut(std::string name, int value, int* ptr); 
  ~SubTreeIntCut() {}
  bool check(); 
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
  bool check(); 
  std::string name(); 
private: 
  std::string m_variable; 
  double m_upper; 
  double m_lower; 
  double* m_ptr; 
}; 

class IntBuffer: public std::map<std::string, int*>
{
public: 
  ~IntBuffer(); 
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
}

namespace opt 
{
  unsigned verbose = 1u << 0; 
}

// class CheckedTree
// {
// public: 
//   CheckedTree(std::string name); 
//   ~CheckedTree(); 
//   void AddObserver(std::pair<std::string, int*>); 
//   void AddObserver(std::pair<std::string, double*>); 
//   void AddCut(SubTreeCut); 
//   void write_to(TFile&); 
//   void fill(); 
// private: 
//   std::vector<SubTreeCut*> m_cuts; 
//   std::vector<int*> m_ints; 
//   std::vector<double*> m_doubles; 
//   TTree* m_tree; 
//   bool m_wrote; 
// }; 

#endif // PRUNE_TREE
