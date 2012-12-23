#include "SmartChain.hh"
#include "TChain.h"
#include <boost/format.hpp>

SmartChain::SmartChain(std::string tree_name): 
  TChain(tree_name.c_str())
{ 
}

std::vector<std::string> SmartChain::get_all_branch_names() const { 
  return m_set_branches; 
}

void SmartChain::SetBranchAddressPrivate(std::string name, void* branch, 
					 bool turnon) { 

  if (turnon) { 
    SetBranchStatus(name.c_str(), 1); 
  }
  int return_code = TChain::SetBranchAddress(name.c_str(), branch); 
  if (return_code != 0 && return_code != 5 ){ 
    std::string issue = (boost::format("can not set %s , return code %i") % 
			 name % return_code).str(); 
    throw std::runtime_error(issue); 
  }
  if (!GetBranch(name.c_str())) { 
    std::string issue = (boost::format("can't find branch %s") % name).str(); 
    throw std::runtime_error(issue); 
  }

  m_set_branches.push_back(name);

  if (m_set_branch_set.count(name)) { 
    std::string err = (boost::format("setting branch address: %s is already"
				     " set in %s") % name % GetName()).str(); 
    throw std::runtime_error(err); 
  }
  m_set_branch_set.insert(name); 
}
