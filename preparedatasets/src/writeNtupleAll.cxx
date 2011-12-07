#include "writeNtuple_Official.hh"
#include "collectionsToProcess.hh"
#include <string>
#include <iostream>

using namespace std;


void writeAllNtuples (std::string inputfilename,
                      std::string inputfilename2,
                      std::string inputfilename3,
                      std::string inputfilename4,
                      bool forNN) 
{
  vector<TString> collectionsToProcess=getCollectionsToProcess();

  vector<TString>::const_iterator collectionsToProcessBegin=collectionsToProcess.begin();
  vector<TString>::const_iterator collectionsToProcessEnd=collectionsToProcess.end();
  
  for (vector<TString>::const_iterator collectionsToProcessIter=collectionsToProcessBegin;
       collectionsToProcessIter!=collectionsToProcessEnd;
       ++collectionsToProcessIter) 
  {
    
    cout << " Writing ALL Ntuples: processing " << *collectionsToProcessIter << endl;
    writeNtuple_Official(TString(inputfilename.c_str()),
                         TString(inputfilename2.c_str()),
                         TString(inputfilename3.c_str()),
                         TString(inputfilename4.c_str()),
                         *collectionsToProcessIter,
                         "AOD",
                         forNN,
                         false);
  }

  
}



        
        
