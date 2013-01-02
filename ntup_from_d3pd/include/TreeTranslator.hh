#ifndef TREE_TRANSLATOR_H
#define TREE_TRANSLATOR_H

#include <vector>
#include <string> 
#include <boost/noncopyable.hpp>

class TChain; 
class PerfNtupleBuilder; 
class JetFactory; 
class Jet; 

class TreeTranslator : public boost::noncopyable 
{
public: 
  TreeTranslator(std::string in_tree_name, 
		 std::string out_file_name = "", 
		 std::string out_tree = "SVTree"); 
  ~TreeTranslator(); 
  void add_collection(std::string collection); 
  void translate(std::vector<std::string> d3pds); 
private: 
  void init_chain(std::vector<std::string> d3pds); 
  void copy_taggers(const Jet&, PerfNtupleBuilder*) const; 
  std::string m_collection; 
  std::string m_in_chain_name; 

  PerfNtupleBuilder* m_ntuple_builder; 
  JetFactory* m_factory; 
  TChain* m_in_chain; 
}; 

#endif 
