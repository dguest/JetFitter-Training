{
gSystem->Load("../jetnetRoot/libTJetNet.so");
gSystem->Load("readReducedDataset_C.so");
gSystem->Load("testJetNet_cxx.so");
//testJetNet("../../JetCalibUtil/jetnetRoot/ATLAS/weights_withIP3D/weightMinimum.root","AntiKt4Jets");
testJetNet("/afs/physik.uni-freiburg.de/home/giacinto/recon11/JetCalibUtil_AutumnReprocessing2010/trainingResultsJetNet/AntiKt4TopoEMJets/comb/weights/weightMinimum.root","AntiKt4TopoEMJets");
//2970
}
