#include "TreeTranslator.hh"
#include "JetFactory.hh"
#include "PerfNtupleBuilder.hh"

#include <string> 
#include <vector> 
#include <stdexcept> 
#include <iostream>

#include <boost/format.hpp>

#include "TFile.h"
#include "TChain.h"
#include "TTree.h"

TreeTranslator::TreeTranslator(std::string in_chain_name, 
			       std::string out_file_name, 
			       std::string out_tree): 
  m_collection(""), 
  m_in_chain_name(in_chain_name), 
  m_ntuple_builder(0), 
  m_factory(0), 
  m_in_chain(0)
{ 
  if (out_file_name.size() != 0) { 
    m_ntuple_builder = new PerfNtupleBuilder(out_file_name.c_str(), 
					     out_tree); 
  }
}

TreeTranslator::~TreeTranslator() { 
  delete m_in_chain; 
  delete m_ntuple_builder; 
  delete m_factory; 
}

void TreeTranslator::add_collection(std::string collection) { 
  if (m_collection.size()) { 
    throw std::logic_error("tried to double set collection"); 
  }
  m_collection = collection; 
}

void TreeTranslator::translate(std::vector<std::string> d3pd) { 
  init_chain(d3pd); 
  m_factory = new JetFactory(m_in_chain); 
  m_factory->add_collection(m_collection); 

  int n_entries = m_in_chain->GetEntries(); 
  int one_percent = n_entries /  100; 
  n_entries = std::min(n_entries, 10); 

  for (int evt_n = 0; evt_n < n_entries; evt_n++) { 
    if (evt_n % one_percent == 0 || evt_n == n_entries - 1 ) { 
      std::cout << boost::format("\r%i of %i (%.1f%%) processed") % 
    	(evt_n + 1) % n_entries % ( (evt_n + 1) / one_percent); 
      std::cout.flush(); 
    }

    std::cout << "here is evt " << evt_n << std::endl; 

    m_in_chain->GetEntry(evt_n); 
    
    std::vector<Jet> jets = m_factory->jets(m_collection); 
    for (std::vector<Jet>::const_iterator itr = jets.begin(); 
    	 itr != jets.end(); itr++) { 
      m_ntuple_builder->pt = itr->Pt(); 
      m_ntuple_builder->eta = itr->Eta(); 
      m_ntuple_builder->flavor = itr->truth_flavor(); 

      copy_taggers(*itr, m_ntuple_builder); 
      m_ntuple_builder->write_entry(); 
    }

  }

}

void TreeTranslator::copy_taggers(const Jet& jet, 
				  PerfNtupleBuilder* builder) const { 
  builder->jfc_anti_b = jet.anti_b(JFC); 
  builder->jfc_anti_u = jet.anti_u(JFC); 

  builder->cnn_anti_b = jet.anti_b(COMBNN); 
  builder->cnn_anti_u = jet.anti_u(COMBNN); 

  builder->mv1_anti_b = jet.anti_b(MV1); 
  builder->mv1_anti_u = jet.anti_u(MV1); 
}

void TreeTranslator::init_chain(std::vector<std::string> d3pds) { 
  assert(!m_in_chain); 
  m_in_chain = new TChain(m_in_chain_name.c_str()); 
  for (std::vector<std::string>::const_iterator itr = d3pds.begin(); 
       itr != d3pds.end(); itr++) { 
    int ret_code = m_in_chain->Add(itr->c_str(), -1); 
    if (ret_code != 1) { 
      std::string err = (boost::format("bad file: %s") % *itr).str(); 
      throw std::runtime_error(err); 
    }
  }
}
