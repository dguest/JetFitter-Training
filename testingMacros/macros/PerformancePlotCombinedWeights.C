{

  gROOT->ProcessLine(".x ~/atlasmacros/rootlogon.C");
  gROOT->ProcessLine(".L ~/atlasmacros/utils.h");
  gROOT->SetStyle("ATLAS");
//  gROOT->SetStyle("Plain");

//  TFile file("simpleTree.root");
//  TTree* myTree=(TTree*)file.Get("SVTree");

//  TFile file1("WH_muel_atlfast.root");
//  TFile file2("WH_output_ttbar_minPt.root");
//  TFile file3("WH_output_Wj.root");

  TChain* myTree=new TChain("AntiKt4Jets");
  myTree->AddFile("outputdataset_AntiKt4Jets_jetnet.root");
//  myTree->AddFile("../outputCutFlow/WH_output_ttbar_minPt.root");
//  myTree->AddFile("../outputCutFlow/WH_output_Wt.root");
//  myTree->AddFile("../outputCutFlow/WH_output_Wj.root");
//  myTree->AddFile("../outputCutFlow/WH_Wbb.root");

  TTree* myTree2=myTree;//needed???


  Float_t minimum=0;
  Float_t maximum=12.;

  Float_t rejectionx[20000];
  Float_t rejectionyl[20000];
  Float_t rejectionyc[20000];
  Float_t efficiencyx[20000];
  Float_t efficiencyyl[20000];
  Float_t efficiencyyc[20000];


  Float_t oldrejectionx[20000];
  Float_t oldrejectionyl[20000];
  Float_t oldrejectionyc[20000];
  Float_t oldefficiencyx[20000];
  Float_t oldefficiencyyl[20000];
  Float_t oldefficiencyyc[20000];

  Float_t charmrejectionx[20000];
  Float_t charmrejectionyl[20000];
  Float_t charmrejectionyc[20000];
  Float_t charmefficiencyx[20000];
  Float_t charmefficiencyyl[20000];
  Float_t charmefficiencyyc[20000];
  
  Int_t num_interval=1000;
  
  TH1F* forb1=new TH1F("forb1","forb1",20000,-3,5);
  TH1F* forl1=new TH1F("forl1","forl1",20000,-3,5);
  TH1F* forc1=new TH1F("forc1","forc1",20000,-3,5);

//TString additionalCondition("filteredPt>200e3&&abs(filteredEta)<2.5&&highestPtLepton>30e3&&isLeptonAroundHiggsSubJet==0&&nextPtMuon<30e3&&nextPtElectron<30e3&&MET>30e3&&WorZpT>200e3&&AdditionalBJetPt<20e3&&AdditionalJetPt<30e3&&DeltaR12>0.3&&filteredMass>40e3&&filteredMass<200e3");
//TString additionalCondition("passedUpToCut>=13&&filteredMass>105e3&&filteredMass<127e3");
//TString additionalCondition("passedUpToCut>=13&&filteredMass>100e3&&filteredMass<150e3");
//TString additionalCondition("passedUpToCut>=17");
 TString additionalCondition("JetPt>50&&JetPt<180&&abs(JetEta)<1.5");
//TString additionalCondition("Entry$ % 2 ==1");

 TString bstring1("cat_flavour==5");
//  TString bstring2("cat_flavour2==5");
  if (additionalCondition!="")
  {
    bstring1+="&&";
    bstring1+=additionalCondition;
//    bstring2+="&&";
//    bstring2+=additionalCondition;
  }

  TString cstring1("cat_flavour==4");
//  TString cstring2("cat_flavour2==4");
  if (additionalCondition!="")
  {
    cstring1+="&&";
    cstring1+=additionalCondition;
//    cstring2+="&&";
//    cstring2+=additionalCondition;
  }
  
  TString lstring1("cat_flavour==1");
//  TString lstring2("cat_flavour2==1");
  if (additionalCondition!="")
  {
    lstring1+="&&";
    lstring1+=additionalCondition;
//    lstring2+="&&";
//    lstring2+=additionalCondition;
  }
   
  myTree->Project("forb1","NNb/(NNb+NNu)",bstring1);
  myTree->Project("forl1","NNb/(NNb+NNu)",lstring1);
  myTree->Project("forc1","NNb/(NNb+NNu)",cstring1);

  TH1F* oldforb1=new TH1F("oldforb1","oldforb1",20000,-20,50);
  TH1F* oldforl1=new TH1F("oldforl1","oldforl1",20000,-20,50);
  TH1F* oldforc1=new TH1F("oldforc1","oldforc1",20000,-20,50);

//  TH1F* oldforb2=new TH1F("oldforb2","oldforb2",20000,-20,50);
//  TH1F* oldforl2=new TH1F("oldforl2","oldforl2",20000,-20,50);
//  TH1F* oldforc2=new TH1F("oldforc2","oldforc2",20000,-20,50);
  
  myTree->Project("oldforb1","discriminatorCOMB",bstring1);
  myTree->Project("oldforl1","discriminatorCOMB",lstring1);
  myTree->Project("oldforc1","discriminatorCOMB",cstring1);

  TH1F* charmforb1=new TH1F("charmforb1","charmforb1",20000,-20,50);
  TH1F* charmforl1=new TH1F("charmforl1","charmforl1",20000,-20,50);
  TH1F* charmforc1=new TH1F("charmforc1","charmforc1",20000,-20,50);
  
  myTree->Project("charmforb1","NNb/(NNb+NNc)",bstring1);
  myTree->Project("charmforl1","NNb/(NNb+NNc)",lstring1);
  myTree->Project("charmforc1","NNb/(NNb+NNc)",cstring1);

//  TH1F* forb=new TH1F(*forb1+*forb2);
//  TH1F* forc=new TH1F(*forc1+*forc2);
//  TH1F* forl=new TH1F(*forl1+*forl2);

TH1F* forb=forb1;
TH1F* forc=forc1;
TH1F* forl=forl1;

  TH1F* oldforb=oldforb1;
  TH1F* oldforc=oldforc1;
  TH1F* oldforl=oldforl1;

//  TH1F* oldforb=new TH1F(*oldforb1+*oldforb2);
//  TH1F* oldforc=new TH1F(*oldforc1+*oldforc2);
//  TH1F* oldforl=new TH1F(*oldforl1+*oldforl2);

//  TH1F* charmforb=new TH1F(*charmforb1+*charmforb2);
//  TH1F* charmforc=new TH1F(*charmforc1+*charmforc2);
//  TH1F* charmforl=new TH1F(*charmforl1+*charmforl2);

  TH1F* charmforb=charmforb1;
  TH1F* charmforc=charmforc1;
  TH1F* charmforl=charmforl1;

  Double_t totalb=0;
  Double_t totalc=0;
  Double_t totall=0;

cout << "n b " << totalb << " n c " << totalc << 
    " n l " << totall << endl;
  
Double_t allbsofar(0);
Double_t allcsofar(0);
Double_t alllsofar(0);

Double_t oldallbsofar(0);
Double_t oldallcsofar(0);
Double_t oldalllsofar(0);

Double_t charmallbsofar(0);
Double_t charmallcsofar(0);
Double_t charmalllsofar(0);



  for (int s=0;s<forb->GetNbinsX();s++) {
    
    
    allbsofar+=forb->GetBinContent(forb->GetNbinsX()-s);
//   cout << " all b so far: " << allbsofar << endl;
    allcsofar+=forc->GetBinContent(forc->GetNbinsX()-s);
    alllsofar+=forl->GetBinContent(forl->GetNbinsX()-s);
//   cout << " all l so far: " << alllsofar << endl;
 
    oldallbsofar+=oldforb->GetBinContent(oldforb->GetNbinsX()-s);
//    cout << " oldallbsofar " << oldallbsofar << endl;
    oldallcsofar+=oldforc->GetBinContent(oldforc->GetNbinsX()-s);
//    cout << " oldallcsofar " << oldallcsofar << endl;
    oldalllsofar+=oldforl->GetBinContent(oldforl->GetNbinsX()-s);
//    cout << " oldalllsofar " << oldalllsofar << endl;

    charmallbsofar+=charmforb->GetBinContent(charmforb->GetNbinsX()-s);
//    cout << " charmallbsofar " << charmallbsofar << endl;
    charmallcsofar+=charmforc->GetBinContent(charmforc->GetNbinsX()-s);
    charmalllsofar+=charmforl->GetBinContent(charmforl->GetNbinsX()-s);

//    std::cout << " AA " << std::endl;


    //    Float_t value=minimum+(maximum-minimum)*s/num_interval;
//    efficiencyx[s]=(float)allbsofar/(float)totalb;   
    efficiencyx[s]=(float)allbsofar;
    //(float)totalb;
    rejectionx[s]=(float)allbsofar;
    ///(float)totalb;

    oldefficiencyx[s]=(float)oldallbsofar;
///(float)totalb;
    oldrejectionx[s]=(float)oldallbsofar;
///(float)totalb;

    charmefficiencyx[s]=(float)charmallbsofar;
///(float)totalb;
    charmrejectionx[s]=(float)charmallbsofar;
///(float)totalb;

    
//    std::cout << " BB " << std::endl;

    if (alllsofar!=0) {
      rejectionyl[s]=(float)alllsofar;
//     rejectionyl[s]=(float)totall;
    } else {
      rejectionyl[s]=-1000;
    }

    if (oldalllsofar!=0) {
      oldrejectionyl[s]=(float)oldalllsofar;
    } else {
      oldrejectionyl[s]=-1000;
    }

    if (charmalllsofar!=0) {
      charmrejectionyl[s]=(float)charmalllsofar;
    } else {
      charmrejectionyl[s]=-1000;
    }

    if (allcsofar!=0) {
      rejectionyc[s]=(float)allcsofar;
    } else {
      rejectionyc[s]=-1000;
    }
   
    if (oldallcsofar!=0) {
      oldrejectionyc[s]=(float)oldallcsofar;
    } else {
      oldrejectionyc[s]=-1000;
    }

    if (charmallcsofar!=0) {
      charmrejectionyc[s]=(float)charmallcsofar;
    } else {
      charmrejectionyc[s]=-1000;
    }
 
    efficiencyyl[s]=(float)alllsofar;
    efficiencyyc[s]=(float)allcsofar;
    oldefficiencyyl[s]=(float)oldalllsofar;
    oldefficiencyyc[s]=(float)oldallcsofar;
    charmefficiencyyl[s]=(float)charmalllsofar;
    charmefficiencyyc[s]=(float)charmallcsofar;
    
//    std::cout << " CC " << std::endl;

  }
  
//std::cout << " A " << std::endl;


int bit=forb->GetNbinsX()-1;

//std::cout << " B " << std::endl;

    totalb=efficiencyx[bit];
    cout << " totalb " << totalb << endl;
    totalc=rejectionyc[bit];
    cout << " totalc " << totalc << endl;
    totall=rejectionyl[bit];
    cout << " totall " << totall << endl;

//std::cout << " C " << std::endl;

  TFile rejectionplots("rejectionplots.root","recreate");

for (int a=0;a<forb->GetNbinsX();a++) 
{

  
  efficiencyx[a]=totalb>0?(efficiencyx[a])/totalb:0;
  rejectionx[a]=totalb>0?(rejectionx[a])/totalb:0;

  oldefficiencyx[a]=totalb>0?(oldefficiencyx[a])/totalb:0;
  oldrejectionx[a]=totalb>0?(oldrejectionx[a])/totalb:0;

  charmefficiencyx[a]=totalb>0?(charmefficiencyx[a])/totalb:0;
  charmrejectionx[a]=totalb>0?(charmrejectionx[a])/totalb:0;
  
  if (rejectionyl[a]>0)
  {
    
    rejectionyl[a]=totall/(rejectionyl[a]);
  }
  else 
  {
    rejectionyl[a]=1e4;
  }
  


  if (oldrejectionyl[a]>0)
  {
    oldrejectionyl[a]=totall/(oldrejectionyl[a]);
  }
  else
  {
    oldrejectionyl[a]=1e3;
  }

  if (charmrejectionyl[a]>0)
  {
    charmrejectionyl[a]=totall/(charmrejectionyl[a]);
  }
  else
  {
    charmrejectionyl[a]=1e3;
  }

  
  
  if (rejectionyc[a]>0)
  {
    rejectionyc[a]=totalc/(rejectionyc[a]);
  }
  else
  {
    rejectionyc[a]=1e6;
  }
  

  if (oldrejectionyc[a]>0)
  {
    oldrejectionyc[a]=totalc/(oldrejectionyc[a]);
  }
  else
  {
    oldrejectionyc[a]=1e6;
  }

  if (charmrejectionyc[a]>0)
  {
    charmrejectionyc[a]=totalc/(charmrejectionyc[a]);
  }
  else
  {
    charmrejectionyc[a]=1e6;
  }
  

  if (totall>0)
  {
    efficiencyyl[a]=(efficiencyyl[a])/totall;
  } 
  else
  {
    efficiencyyl[a]=1e6;
  }
  

  if (totall>0)
  {
    oldefficiencyyl[a]=(oldefficiencyyl[a])/totall;
  }
  else
  {
    oldefficiencyyl[a]=1e6;
  }

  if (totall>0)
  {
    charmefficiencyyl[a]=(charmefficiencyyl[a])/totall;
  }
  else
  {
    charmefficiencyyl[a]=1e6;
  }
  
    
  
  if (totalc>0)
  {
    efficiencyyc[a]=(efficiencyyc[a])/totalc;
  }
  else 
  {
    efficiencyyc[a]=1e6;
  }
  

  if (totalc>0)
  {
    oldefficiencyyc[a]=(oldefficiencyyc[a])/totalc;
  }
  else
  {
    oldefficiencyyc[a]=1e6;
  }

  if (totalc>0)
  {
    charmefficiencyyc[a]=(charmefficiencyyc[a])/totalc;
  }
  else
  {
    charmefficiencyyc[a]=1e6;
  }
  
}

std::cout << " D " << std::endl;

//  std::cout << " l n. " << a << " rejx" << rejectionx[a] << " rejl " << rejectionyl[a] << std::endl;
//  std::cout << " c n. " << a << " rejx" << rejectionx[a] << " rejl " << rejectionyc[a] << std::endl;
//}

  
  TGraph* rejectionplotl=new TGraph(20000,rejectionx,rejectionyl);
  TGraph* rejectionplotc=new TGraph(20000,rejectionx,rejectionyc);

  TGraph* oldrejectionplotl=new TGraph(20000,oldrejectionx,oldrejectionyl);
  TGraph* oldrejectionplotc=new TGraph(20000,oldrejectionx,oldrejectionyc);

  TGraph* charmrejectionplotl=new TGraph(20000,charmrejectionx,charmrejectionyl);
  TGraph* charmrejectionplotc=new TGraph(20000,charmrejectionx,charmrejectionyc);


  //  TGraph* efficiencyplot=new TGraph(1001,efficiencyx,efficiencyy);
  
  TCanvas c1("c1","c1",400,400);
  c1.SetLogy();

   rejectionplotl->SetLineColor(3);
   rejectionplotl->SetLineWidth(3);
   rejectionplotl->SetLineStyle(9);
   rejectionplotl->SetMarkerColor(3);
   rejectionplotl->SetMarkerStyle(1);
   rejectionplotl->SetTitle("Light jet rejection with VKalVrt (green) and with the new JetFitter (blue) (combined with IP3d)");
   rejectionplotl->GetXaxis()->SetTitle("b-tagging efficiency");
   rejectionplotl->GetYaxis()->SetRangeUser(1,2e3);
   rejectionplotl->GetYaxis()->SetTitle("Light jet rejection");
   rejectionplotl->GetXaxis()->SetRangeUser(0.4,1);
   rejectionplotl->Draw("AC");

   charmrejectionplotl->SetLineColor(2);
   charmrejectionplotl->SetLineStyle(7);
   charmrejectionplotl->SetLineWidth(3);
   charmrejectionplotl->SetMarkerColor(2);
   charmrejectionplotl->SetMarkerStyle(1);
   charmrejectionplotl->SetTitle("a simple rejectionplotlaph");
   charmrejectionplotl->GetXaxis()->SetTitle("b-tagging efficiency");
   charmrejectionplotl->GetYaxis()->SetRangeUser(1,1e2);
   charmrejectionplotl->GetYaxis()->SetTitle("Light quark rejection");
   charmrejectionplotl->GetXaxis()->SetRangeUser(0.5,1);
//   charmrejectoinplotl->SetLineStyle(2);
   charmrejectionplotl->Draw("C");


   oldrejectionplotl->SetLineColor(4);
   oldrejectionplotl->SetLineWidth(3);
   oldrejectionplotl->SetMarkerColor(4);
   oldrejectionplotl->SetMarkerStyle(1);
   oldrejectionplotl->SetTitle("a simple rejectionplotlaph");
   oldrejectionplotl->GetXaxis()->SetTitle("b-tagging efficiency");
   oldrejectionplotl->GetYaxis()->SetRangeUser(1,1e2);
   oldrejectionplotl->GetYaxis()->SetTitle("Light quark rejection");
   oldrejectionplotl->GetXaxis()->SetRangeUser(0.5,1);
//   oldrejectionplotl->SetLineStyle(3);
   oldrejectionplotl->Draw("C");

  TLegend *lcanvas4 = new TLegend(0.55,0.7,0.94,0.82);
  lcanvas4->SetFillColor(0);
////  l1->SetBorderSize(1);
////  l1->SetLineWidth(1);
////  l1->SetMargin(0.1);
  lcanvas4->AddEntry(rejectionplotl, "JetFitter Tagger", "l");
  lcanvas4->AddEntry(oldrejectionplotl, "COMB Tagger", "l");
  lcanvas4->AddEntry(charmrejectionplotl, "JetFitter Tagger (rej. charm)", "l");
  lcanvas4->Draw("same");

   ATLAS_LABEL(0.15,0.20);

   c1.SaveAs("CombinedPerformanceLightWeights.eps");
   c1.SaveAs("CombinedPerformanceLightWeight.gif");
   
   TCanvas c2("c2","c2",400,400);
   c2.SetLogy();
   
   charmrejectionplotc->SetLineColor(2);
   charmrejectionplotc->SetLineWidth(3);
   charmrejectionplotc->SetLineStyle(7);
   charmrejectionplotc->SetMarkerColor(2);
   charmrejectionplotc->SetMarkerStyle(1);
   //   charmrejectionplotc->SetTitle("a simple rejectionplotcaph");
   charmrejectionplotc->GetXaxis()->SetTitle("b-tagging efficiency");
   charmrejectionplotc->GetYaxis()->SetRangeUser(1,5e1);
   charmrejectionplotc->GetYaxis()->SetTitle("Charm jet rejection");
   charmrejectionplotc->GetXaxis()->SetRangeUser(0.4,1);
   charmrejectionplotc->Draw("AC");

   rejectionplotc->SetLineColor(3);
   rejectionplotc->SetLineWidth(3);
   rejectionplotc->SetMarkerColor(3);
   rejectionplotc->SetLineStyle(9);
   rejectionplotc->SetMarkerStyle(1);
   rejectionplotc->SetTitle("Charm jet rejection with VKalVrt (green) and with the new JetFitter (blue) (combined with IP3d)");
   rejectionplotc->GetXaxis()->SetTitle("b-tagging efficiency");
   rejectionplotc->GetYaxis()->SetRangeUser(1,5e1);
   rejectionplotc->GetYaxis()->SetTitle("Charm jet rejection");
   rejectionplotc->GetXaxis()->SetRangeUser(0.4,1);
   rejectionplotc->Draw("C");
   
   oldrejectionplotc->SetLineColor(4);
   oldrejectionplotc->SetLineWidth(3);
   oldrejectionplotc->SetMarkerColor(4);
   oldrejectionplotc->SetMarkerStyle(1);
   //   oldrejectionplotc->SetTitle("a simple rejectionplotcaph");
   oldrejectionplotc->GetXaxis()->SetTitle("b-tagging efficiency");
   oldrejectionplotc->GetYaxis()->SetRangeUser(1,5e1);
   oldrejectionplotc->GetYaxis()->SetTitle("Charm jet rejection");
   oldrejectionplotc->GetXaxis()->SetRangeUser(0.4,1);
   oldrejectionplotc->Draw("C");
   
   ATLAS_LABEL(0.15,0.20);

  TLegend *lcanvas5 = new TLegend(0.55,0.7,0.94,0.82);
  lcanvas5->SetFillColor(0);
////  l1->SetBorderSize(1);
////  l1->SetLineWidth(1);
////  l1->SetMargin(0.1);
  lcanvas5->AddEntry(rejectionplotc, "JetFitter Tagger", "l");
  lcanvas5->AddEntry(oldrejectionplotc, "COMB Tagger", "l");
  lcanvas5->AddEntry(charmrejectionplotc, "JetFitter Tagger (rej. charm)", "l");
  lcanvas5->Draw("same");

   c2.SaveAs("CombinedPerformanceCharmWeights.eps");
   c2.SaveAs("CombinedPerformanceCharmWeights.gif");

   c2.Update();

   TCanvas c3("c3","c3",400,400);
   c3.SetLogy();


   for (int s=0;s<20000;s++) {

     if (rejectionx[s]>0.40) {
       cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
	 rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }
   for (int s=0;s<20000;s++) {
     if (oldrejectionx[s]>0.40) {
       cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
	 oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }
   for (int s=0;s<20000;s++) {
     if (charmrejectionx[s]>0.40) {
       cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
	 charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }

   for (int s=0;s<20000;s++) {

     if (rejectionx[s]>0.50) {
       cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
	 rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }
   for (int s=0;s<20000;s++) {
     if (oldrejectionx[s]>0.50) {
       cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
	 oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }
   for (int s=0;s<20000;s++) {
     if (charmrejectionx[s]>0.50) {
       cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
	 charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }

   for (int s=0;s<20000;s++) {

     if (rejectionx[s]>0.60) {
       cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
	 rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }
   for (int s=0;s<20000;s++) {
     if (oldrejectionx[s]>0.60) {
       cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
	 oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }

   for (int s=0;s<20000;s++) {
     if (charmrejectionx[s]>0.60) {
       cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
	 charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
	 " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
       break;
     }
   }

for (int s=0;s<20000;s++) {
  if (rejectionx[s]>0.665) {
    cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
        rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << forb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (oldrejectionx[s]>0.665) {
    cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
        oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << oldforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (charmrejectionx[s]>0.665) {
    cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
        charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << charmforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}



for (int s=0;s<20000;s++) {
  if (rejectionx[s]>0.70) {
    cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
        rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << forb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}
for (int s=0;s<20000;s++) {
  if (oldrejectionx[s]>0.70) {
    cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
        oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << oldforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}


for (int s=0;s<20000;s++) {
  if (charmrejectionx[s]>0.70) {
    cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
        charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << charmforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (rejectionx[s]>0.727) {
    cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
        rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << forb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (oldrejectionx[s]>0.727) {
    cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
        oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << oldforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (charmrejectionx[s]>0.727) {
    cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
        charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    cout << "cut value was: " << charmforb->GetBinCenter(forb->GetNbinsX()-s) << endl;
    break;
  }
}

for (int s=0;s<20000;s++) {
  if (rejectionx[s]>0.80) {
    cout << "efficiency on B at " << rejectionx[s] << endl << " light rejection " << 
        rejectionyl[s] << " +/- " << TMath::Power(rejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << rejectionyc[s] << " +/- " << TMath::Power(rejectionyc[s],3./2.)/sqrt(totalc) << endl;
    break;
  }
}
for (int s=0;s<20000;s++) {
  if (oldrejectionx[s]>0.80) {
    cout << "old efficiency on B at " << oldrejectionx[s] << endl << " light rejection " << 
        oldrejectionyl[s] << " +/- " << TMath::Power(oldrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << oldrejectionyc[s] << " +/- " << TMath::Power(oldrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    break;
  }
}
for (int s=0;s<20000;s++) {
  if (charmrejectionx[s]>0.80) {
    cout << "charm efficiency on B at " << charmrejectionx[s] << endl << " light rejection " << 
        charmrejectionyl[s] << " +/- " << TMath::Power(charmrejectionyl[s],3./2.)/sqrt(totall) << 
        " charm rejection " << charmrejectionyc[s] << " +/- " << TMath::Power(charmrejectionyc[s],3./2.)/sqrt(totalc) << endl;
    break;
  }
}
  //  rejectionplotc->Draw("AC");
  //  efficiencyplot.Draw();
  c3.Update();


  rejectionplots->Write();


}
