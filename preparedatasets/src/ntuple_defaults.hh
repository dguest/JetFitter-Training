#ifndef NTUPLE_DEFAULTS_H
#define NTUPLE_DEFAULTS_H

namespace magic { 
  const float min_jet_pt_gev = 15.0; 
  const float max_jet_eta = 2.5; 
}

namespace defopt { 
  const std::string JCN = "AntiKt4TopoEMJets"; 
  const std::string OFN = "../reduceddatasets/"
    "reduceddataset_" + JCN + "_forNN.root"; 
  // const double PT_CATEGORIES[]  = {15.0, 30.0, 50.0, 80.0, 120.0}; 
  const double PT_CATEGORIES2[] = {25.0, 35.0, 50.0, 80.0, 120.0, 200.0}; 
}


#endif // NTUPLE_DEFAULTS_H
