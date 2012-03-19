#ifndef NTUPLE_DEFAULTS_H
#define NTUPLE_DEFAULTS_H

namespace magic { 
  const float min_jet_pt_gev = 15.0; 
}

namespace defopt { 
  const std::string JCN = "AntiKt4TopoEMJets"; 
  const std::string OFN = "../reduceddatasets/"
    "reduceddataset_" + JCN + "_forNN.root"; 
}


#endif // NTUPLE_DEFAULTS_H
