#include "JetFactory.hh"
#include "Mv1c.hh"
#include <cassert> 
#include <stdexcept>
#include <boost/format.hpp>

#include "TChain.h"


Jet::Jet(const JetCollectionBuffer* buffer, int d3pd_index) 
{ 
  set_kinematics(buffer, d3pd_index); 
  set_jfc(buffer, d3pd_index); 
  set_cnn(buffer, d3pd_index); 
  set_mv1(buffer, d3pd_index); 
  set_mv1c(buffer, d3pd_index); 

  m_truth_flavor = buffer->truth_flavor->at(d3pd_index); 
}

double Jet::anti_b(Tagger tagger) const { 
  assert(m_anti_b.count(tagger) == 1); 
  return m_anti_b.find(tagger)->second; 
}
double Jet::anti_u(Tagger tagger) const { 
  assert(m_anti_u.count(tagger) == 1); 
  return m_anti_u.find(tagger)->second; 
}
int Jet::truth_flavor() const { 
  return m_truth_flavor; 
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

void Jet::set_mv1c(const JetCollectionBuffer* buffer, int index) { 
  double ip3d = buffer->ip3d->at(index); 
  double sv1 = buffer->sv1->at(index); 
  double pu = buffer->cnn_pu->at(index); 
  double pc = buffer->cnn_pc->at(index); 
  double pb = buffer->cnn_pb->at(index); 

  double pt = buffer->pt->at(index); 
  double eta = buffer->eta->at(index); 

  double mv1c = mv1cEval(ip3d, sv1, pu, pc, pb, pt, eta); 

  m_anti_u[MV1C] = mv1c; 
  m_anti_b[MV1C] = -mv1c; 
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
  
  set_branch("jet_" + collection + "_n", &b.n); 
  set_branch("jet_" + collection + "_pt", &b.pt); 
  set_branch("jet_" + collection + "_eta", &b.eta);  
  set_branch("jet_" + collection + "_phi", &b.phi); 
  set_branch("jet_" + collection + "_E", &b.e); 

  set_branch("jet_" + collection + "_flavor_truth_label", &b.truth_flavor); 

  std::string jfc_comp = "_flavor_component_jfitc"; 
  set_branch("jet_" + collection + jfc_comp + "_pu", &b.jfc_pu); 
  set_branch("jet_" + collection + jfc_comp + "_pc", &b.jfc_pc); 
  set_branch("jet_" + collection + jfc_comp + "_pb", &b.jfc_pb); 

  std::string cnn_comp = "_flavor_component_jfitcomb"; 
  set_branch("jet_" + collection + cnn_comp + "_pu", &b.cnn_pu); 
  set_branch("jet_" + collection + cnn_comp + "_pc", &b.cnn_pc); 
  set_branch("jet_" + collection + cnn_comp + "_pb", &b.cnn_pb); 

  set_branch("jet_" + collection + "_flavor_weight_MV1", &b.mv1); 

  set_branch("jet_" + collection + "_flavor_weight_SV1", &b.sv1); 
  set_branch("jet_" + collection + "_flavor_weight_IP3D", &b.ip3d); 
    
}

std::vector<Jet> JetFactory::jets(std::string collection) const { 
  if (m_buffers.count(collection) == 0) { 
    throw std::runtime_error("jet collection " + collection + 
			     " has not been set"); 
  }
  const JetCollectionBuffer* d3pd_jets = m_buffers.find(collection)->second; 
  
  std::vector<Jet> jets; 
  for (int jet_n = 0; jet_n < d3pd_jets->n; jet_n++) { 
    jets.push_back(Jet(d3pd_jets, jet_n)); 
  }
  return jets; 
}

void JetFactory::set_branch_without_zeroing(std::string name, void* branch) { 
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
