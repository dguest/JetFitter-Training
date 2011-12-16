#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include "TString.h"

void trainNN_no_root(const char* inputfile,
				const char* outputclass,
				int nIterations,
				int dilutionFactor,
				bool useSD,
				bool withIP3D,
				int nodesFirstLayer,
				int nodesSecondLayer,
				int restartTrainingFrom);


void trainNN(TString inputfile,
             TString outputclass="JetFitterNN",
             int nIterations=10,
             int dilutionFactor=2,
             bool useSD=false,
             bool withIP3D=true,
             int nodesFirstLayer=10,
             int nodesSecondLayer=9,
             int restartTrainingFrom=0);

#endif // TRAIN_NN_H
