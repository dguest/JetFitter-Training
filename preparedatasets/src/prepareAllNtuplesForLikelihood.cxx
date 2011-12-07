{
gROOT->ProcessLine(".L readJFBTagAna.C+");
gROOT->ProcessLine(".L readBaseBTagAnaTree.C+");
gROOT->ProcessLine(".L writeNtuple_Official.cxx+");
gROOT->ProcessLine(".L writeNtupleAll.C+");

writeAllNtuples("../datasets/all.root",false);

}
