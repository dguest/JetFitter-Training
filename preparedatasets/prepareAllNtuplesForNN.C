{
  gROOT->ProcessLine(".L readJFBTagAna.C+");
  gROOT->ProcessLine(".L readBaseBTagAnaTree.C+");
  gROOT->ProcessLine(".L writeNtuple_Official.cxx+");
  gROOT->ProcessLine(".L writeNtupleAll.C+");

//  writeAllNtuples("root://castoratlas///castor/cern.ch/user/g/giacinto/WH_ATLAS/WHbb/*BTagAna.root",
//                  "root://castoratlas///castor/cern.ch/user/g/giacinto/WH_ATLAS/WHcc/*BTagAna.root",
//                  "root://castoratlas///castor/cern.ch/user/g/giacinto/WH_ATLAS/WHuu/*BTagAna.root",
//                  "",
//                  true);


  writeAllNtuples("/tmp/giacinto/r2722/*.root",
                  "",
                  "",
                  "",
                  true);


}
