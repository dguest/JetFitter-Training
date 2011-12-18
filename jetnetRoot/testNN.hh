#ifndef TEST_NN_H
#define TEST_NN_H

#include <string>


void testNN(std::string inputfile,
	    std::string training_file = "weights/weightMinimum.root",
	    int dilutionFactor = 2,
	    // int nodesFirstLayer = 10, 
	    // int nodesSecondLayer = 9, 
	    bool useSD = false,
	    bool withIP3D = true);


class NetworkLoadException
{};

class WrongNetworkSizeException: public NetworkLoadException 
{}; 

#endif // TEST_NN_H
