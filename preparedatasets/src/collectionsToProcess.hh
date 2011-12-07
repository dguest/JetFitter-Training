#include <vector>
#include <TString.h>
#include <iostream>

using namespace std;

vector<TString> getCollectionsToProcess() 
{
  
  vector<TString> collectionsToProcess;
  
//REALLY CALIBRATED
  collectionsToProcess.push_back("AntiKt4TopoEMJets");
//  collectionsToProcess.push_back("AntiKt6TopoEMJets");
//  collectionsToProcess.push_back("AntiKt4LCTopoJets");
//  collectionsToProcess.push_back("AntiKt6LCTopoJets");

//ALL OTHERS WHICH ARE NOT CALIBRATED

  return collectionsToProcess;
  
}


vector<TString> getCollectionsToPutIntoFile() 
{
    vector<TString> collectionsToProcess;
//  collectionsToProcess.push_back("AntiKt4TowerJets");
//  collectionsToProcess.push_back("AntiKt6TowerJets");
  collectionsToProcess.push_back("AntiKt6TopoEMJets");
  collectionsToProcess.push_back("AntiKt4TopoEMJets");
  collectionsToProcess.push_back("AntiKt4LCTopoJets");
  collectionsToProcess.push_back("AntiKt6LCTopoJets");
  return collectionsToProcess;
}

TString renameForCalibration(TString input) {

//rename
  if (input=="AntiKt6TopoEMJets") return TString("AntiKt4TopoEMJets");
  if (input=="AntiKt4LCTopoJets") return TString("AntiKt4TopoEMJets");
  if (input=="AntiKt6LCTopoJets") return TString("AntiKt4TopoEMJets");

  return input;
//if (input=="AntiKt4H1TopoEMJets") return TString("AntiKt4TopoEMJets");
//if (input=="AntiKt6H1TopoEMJets") return TString("AntiKt6TopoEMJets");
  
}


vector<TString> getAllCollections() 
{
  
  vector<TString> collectionsToProcess;
  
//  collectionsToProcess.push_back("AntiKt4LCTopoJetsAOD");
//  collectionsToProcess.push_back("AntiKt6LCTopoJetsAOD");
//  collectionsToProcess.push_back("AntiKt4TruthJetsAOD");
//  collectionsToProcess.push_back("AntiKt6TruthJetsAOD");

  collectionsToProcess.push_back("AntiKt4TopoEMJetsAOD");
//  collectionsToProcess.push_back("AntiKt4TopoJetsAOD");
//  collectionsToProcess.push_back("AntiKt6TopoEMJetsAOD");
//  collectionsToProcess.push_back("AntiKt6TopoJetsAOD");

  return collectionsToProcess;
  
}
