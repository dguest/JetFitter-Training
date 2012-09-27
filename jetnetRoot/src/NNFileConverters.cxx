#include "NNFileConverters.hh"
#include "NetworkToHistoTool.hh"
#include "TNeuralNetwork.h"

#include <string> 
#include <map> 
#include <stdexcept> 
#include <cassert>
#include <boost/format.hpp>

#include "TH1.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"

void hist_from_nn(std::string nn_file, 
		  std::string out_file, 
		  std::string nn_name) { 

  TFile in_file(nn_file.c_str()); 
  if (in_file.IsZombie()) { 
    throw std::runtime_error("can't open " + nn_file); 
  }
  TNeuralNetwork* net = dynamic_cast<TNeuralNetwork*>
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
  copy_hists(in_file, out); 
}

void nn_from_hist(std::string hist_file, 
		  std::string nn_file, 
		  std::string nn_name) { 

  TFile in_file(hist_file.c_str()); 
  if (in_file.IsZombie()) { 
    throw std::runtime_error("can't open " + hist_file); 
  }

  std::map<std::string, TH1*> hists; 

  hists["LayersInfo"] = dynamic_cast<TH1*>(in_file.Get("LayersInfo")); 
  if (!hists["LayersInfo"]) { 
    throw std::runtime_error("can't find LayersInfo in " + hist_file); 
  }
  hists["InputsInfo"] = dynamic_cast<TH1*>(in_file.Get("InputsInfo")); 
  if (!hists["InputsInfo"]) { 
    throw std::runtime_error("can't find InputsInfo in " + hist_file); 
  }
  int n_layers_with_wt = hists["LayersInfo"]->GetNbinsX() - 1; 
  for (int i = 0; i < n_layers_with_wt; i++) { 
    std::string wt_name = (boost::format("Layer%i_weights") % i).str();
    std::string th_name = (boost::format("Layer%i_thresholds") % i).str();
    hists[wt_name] = dynamic_cast<TH1*>(in_file.Get(wt_name.c_str())); 
    hists[th_name] = dynamic_cast<TH1*>(in_file.Get(th_name.c_str())); 
    if (!hists[wt_name] || !hists[th_name]) { 
      throw std::runtime_error("missing thresholds or wt in " + hist_file); 
    }
  }
  
  std::set<std::string> exclude; 
  for (std::map<std::string, TH1*>::const_iterator itr = hists.begin(); 
       itr != hists.end(); 
       itr++) { 
    exclude.insert(itr->first); 
  }

  TFile out(nn_file.c_str(), "recreate"); 

  NetworkToHistoTool tool; 
  TNeuralNetwork* nn = tool.networkFromHists(hists); 
  out.WriteTObject(nn); 

  copy_hists(in_file, out, exclude); 

}

namespace { 
  void copy_hists(TFile& in_file, TFile& out, std::set<std::string> exc) { 
    TList* key_list = in_file.GetListOfKeys(); 
    int n_keys = key_list->GetEntries(); 
    for (int n = 0; n < n_keys; n++) { 
      TKey* key = dynamic_cast<TKey*>(key_list->At(n)); 
      assert(key); 
      if (exc.count(key->GetName())) { 
	continue; 
      }
      TObject* the_object = key->ReadObj(); 
      if (the_object->InheritsFrom("TH1")) { 
	out.WriteTObject(the_object); 
      }
    }
  
  }
}
