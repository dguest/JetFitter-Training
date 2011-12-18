#include <TTree.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <iostream>
#include <TPad.h>
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <math.h>
#include "TJetNet.h"
#include "doNormalization.hh"
#include "Riostream.h"
#include "TNetworkToHistoTool.h"

#include "TTrainedNetwork.h"
#include "testNN.hh"

#include <iostream>

#include "TMatrixD.h"
#include "TVectorD.h"


void testNN(std::string inputfile,
	    std::string training_file,
	    int dilutionFactor,
	    bool useSD,
	    bool withIP3D) {

  double bweight=1;
  double cweight=1.;
  double lweight=5;

  gROOT->SetStyle("Plain");

  std::cout << "starting with settings: " << std::endl;
  std::cout << " dilutionFactor: " << dilutionFactor << std::endl;
  std::cout << " useSD: " << (useSD==true?"yes":"no") << std::endl;
  std::cout << " withIP3D: " << (withIP3D==true?"yes":"no") << std::endl;
  
  
  TFile *file= new TFile(inputfile.c_str());
  TTree *simu = (TTree*)file->Get("SVTree");

  Int_t           nVTX;
  Int_t           nTracksAtVtx;
  Int_t           nSingleTracks;
  Double_t        energyFraction;
  Double_t        mass;
  Double_t        significance3d;
  Double_t        discriminatorIP3D;
  Int_t        cat_pT;
  Int_t        cat_eta;
  Double_t        weight;
  Int_t bottom;
  Int_t charm;
  Int_t light;
  
  simu->SetBranchAddress("nVTX",&nVTX);
  simu->SetBranchAddress("nTracksAtVtx",&nTracksAtVtx);
  simu->SetBranchAddress("nSingleTracks",&nSingleTracks);
  simu->SetBranchAddress("energyFraction",&energyFraction);
  simu->SetBranchAddress("mass",&mass);
  simu->SetBranchAddress("significance3d",&significance3d);
  simu->SetBranchAddress("discriminatorIP3D",&discriminatorIP3D);
  simu->SetBranchAddress("cat_pT",&cat_pT);
  simu->SetBranchAddress("cat_eta",&cat_eta);
  simu->SetBranchAddress("weight",&weight);
  simu->SetBranchAddress("bottom",   &bottom);
  simu->SetBranchAddress("charm",   &charm);
  simu->SetBranchAddress("light",&light);


  // === above was from the top of the train routine
  TFile trained_network_file(training_file.c_str()); 
  
  TObject* trained_network_obj = trained_network_file.Get("TTrainedNetwork");
  TTrainedNetwork* trained_network = 
    dynamic_cast<TTrainedNetwork*>(trained_network_obj); 
  if (trained_network == 0){ 
    throw NetworkLoadException(); 
  }
  
  // TJetNet* jn = new TJetNet( numberTestingEvents, 
  // 			     numberTrainingEvents, 
  // 			     nlayer, 
  // 			     nneurons );
  TJetNet* jn = new TJetNet();
  jn->readBackTrainedNetwork(trained_network); 

  // === below was from the end of the train routine 

  //here you should create the class... Still open how to deal with this...
  //  char* myname=const_cast<char*>(static_cast<const char*>(outputclass));
  //  ierr=mlpsavecf_(myname);
 

 
  TCanvas* mlpa_canvas = new TCanvas("jetnet_canvas","Network analysis");
  mlpa_canvas->Divide(2,4);


  
  //  TCanvas* mlpa_canvas_5=gDirectory->Get("mlpa_canvas_5");
  //  mlpa_canvas_5->SetLogy(kTrue);
  gPad->SetLogy();

  // Use the NN to plot the results for each sample
  // This will give approx. the same result as DrawNetwork.
  // All entries are used, while DrawNetwork focuses on 
  // the test sample. Also the xaxis range is manually set.
  TH1F *bg2 = new TH1F("bg2h", "NN output", 50, -.5, 1.5);
  TH1F *bg = new TH1F("bgh", "NN output", 50, -.5, 1.5);
  TH1F *sig = new TH1F("sigh", "NN output", 50, -.5, 1.5);

  TH1F *bg2test = new TH1F("bg2htest", "NN output", 50, -.5, 1.5);
  TH1F *bgtest = new TH1F("bghtest", "NN output", 50, -.5, 1.5);
  TH1F *sigtest = new TH1F("sightest", "NN output", 50, -.5, 1.5);

  for (Int_t i = 0; i < simu->GetEntries(); i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " First plot. Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor!=0&&i%dilutionFactor!=1) continue;
    
    simu->GetEntry(i);

    jn->SetInputs(0, norm_nVTX(nVTX) );
    jn->SetInputs(1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputs(2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputs(3, norm_energyFraction(energyFraction) );
    jn->SetInputs(4, norm_mass(mass) );
    jn->SetInputs(5, norm_significance3d(significance3d ) );
    if (withIP3D) {
	jn->SetInputs(6, norm_IP3D(discriminatorIP3D) );
	jn->SetInputs(7, norm_cat_pT(cat_pT) );
	jn->SetInputs(8, norm_cat_eta(cat_eta) );
    }
    else {
      jn->SetInputs(6, norm_cat_pT(cat_pT) );
      jn->SetInputs(7, norm_cat_eta(cat_eta) );
    }

    jn->Evaluate();

    float bvalue=jn->GetOutput(0);
    float lvalue=jn->GetOutput(2);

    // TODO: this is where we need to play with the plot values
    if (bottom==1) {
      if (i%dilutionFactor==0) {
	sig->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	sigtest->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
    if (light==1) {
      if (i%dilutionFactor==0) {
	bg->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	bgtest->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
    if (charm==1) {
      if (i%dilutionFactor==0) {
	bg2->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	bg2test->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
  } // end of hist filling loop  

  //now you need the maximum
  float maximum=1;
  for (Int_t a=0;a<bg->GetNbinsX();a++) {
    if (bg->GetBinContent(a)>maximum) {
      maximum=1.2*bg->GetBinContent(a);
    }
  }

  bg2->SetLineColor(kYellow);
  bg2->SetFillStyle(3008);   bg2->SetFillColor(kYellow);
  bg->SetLineColor(kBlue);
  bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
  sig->SetLineColor(kRed);
  sig->SetFillStyle(3003); sig->SetFillColor(kRed);
  bg2->SetStats(0);
  bg->SetStats(0);
  sig->SetStats(0);


  bg2test->SetLineColor(kYellow);
  bg2test->SetFillStyle(3008);   bg2test->SetFillColor(kYellow);
  bgtest->SetLineColor(kBlue);
  bgtest->SetFillStyle(3008);   bgtest->SetFillColor(kBlue);
  sigtest->SetLineColor(kRed);
  sigtest->SetFillStyle(3003); sigtest->SetFillColor(kRed);
  bg2test->SetStats(0);
  bgtest->SetStats(0);
  sigtest->SetStats(0);

  mlpa_canvas->cd(1);
  gPad->SetLogy();

  bg->GetYaxis()->SetRangeUser(1,maximum);
  bgtest->GetYaxis()->SetRangeUser(1,maximum);

  mlpa_canvas->cd(1);
  bg->Draw();
  bg2->Draw("same");
  sig->Draw("same");

  TLegend *legend = new TLegend(.75, .80, .95, .95);
  legend->AddEntry(bg2, "Background2 (charm)");
  legend->AddEntry(bg, "Background (light)");
  legend->AddEntry(sig, "Signal (bottom)");
  legend->Draw();
 
  mlpa_canvas->cd(2);
  gPad->SetLogy();

  bgtest->Draw();
  bg2test->Draw("same");
  sigtest->Draw("same");

  TLegend *legendtest = new TLegend(.75, .80, .95, .95);
  legendtest->AddEntry(bg2test, "Background2 (charm)");
  legendtest->AddEntry(bgtest, "Background (light)");
  legendtest->AddEntry(sigtest, "Signal (bottom)");
  legendtest->Draw();

  std::cout << "drawing fith pad\n";
  mlpa_canvas->cd(5);
  gPad->SetLogy();
  bg->DrawNormalized();
  bg2->DrawNormalized("same");
  sig->DrawNormalized("same");
  legend->Draw();
 
  std::cout << "drawing sixth pad\n";
  mlpa_canvas->cd(6);
  gPad->SetLogy();
  bgtest->DrawNormalized();
  bg2test->DrawNormalized("same");
  sigtest->DrawNormalized("same");
  legendtest->Draw();


 
  mlpa_canvas->cd(3);
  gPad->SetLogy();
 
  // Use the NN to plot the results for each sample
  // This will give approx. the same result as DrawNetwork.
  // All entries are used, while DrawNetwork focuses on 
  // the test sample. Also the xaxis range is manually set.
  TH1F *c_bg2 = new TH1F("c_bg2h", "NN output", 50, -.5, 1.5);
  TH1F *c_bg = new TH1F("c_bgh", "NN output", 50, -.5, 1.5);
  TH1F *c_sig = new TH1F("c_sigh", "NN output", 50, -.5, 1.5);

  TH1F *c_bg2test = new TH1F("c_bg2htest", "NN output", 50, -.5, 1.5);
  TH1F *c_bgtest = new TH1F("c_bghtest", "NN output", 50, -.5, 1.5);
  TH1F *c_sigtest = new TH1F("c_sightest", "NN output", 50, -.5, 1.5);

  for (Int_t i = 0; i < simu->GetEntries(); i++) {
   
    if (i % 100000 == 0 ) {
      std::cout << " Second plot. Looping over event " << i << std::endl;
    }
   
    if (i%dilutionFactor!=0&&i%dilutionFactor!=1) continue;
   
    simu->GetEntry(i);

    jn->SetInputs(0, norm_nVTX(nVTX) );
    jn->SetInputs(1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputs(2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputs(3, norm_energyFraction(energyFraction) );
    jn->SetInputs(4, norm_mass(mass) );
    jn->SetInputs(5, norm_significance3d(significance3d ) );
    if (withIP3D) {
      jn->SetInputs(6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputs(7, norm_cat_pT(cat_pT) );
      jn->SetInputs(8, norm_cat_eta(cat_eta) );
    }
    else {
      jn->SetInputs(6, norm_cat_pT(cat_pT) );
      jn->SetInputs(7, norm_cat_eta(cat_eta) );
    }

    jn->Evaluate();

    float bvalue=jn->GetOutput(0);
    float cvalue=jn->GetOutput(1);

    if (bottom==1) {
      if (i%dilutionFactor==0) {
	c_sig->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	c_sigtest->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
    if (light==1) {
      if (i%dilutionFactor==0) {
	c_bg->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	c_bgtest->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
    if (charm==1) {
      if (i%dilutionFactor==0) {
	c_bg2->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else if (i%dilutionFactor==1) {
	c_bg2test->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
  } // end of hist filling loop 

  //now you need the maximum
  maximum=1;
  for (Int_t a=0;a<c_bg->GetNbinsX();a++) {
    if (c_bg->GetBinContent(a)>maximum) {
      maximum=1.2*c_bg->GetBinContent(a);
    }
  }

  // TODO: this is needless repetition (it's done above, shoud just have a
  // formatting function ) 

  c_bg2->SetLineColor(kYellow);
  c_bg2->SetFillStyle(3008);   c_bg2->SetFillColor(kYellow);
  c_bg->SetLineColor(kBlue);
  c_bg->SetFillStyle(3008);   c_bg->SetFillColor(kBlue);
  c_sig->SetLineColor(kRed);
  c_sig->SetFillStyle(3003); c_sig->SetFillColor(kRed);
  c_bg2->SetStats(0);
  c_bg->SetStats(0);
  c_sig->SetStats(0);
 
  c_bg2test->SetLineColor(kYellow);
  c_bg2test->SetFillStyle(3008);   c_bg2test->SetFillColor(kYellow);
  c_bgtest->SetLineColor(kBlue);
  c_bgtest->SetFillStyle(3008);   c_bgtest->SetFillColor(kBlue);
  c_sigtest->SetLineColor(kRed);
  c_sigtest->SetFillStyle(3003); c_sigtest->SetFillColor(kRed);
  c_bg2test->SetStats(0);
  c_bgtest->SetStats(0);
  c_sigtest->SetStats(0);

  mlpa_canvas->cd(3);
  gPad->SetLogy();


  c_bg->GetYaxis()->SetRangeUser(1,maximum);
  c_bgtest->GetYaxis()->SetRangeUser(1,maximum);
   
  c_bg->Draw();
  c_bg2->Draw("same");
  c_sig->Draw("same");

  TLegend *legend2 = new TLegend(.75, .80, .95, .95);
  legend2->AddEntry(c_bg2, "Background2 (charm)");
  legend2->AddEntry(c_bg, "Background (light)");
  legend2->AddEntry(c_sig, "Signal (bottom)");
  legend2->Draw();

  mlpa_canvas->cd(4);
  gPad->SetLogy();
   
  c_bgtest->Draw();
  c_bg2test->Draw("same");
  c_sigtest->Draw("same");

  TLegend *legend2test = new TLegend(.75, .80, .95, .95);
  legend2test->AddEntry(c_bg2test, "Background2 (charm)");
  legend2test->AddEntry(c_bgtest, "Background (light)");
  legend2test->AddEntry(c_sigtest, "Signal (bottom)");
  legend2test->Draw();

  mlpa_canvas->cd(7);
  gPad->SetLogy();
  c_bg->DrawNormalized();
  c_bg2->DrawNormalized("same");
  c_sig->DrawNormalized("same");
  legend2->Draw();
 
  mlpa_canvas->cd(8);
  gPad->SetLogy();
  c_bgtest->DrawNormalized();
  c_bg2test->DrawNormalized("same");
  c_sigtest->DrawNormalized("same");
  legend2test->Draw();


  mlpa_canvas->cd(0);

  mlpa_canvas->SaveAs("weights/result.eps");
}

