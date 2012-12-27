#ifndef BJET_FACTORY_H
#define BJET_FACTORY_H

#include <map>
#include <set> 
#include <string> 
#include <boost/noncopyable.hpp>
#include "TLorentzVector.h"

class TChain; 
class JetCollectionBuffer; 

enum Tagger { 
  COMBNN, 
  JFC, 
  MV1, 
  MV1C
}; 

class Jet : public TLorentzVector
{ 
public: 
  Jet(const JetCollectionBuffer*, int); 
  double anti_b(Tagger) const; 
  double anti_u(Tagger) const; 
private: 
  void set_kinematics(const JetCollectionBuffer* buffer, int index); 
  void set_jfc(const JetCollectionBuffer* buffer, int index); 
  void set_cnn(const JetCollectionBuffer* buffer, int index); 
  void set_mv1(const JetCollectionBuffer* buffer, int index); 

  std::map<Tagger, double> m_anti_u; 
  std::map<Tagger, double> m_anti_b; 
}; 

// these guys are owned by JetFactory
struct JetCollectionBuffer { 
  int n; 
  std::vector<float>* pt; 
  std::vector<float>* eta; 
  std::vector<float>* phi; 
  std::vector<float>* e; 

  std::vector<float>* jfc_pu; 
  std::vector<float>* jfc_pc; 
  std::vector<float>* jfc_pb; 

  std::vector<float>* cnn_pu; 
  std::vector<float>* cnn_pc; 
  std::vector<float>* cnn_pb; 

  std::vector<float>* mv1; 
  std::vector<float>* mv1c; 
  
}; 

class JetFactory : public boost::noncopyable { 
public: 
  JetFactory(TChain* chain); 
  ~JetFactory(); 
  void add_collection(std::string collection); 
  std::vector<Jet> jets(std::string collection) const; 
private: 
  template<typename T> void set_branch(std::string name, T* branch); 
  void set_branch_without_zeroing(std::string name, void* branch); 
  typedef std::map<std::string, JetCollectionBuffer*> BufferContainer; 
  TChain* m_chain; 
  BufferContainer m_buffers; 
  std::set<std::string> m_set_branches; 
}; 

template<typename T> 
void JetFactory::set_branch(std::string name, T* branch) { 
  *branch = 0; 
  set_branch_without_zeroing(name, branch); 
}

#endif 
