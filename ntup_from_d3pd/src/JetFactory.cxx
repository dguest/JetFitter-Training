#include "JetFactory.hh"
#include <cassert> 
#include <stdexcept>
#include <boost/format.hpp>

#include "TChain.h"


Jet::Jet(JetCollectionBuffer* buffer, int d3pd_index) 
{ 
  set_kinematics(buffer, d3pd_index); 
  set_jfc(buffer, d3pd_index); 
  set_cnn(buffer, d3pd_index); 
  set_mv1(buffer, d3pd_index); 
}


void Jet::set_kinematics(const JetCollectionBuffer* buffer, int index) { 
  double pt = buffer->pt->at(index); 
  double eta = buffer->eta->at(index); 
  double phi = buffer->phi->at(index); 
  double e = buffer->e->at(index); 
  
  SetPtEtaPhiE(pt, eta, phi, e); 
}

void Jet::set_jfc(const JetCollectionBuffer* buffer, int index) {
  m_anti_u[JFC] = log(buffer->jfc_pc->at(index) / 
		      buffer->jfc_pu->at(index)); 
  m_anti_b[JFC] = log(buffer->jfc_pc->at(index) / 
		      buffer->jfc_pb->at(index)); 
}

void Jet::set_cnn(const JetCollectionBuffer* buffer, int index) {
  m_anti_u[COMBNN] = log(buffer->cnn_pc->at(index) / 
			 buffer->cnn_pu->at(index)); 
  m_anti_b[COMBNN] = log(buffer->cnn_pc->at(index) / 
			 buffer->cnn_pb->at(index)); 
}

void Jet::set_mv1(const JetCollectionBuffer* buffer, int index) { 
  m_anti_u[MV1] = buffer->mv1->at(index); 
  m_anti_b[MV1] = -buffer->mv1->at(index); 
}


JetFactory::JetFactory(TChain* chain) : 
  m_chain(chain)
{ 
}

JetFactory::~JetFactory() { 
  for (BufferContainer::iterator itr = m_buffers.begin(); 
       itr != m_buffers.end(); itr++) { 
    delete itr->second; 
    itr->second = 0; 
  }
}

void JetFactory::add_collection(std::string collection) { 
  assert(m_buffers.count(collection) == 0); 
  m_buffers[collection] = new JetCollectionBuffer; 
  JetCollectionBuffer& b = *m_buffers[collection]; 
  
  set_branch("jet_" + collection + "_pt", b.pt); 
  set_branch("jet_" + collection + "_eta", b.eta); 
  set_branch("jet_" + collection + "_phi", b.phi); 
  set_branch("jet_" + collection + "_e", b.e); 
  
  
}

void JetFactory::set_branch(std::string name, void* branch) { 
  if (m_set_branches.count(name) != 0) { 
    throw std::runtime_error(name + " was set twice"); 
  }
  m_set_branches.insert(name); 

  m_chain->SetBranchStatus(name.c_str(),1); 
  int ret_code = m_chain->SetBranchAddress(name.c_str(), branch); 

  if (ret_code != 0 && ret_code != 5 ){ 
    std::string issue = (boost::format("can not set %s , return code %i") % 
			 name % ret_code).str(); 
    throw std::runtime_error(issue); 
  }
  if (! m_chain->GetBranch(name.c_str())) { 
    std::string issue = (boost::format("can't find branch %s") % name).str(); 
    throw std::runtime_error(issue); 
  }
  
  
}
