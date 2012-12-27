#ifndef TREE_TRANSLATOR_H
#define TREE_TRANSLATOR_H

#include <vector>
#include <string> 
#include <boost/noncopyable.hpp>

class TChain; 
class TTree; 
class TFile; 
class JetFactory; 

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
  std::string m_collection; 
  std::string m_in_chain_name; 

  TTree* m_out_ntuple; // owned by m_out_file
  TFile* m_out_file; 
  JetFactory* m_factory; 
  TChain* m_in_chain; 
}; 

#endif 
