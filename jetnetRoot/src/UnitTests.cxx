
// this is a work in progress

  Int_t nInput=jn->GetInputDim();
  
  cout << " create Trained Network object..." << endl;
  
  TTrainedNetwork* trainedNetwork = jn->createTrainedNetwork();

  cout << " now getting value with trained Network ";

  double inputexample[9] = {norm_nVTX(1),
			    norm_nTracksAtVtx(2),
			    norm_nSingleTracks(0),
			    norm_energyFraction(0.6),
			    norm_mass(2500),
			    norm_significance3d(4 ),
			    norm_IP3D(3),
			    norm_cat_pT(3),
			    norm_cat_eta(1)};

  for (Int_t i=0;i<nInput;++i){
    jn->SetInputs(i,inputexample[i]);
  }

  
  cronology.open(chronology_name.c_str(), ios_base::app);

  jn->Evaluate();

  cronology << "----------------CONSISTENCY CHECK-----------" << endl;
  cout << "Result 0:" << jn->GetOutput(0);
  cronology << "Result 0:" << jn->GetOutput(0);
  cout << " Result 1:" << jn->GetOutput(1);
  cronology << "Result 0:" << jn->GetOutput(1);
  cout << " Result 2:" << jn->GetOutput(2) << endl;
  cronology << " Result 2:" << jn->GetOutput(2) << endl;

  cout << " Reading back old network " << endl;
  jn->readBackTrainedNetwork(trainedNetwork);

  cout <<" resetting input " << endl;
  for (Int_t i=0;i<nInput;++i) {
    jn->SetInputs(i,inputexample[i]);
  }

  jn->Evaluate();
 
  cout << "After reading back - Result 0:" << jn->GetOutput(0);
  cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // <<     " my: " << result[0] << endl;
  cout << " After reading back - Result 1:" << jn->GetOutput(1);
  cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  //<<     " my: " << result[1] << endl;
  cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // << " my: " << result[2] << endl;

  cout << " Now getting histograms from trainingResult" << endl;
  cronology << " Now getting histograms from trainingResult" << endl;

  NetworkToHistoTool myHistoTool;

  cout << " From network to histo..." << endl;
  OwnerVector<TH1*> myHistos = myHistoTool.
    fromTrainedNetworkToHisto(trainedNetwork);

  cout << " From histo to network back..." << endl;
  TTrainedNetwork* trainedNetwork2 = myHistoTool.
    fromHistoToTrainedNetwork(myHistos);

  cout << " reading back " << endl;
  jn->readBackTrainedNetwork(trainedNetwork2);
   
  cout <<" resetting input " << endl;
  for (Int_t i=0;i<nInput;++i) {
    jn->SetInputs(i,inputexample[i]);
  }

  jn->Evaluate();

  cout << "After reading back - Result 0:" << jn->GetOutput(0);
  cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // <<     " my: " << result[0] << endl;
  cout << " After reading back - Result 1:" << jn->GetOutput(1);
  cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  //<<     " my: " << result[1] << endl;
  cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // << " my: " << result[2] << endl;
  
  cout << " Directly from the trainedNetwork read back from HISTOS...!" << endl;

  std::vector<Double_t> inputData;
  for (Int_t u=0;u<nInput;++u) {
    inputData.push_back(inputexample[u]);
  }

  std::vector<Double_t> outputData=trainedNetwork2->
    calculateOutputValues(inputData);

  cout << "After reading back - Result 0:" << outputData[0] << endl;
  cout << " After reading back - Result 1:" << outputData[1] << endl;
  cout << " After reading back - Result 2:" << outputData[2] << endl;
   
    cout << " -------------------- " << endl;
    cout << " Writing OUTPUT histos " << endl;
    std::string histo_weights_name = out_dir + "/histoWeights.root"; 
    TFile* fileHistos=new TFile(histo_weights_name.c_str(),"recreate");
    NetworkToHistoTool histoTool;
    std::vector<TH1*> myHistos=histoTool.
      fromTrainedNetworkToHisto(trainedNetwork);

    for (std::vector<TH1*>::iterator histoIter = myHistos.begin();
	 histoIter != myHistos.end(); 
	 ++histoIter) {
      (*histoIter)->Write();
      delete *histoIter; 
      *histoIter = 0; 
    }

    // SUSPICIOUS: may be writing twice here too
    // fileHistos->Write();
    fileHistos->Close();
    delete fileHistos;
