#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include "TString.h"
#include <string>

// int nothing();


void trainNN(TString inputfile,
             TString outputclass="JetFitterNN",
	     std::string out_dir = "weights", 
             int nIterations=10,
             int dilutionFactor=2,
             bool useSD=false,
             bool withIP3D=true,
             int nodesFirstLayer=10,
             int nodesSecondLayer=9,
             int restartTrainingFrom=0);

#endif // TRAIN_NN_H
