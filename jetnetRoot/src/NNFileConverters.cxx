#include "NNFileConverters.hh"
#include "NetworkToHistoTool.hh"
#include "TFlavorNetwork.h"

#include <string> 
#include <map> 
#include <stdexcept> 

#include "TH1.h"
#include "TFile.h"

void nn_file_to_hist_file(std::string nn_file, 
			  std::string out_file, 
			  std::string nn_name) { 

  TFile in_file(nn_file.c_str()); 
  if (in_file.IsZombie()) { 
    throw std::runtime_error("can't open " + nn_file); 
  }
  TFlavorNetwork* net = dynamic_cast<TFlavorNetwork*>
    (in_file.Get(nn_name.c_str())); 
  if (!net) { 
    throw std::runtime_error("no " + nn_name + " in " + nn_file); 
  }

  TFile out(out_file.c_str(), "recreate"); 

  NetworkToHistoTool tool; 
  std::map<std::string,TH1*> hists = tool.histsFromNetwork(net); 
  for (std::map<std::string,TH1*>::iterator itr = hists.begin(); 
       itr != hists.end(); itr++) { 
    out.WriteTObject(itr->second); 
  }
  
  
}
