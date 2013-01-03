#ifndef TREE_TRANSLATOR_H
#define TREE_TRANSLATOR_H

#include <vector>
#include <string> 
#include <boost/noncopyable.hpp>

class TChain; 
class PerfNtupleBuilder; 
class JetFactory; 
class Jet; 

namespace trans { 
  const unsigned skip_taus = 1u << 0; 
}

class TreeTranslator : public boost::noncopyable 
{
public: 
  TreeTranslator(std::string in_tree_name, 
		 std::string out_file_name = "", 
		 std::string out_tree = "SVTree", 
		 unsigned flags = 0); 
  ~TreeTranslator(); 
  void add_collection(std::string collection); 
  void translate(std::vector<std::string> d3pds); 
  void set_float(std::string name, float value); 
private: 
  void init_chain(std::vector<std::string> d3pds); 
  void copy_taggers(const Jet&, PerfNtupleBuilder*) const; 
  bool is_good_jet(const Jet&) const; 
  std::string m_collection; 
  std::string m_in_chain_name; 

  PerfNtupleBuilder* m_ntuple_builder; 
  JetFactory* m_factory; 
  TChain* m_in_chain; 

  float m_min_pt; 
  float m_max_pt; 
  float m_max_abs_eta; 

  unsigned m_flags; 
}; 

#endif 
