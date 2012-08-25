#include "NNFileConverters.hh"
#include "NetworkToHistoTool.hh"
#include "TFlavorNetwork.h"

#include <string> 
#include <map> 
#include <stdexcept> 
#include <cassert>

#include "TH1.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"

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

  // also copy over any remaining histograms
  TList* key_list = in_file.GetListOfKeys(); 
  int n_keys = key_list->GetEntries(); 
  for (int n = 0; n < n_keys; n++) { 
    TKey* key = dynamic_cast<TKey*>(key_list->At(n)); 
    assert(key); 
    TObject* the_object = key->ReadObj(); 
    if (the_object->InheritsFrom("TH1")) { 
      out.WriteTObject(the_object); 
    }
  }
  
}
