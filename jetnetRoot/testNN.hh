#ifndef TEST_NN_H
#define TEST_NN_H

#include <string>


void testNN(std::string inputfile,
	    std::string training_file = "weights/weightMinimum.root",
	    int dilutionFactor = 2,
	    bool useSD = false,
	    bool withIP3D = true);


class NetworkLoadException
{};

#endif // TEST_NN_H
