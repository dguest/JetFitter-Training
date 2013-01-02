#include "PerfNtupleBuilder.hh"
#include <stdexcept> 
#include <boost/format.hpp>


#include "TFile.h"
#include "TTree.h"

PerfNtupleBuilder::PerfNtupleBuilder(std::string out_file_name, 
				     std::string out_tree_name): 
  m_out_file(0), 
  m_out_tree(0)
{
  if (out_tree_name.size() != 0) { 
    m_out_file = new TFile(out_file_name.c_str(), "recreate"); 
    m_out_tree = new TTree(out_tree_name.c_str(), out_tree_name.c_str()); 
    init_tree(); 
  }
}

PerfNtupleBuilder::~PerfNtupleBuilder() { 
  m_out_file->WriteTObject(m_out_tree); 
  delete m_out_file; 
}

void PerfNtupleBuilder::write_entry() { 
  set_flavor_branches(flavor); 
  if (m_out_tree) { 
    m_out_tree->Fill(); 
  }
}

void PerfNtupleBuilder::init_tree() { 
  m_out_tree->Branch("logCuJetFitterCharm", &jfc_anti_u); 
  m_out_tree->Branch("logCbJetFitterCharm", &jfc_anti_b); 

  m_out_tree->Branch("logCuJetFitterCOMBNN", &cnn_anti_u); 
  m_out_tree->Branch("logCbJetFitterCOMBNN", &cnn_anti_b); 

  m_out_tree->Branch("antiUMv1", &mv1_anti_u); 
  m_out_tree->Branch("antiBMv1", &mv1_anti_b); 

  m_out_tree->Branch("JetPt", &pt); 
  m_out_tree->Branch("JetEta", &eta); 
  m_out_tree->Branch("Flavour", &flavor); 

  m_out_tree->Branch("bottom", &m_bottom); 
  m_out_tree->Branch("charm", &m_charm); 
  m_out_tree->Branch("light", &m_light); 
  
}

void PerfNtupleBuilder::set_flavor_branches(int flavor_code) { 
  switch (flavor_code) { 
  case 0: 
    m_bottom = 0; 
    m_charm = 0; 
    m_light = 1; 
    return; 
  case 4: 
    m_bottom = 0; 
    m_charm = 1; 
    m_light = 0; 
    return; 
  case 5: 
    m_bottom = 1; 
    m_charm = 0; 
    m_light = 0; 
    return; 
  default: 
    m_bottom = 0; 
    m_charm = 0; 
    m_light = 0; 
  }
}
