//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sat Feb 16 12:27:32 2008 by ROOT version 5.18/00
// from TTree SVTree/SVTree
// found on file: /afs/physik.uni-freiburg.de/home/giacinto/recon3/officialCalibrationJetFitter2/reduceddatasets/reduceddataset_Cone4H1TopoParticleJets.root
//////////////////////////////////////////////////////////

#ifndef readReducedDataset_h
#define readReducedDataset_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class readReducedDataset {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           nVTX;
   Int_t           nTracksAtVtx;
   Int_t           nSingleTracks;
   Double_t        energyFraction;
   Double_t        mass;
   Double_t        significance3d;
   Int_t           cat_pT;
   Int_t           cat_eta;
   Double_t        weight;
   Double_t        discriminatorIP3D;
   Int_t           cat_flavour;
   Double_t        deltaR;
   Double_t        JetPt;
   Double_t        JetEta;
   Double_t        discriminatorIP2D;
   Double_t        discriminatorSV1;
   Double_t        discriminatorCOMB;

   // List of branches
   TBranch        *b_nVTX;   //!
   TBranch        *b_nTracksAtVtx;   //!
   TBranch        *b_nSingleTracks;   //!
   TBranch        *b_energyFraction;   //!
   TBranch        *b_mass;   //!
   TBranch        *b_significance3d;   //!
   TBranch        *b_cat_pT;   //!
   TBranch        *b_cat_eta;   //!
   TBranch        *b_weight;   //!
   TBranch        *b_discriminatorIP3D;   //!
   TBranch        *b_cat_flavour;   //!
   TBranch        *b_deltaR;   //!
   TBranch        *b_JetPt;   //!
   TBranch        *b_JetEta;   //!
   TBranch        *b_discriminatorIP2D;   //!
   TBranch        *b_discriminatorSV1;   //!
   TBranch        *b_discriminatorCOMB;   //!

   readReducedDataset(TTree *tree=0);
   virtual ~readReducedDataset();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef readReducedDataset_cxx
readReducedDataset::readReducedDataset(TTree *tree)
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/afs/physik.uni-freiburg.de/home/giacinto/recon3/officialCalibrationJetFitter2/reduceddatasets/reduceddataset_Cone4H1TopoParticleJets.root");
      if (!f) {
         f = new TFile("/afs/physik.uni-freiburg.de/home/giacinto/recon3/officialCalibrationJetFitter2/reduceddatasets/reduceddataset_Cone4H1TopoParticleJets.root");
      }
      tree = (TTree*)gDirectory->Get("SVTree");

   }
   Init(tree);
}

readReducedDataset::~readReducedDataset()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t readReducedDataset::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t readReducedDataset::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (!fChain->InheritsFrom(TChain::Class()))  return centry;
   TChain *chain = (TChain*)fChain;
   if (chain->GetTreeNumber() != fCurrent) {
      fCurrent = chain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void readReducedDataset::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("nVTX", &nVTX, &b_nVTX);
   fChain->SetBranchAddress("nTracksAtVtx", &nTracksAtVtx, &b_nTracksAtVtx);
   fChain->SetBranchAddress("nSingleTracks", &nSingleTracks, &b_nSingleTracks);
   fChain->SetBranchAddress("energyFraction", &energyFraction, &b_energyFraction);
   fChain->SetBranchAddress("mass", &mass, &b_mass);
   fChain->SetBranchAddress("significance3d", &significance3d, &b_significance3d);
   fChain->SetBranchAddress("cat_pT", &cat_pT, &b_cat_pT);
   fChain->SetBranchAddress("cat_eta", &cat_eta, &b_cat_eta);
   fChain->SetBranchAddress("weight", &weight, &b_weight);
   fChain->SetBranchAddress("discriminatorIP3D", &discriminatorIP3D, &b_discriminatorIP3D);
   fChain->SetBranchAddress("cat_flavour", &cat_flavour, &b_cat_flavour);
   fChain->SetBranchAddress("deltaR", &deltaR, &b_deltaR);
   fChain->SetBranchAddress("JetPt", &JetPt, &b_JetPt);
   fChain->SetBranchAddress("JetEta", &JetEta, &b_JetEta);
   fChain->SetBranchAddress("discriminatorIP2D", &discriminatorIP2D, &b_discriminatorIP2D);
   fChain->SetBranchAddress("discriminatorSV1", &discriminatorSV1, &b_discriminatorSV1);
   fChain->SetBranchAddress("discriminatorCOMB", &discriminatorCOMB, &b_discriminatorCOMB);
   Notify();
}

Bool_t readReducedDataset::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void readReducedDataset::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t readReducedDataset::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef readReducedDataset_cxx
