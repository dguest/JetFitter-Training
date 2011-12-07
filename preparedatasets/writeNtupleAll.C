#include "writeNtuple_Official.h"
#include "collectionsToProcess.h"
#include <TString.h>
#include <iostream>

using namespace std;


void writeAllNtuples (TString inputfilename,
                      TString inputfilename2,
                      TString inputfilename3,
                      TString inputfilename4,
                      bool forNN=false) 
{
  vector<TString> collectionsToProcess=getCollectionsToProcess();

  vector<TString>::const_iterator collectionsToProcessBegin=collectionsToProcess.begin();
  vector<TString>::const_iterator collectionsToProcessEnd=collectionsToProcess.end();
  
  for (vector<TString>::const_iterator collectionsToProcessIter=collectionsToProcessBegin;
       collectionsToProcessIter!=collectionsToProcessEnd;
       ++collectionsToProcessIter) 
  {
    
    cout << " Writing ALL Ntuples: processing " << *collectionsToProcessIter << endl;
    writeNtuple_Official(inputfilename,
                         inputfilename2,
                         inputfilename3,
                         inputfilename4,
                         *collectionsToProcessIter,
                         "AOD",
                         forNN,
                         false);
  }

  
}



        
        
