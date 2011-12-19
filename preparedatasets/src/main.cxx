#include "writeNtupleAll.hh"

#include <string>

int main (int narg, char* argv[]) { 

  writeAllNtuples("files/*.root*",
                  "",
                  "",
                  "",
                  true);


}
