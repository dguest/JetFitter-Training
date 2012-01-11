// interface for ds preparation script
%module prepntuple
%{
#include "writeNtupleAll.hh"
%}

void writeAllNtuples (std::string inputfilename,
                      std::string inputfilename2 = "",
                      std::string inputfilename3 = "",
                      std::string inputfilename4 = "",
                      bool forNN = true); 
