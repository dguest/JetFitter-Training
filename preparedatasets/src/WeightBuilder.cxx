#include "WeightBuilder.hh"

#include <string> 
#include <map> 
#include <cstdlib> // rand
#include <boost/lexical_cast.hpp>
#include <stdexcept> 

#include "TFile.h"
#include "TH2D.h"
#include "TROOT.h"

WeightBuilder::WeightBuilder(const std::map<int,std::string>& f2b, 
			     std::string file_name, 
			     std::string hist_base_name){ 
  TFile file(file_name.c_str());

  // couple this with clone and rand and you get reasonable root behavior
  gROOT->cd(); 			
  
  TH2D* base_hist = dynamic_cast<TH2D*> ( file.Get(hist_base_name.c_str())); 
  if (!base_hist) { 
    throw std::runtime_error("could not find " + hist_base_name); 
  }

  for (std::map<int,std::string>::const_iterator 
	 itr = f2b.begin(); itr != f2b.end(); itr++){ 
    
    std::string hist_name = hist_base_name + "_" + itr->second; 
    std::string rand_name = boost::lexical_cast<std::string>(rand()); 
    TH2D* loc_hist = dynamic_cast<TH2D*> 
      (file.Get(hist_name.c_str())->Clone(rand_name.c_str())); 
    if (!loc_hist) throw std::runtime_error("can't find " + hist_name); 
    
    loc_hist->Divide(base_hist); 
    
    m_weight_hists[itr->first] = loc_hist; 
  }
}

WeightBuilder::~WeightBuilder() { 
  for (std::map<int,TH2D*>::iterator itr = m_weight_hists.begin(); 
       itr != m_weight_hists.end(); 
       itr++) { 
    delete itr->second; 
    itr->second = 0; 
  }
}

double WeightBuilder::lookup(int flavor, double x, double y) const { 
  std::map<int,TH2D*>::const_iterator the_hist = m_weight_hists.find(flavor); 
  if (the_hist == m_weight_hists.end() ) { 
    throw std::domain_error
      ("could not find " + boost::lexical_cast<std::string>(flavor) 
       + " in lookup hists"); 
  }
  TH2D* hist = the_hist->second; 
  int the_bin = hist->FindBin(x, y); 

  int bin_x = hist->GetXaxis()->FindBin(x); 
  if (bin_x == 0 || bin_x > hist->GetXaxis()->GetNbins() ) { 
    return 0; 
  }
  int bin_y = hist->GetYaxis()->FindBin(y); 
  if (bin_y == 0 || bin_y > hist->GetYaxis()->GetNbins() ) { 
    return 0; 
  }

  return hist->GetBinContent(the_bin); 
}
