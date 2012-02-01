
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "testJetNet.hh"
#include <string>
#include <iostream>

int main(int argn, char* argv[])
{
  std::string what_is_it =
    "usage: <weights file> [options]\n";

  namespace po = boost::program_options;
  std::string weight_file = ""; 
  std::string jet_collection = ""; 

  // Declare the supported options.
  po::options_description desc("options");
  desc.add_options()
    ("collection"  ,
     po::value<std::string>(&jet_collection)->
     default_value("AntiKt4TopoEMJets"), 
     "name of jet collection")
    ("weights", 
     po::value<std::string>(&weight_file), 
     "input weights file")
    ("help,h"    ,"get help")
    ;

  po::positional_options_description p;
  p.add("weights", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argn, argv).
	    options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << what_is_it << std::endl;
    std::cout << desc << "\n";
    return 1;
  }


  testJetNet(weight_file.c_str(),jet_collection.c_str());
//2970
}
