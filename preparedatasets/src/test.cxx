#include "PtEtaCategoryTool.hh"
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <TH1D.h>
#include <TCanvas.h>

int main (int narg, char* argv[]){ 
  
  std::vector<float> values; 


  values.push_back(-4); 
  values.push_back(-3); 
  values.push_back(-2); 
  values.push_back(-1); 
  values.push_back(0); 
  values.push_back(1); 
  values.push_back(2); 
  values.push_back(3); 
  values.push_back(4); 
  

  CategoryMap cat_map(values); 
  
  std::cout << cat_map.get_category(1e-4) << std::endl; 
  std::cout << cat_map.get_category(1e-5) << std::endl; 
  std::cout << cat_map.get_category(1e-6) << std::endl; 
  std::cout << cat_map.get_category(1e-7) << std::endl; 
  std::cout << cat_map.get_category(0) << std::endl; 
  std::cout << cat_map.get_category(-1e-7) << std::endl; 
  std::cout << cat_map.get_category(-1e-6) << std::endl; 
  std::cout << cat_map.get_category(-1e-5) << std::endl; 
  std::cout << cat_map.get_category(-1e-4) << std::endl; 

  for (CategoryMap::const_iterator itr = cat_map.begin(); 
       itr != cat_map.end(); 
       itr++){ 
    std::cout << itr->first << "\t " << itr->second << std::endl; 
  }


  srand(time(0)); 
  TH1F* hist = new TH1F("c","", 10, 0,10); 
  
  for (unsigned i = 0; i < 100000000; i++){ 
    float value = (float(rand()) / float(RAND_MAX) - 0.5 ) * 10; 
    int cat = cat_map.get_category(value); 

    hist->Fill(cat); 

    assert(cat >= 0); 
  }

  TCanvas can("can","",100,100,600,400); 
  hist->Draw(); 
  can.Print("h.pdf"); 

  return 0; 
  
}
