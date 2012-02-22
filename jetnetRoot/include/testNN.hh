#ifndef TEST_NN_H
#define TEST_NN_H

#include <string>


void testNN(std::string inputfile,
	    std::string training_file = "weights/weightMinimum.root",
	    int dilutionFactor = 2,
	    // int nodesFirstLayer = 10, 
	    // int nodesSecondLayer = 9, 
	    bool useSD = false,
	    bool withIP3D = true, 
	    std::string out_file = "all_hists.root", 
	    bool debug = true);



enum Sample { 
  TRAIN, 
  TEST
}; 
enum Flavor { 
  LIGHT, 
  CHARM, 
  BOTTOM
}; 


std::string flavor_to_string(Flavor); 
std::string sample_to_string(Sample); 

#endif // TEST_NN_H
