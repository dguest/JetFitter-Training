#ifndef BJET_FACTORY_H
#define BJET_FACTORY_H

#include <map>
#include <string> 
#include <boost/noncopyable.hpp>
#include "TLorentzVector.h"

class TChain; 

enum Tagger { 
  COMBNN, 
  JFC, 
  MV1, 
  MV1C
}; 

class Jet : public TLorentzVector
{ 
public: 
  Jet(JetCollectionBuffer*); 
  double anti_b(Tagger); 
  double anti_u(Tagger); 
private: 
  std::map<Tagger, double> m_anti_u; 
  std::map<Tagger, double> m_anti_b; 
}; 

// these guys are owned by JetFactory
struct JetCollectionBuffer { 
  std::vector<double>* pt; 
  std::vector<double>* eta; 
  std::vector<double>* phi; 
  std::vector<double>* e; 

  std::vector<double>* jfc_pu; 
  std::vector<double>* jfc_pc; 
  std::vector<double>* jfc_pb; 

  std::vector<double>* cnn_pu; 
  std::vector<double>* cnn_pc; 
  std::vector<double>* cnn_pb; 
  
  std::vector<double>* mv1; 
  std::vector<double>* mv1c; 
  
}; 

class JetFactory : public boost::noncopyable { 
public: 
  JetFactory(TChain* chain); 
  ~JetFactory(); 
  void add_collection(std::string collection); 
  std::vector<Jet> jets(std::string collection); 
private: 
  TChain* m_chain; 
  std::map<std::string, JetCollectionBuffer*> m_buffers; 
}; 

#endif 
