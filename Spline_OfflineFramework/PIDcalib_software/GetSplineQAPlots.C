void GetSplineQAPlots(TString file, Bool_t plotSystematics = kTRUE, Int_t multStepSize = -1, Int_t maxmult = 0) {
  TFile* f = new TFile(file.Data(),"UPDATE");
  if (!f) {
    cout << "File " << file << " not found." << endl;
    return;
  }
  
  if (file.Contains("outputCheckShapeEta") && !file.Contains("Adv")) {
    TCanvas* c = new TCanvas();
    TH2D* hPureResults = 0x0;
    f->GetObject("hPureResults",hPureResults);
    hPureResults->SetDirectory(0x0);
    hPureResults->Draw("surf1");
  }
  else if (file.Contains("checkPullSigma")) {
    TCanvas* c = (TCanvas*)f->FindObjectAny("canvPullMeanCorr");
    TH1D* h = (TH1D*)c->GetListOfPrimitives()->FindObject("hPullAdditionalCorrTheta_0_1");
        h->SetDirectory(0x0);
    h->GetYaxis()->SetRangeUser(-0.5,0.5);
    c->Draw();
    TCanvas* c1 = (TCanvas*)f->FindObjectAny("canvPullSigmaCorr");
    TH1D* h1 = (TH1D*)c1->GetListOfPrimitives()->FindObject("hPullAdditionalCorrTheta_0_2");
    h1->SetDirectory(0x0);
    h1->GetYaxis()->SetRangeUser(0.5,1.5);
    c1->Draw();
    // added by Jiyoung, just to make pdf file for checking
    c->SaveAs("checkPullTree_mean.pdf");
    c1->SaveAs("checkPullTree_sigma.pdf");
  }
  else if (file.Contains("EtaAdv")) {
    gStyle->SetOptStat("");
    TSubString path = file(0,file.Last(*((TString("/").Data()))));
    const Int_t nOfHist = 8;
    TString histNamesBases[nOfHist] = { "PMeandEdxDependenceDeltaPrime_El" , "PMeandEdxDependenceDeltaPrime_Ka", "PMeandEdxDependenceDeltaPrime_Pi", "PMeandEdxDependenceDeltaPrime_Pr", "PMeandEdxDependenceDeltaPrime_V0plusTOFel", "PMeandEdxDependenceDeltaPrime_V0el", "PMeandEdxDependenceDeltaPrime_V0pi", "PMeandEdxDependenceDeltaPrime_V0pr"};
    Int_t maxMultForSpecies[nOfHist] = {-1, -1, -1, -1, -1, -1, -1, -1};
    TString histTitlesDifference[nOfHist] = { "TPC (and TOF) electrons", "TPC (and TOF) kaons", "TPC (and TOF) #pi", "TPC (and TOF) protons", "V0 plus TOF electron", "V0 electrons", "V0 #pi", "V0 protons"};
    Bool_t histusedforSystematics[nOfHist] = {kFALSE, kTRUE, kFALSE, kTRUE, kFALSE, kTRUE, kTRUE, kTRUE};
    TH1F* systHists[nOfHist] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    Int_t colorsSystematics[nOfHist] = {0, kGreen, 0, kBlack, 0, kMagenta, kRed, kBlue};
    TCanvas* canvases[nOfHist];
    TCanvas* multcanvases[nOfHist];
    for (Int_t i=0;i<nOfHist;++i)
      multcanvases[i] = 0x0;
    for (Int_t i=0;i<nOfHist;++i) {
      Int_t j=0;
      Int_t k=0;
    //Looping through multiplicity QA plots, if available
      legend = new TLegend(0.1,0.7,0.48,0.9);
      while (j <= (maxmult - multStepSize) && (maxMultForSpecies[i] == -1 || j < maxMultForSpecies[i])) {    
        TString histname = TString::Format("h%s_Multiplicity%i_%i",histNamesBases[i].Data(),j,j + multStepSize);
        TH1F* h = (TH1F*)f->FindObjectAny(histname.Data());
        if (!h)
          continue;
        if (!multcanvases[i]) {
          TString canvasname = TString::Format("c%s_MultComp",histNamesBases[i].Data());
          multcanvases[i] = new TCanvas(canvasname.Data(),canvasname.Data());
        }
        h = (TH1F*)h->Clone(histname.Data());
        h->SetDirectory(0x0);
        h->SetTitle(TString::Format("Momentum dependence of #Delta' for %s, comparison for different multiplicities",histTitlesDifference[i].Data()).Data());
        h->SetLineColor(kBlack + k);
        multcanvases[i]->cd();
        h->Draw(j==0 ? "" : "same");
        if (plotSystematics && j==0)
          systHists[i] = (TH1F*)h->Clone((histname + TString("_syst")).Data());
        
        legend->AddEntry(h,TString::Format("Multiplicity %i - %i",j,j+multStepSize).Data());
        k++;
        j+= multStepSize;
      }
      if (multcanvases[i]) {
        multcanvases[i]->cd();
        legend->Draw("same");
        multcanvases[i]->Write(0x0,TObject::kOverwrite);
      }
//       Getting QA plots for all multiplicities
      TString histname = TString::Format("h%s_AllMultiplicites",histNamesBases[i].Data());
      TH1F* h = (TH1F*)f->FindObjectAny(histname.Data());
      if (!h)
        continue;
      canvases[i] = new TCanvas(TString::Format("c%s",histNamesBases[i].Data()).Data(),TString::Format("c%s",histNamesBases[i].Data()).Data());
      h = (TH1F*)h->Clone(histname.Data());
      h->SetDirectory(0x0);
      h->SetTitle(TString::Format("Momentum dependence of #Delta' for %s, all multiplicities",histTitlesDifference[i].Data()).Data());
      h->Draw();
      if (plotSystematics && !systHists[i])
        systHists[i] = (TH1F*)h->Clone((histname + TString("_syst")).Data());
    }
    
    if (plotSystematics) {
      TCanvas* csplineAccuracy = new TCanvas("csplineAccuracyLowP_Test");
      csplineAccuracy->cd();
      csplineAccuracy->SetLogx();
      for (Int_t i=0;i<nOfHist;++i) {
        systHists[i]->SetLineColor(colorsSystematics[i]);
        systHists[i]->SetLineWidth(2);
        systHists[i]->SetFillColor(colorsSystematics[i]);
        systHists[i]->GetXaxis()->SetRangeUser(0.15,10);
        systHists[i]->GetXaxis()->SetMoreLogLabels();
        systHists[i]->GetXaxis()->SetNoExponent();
        systHists[i]->SetTitle("Momentum dependence of mean d#it{E}/d#it{x} for different species");
        systHists[i]->Draw(i==0 ? "" : "same");        
      }
      TFile* sysFile = new TFile((path + "/SplineSystematics.root").Data(),"UPDATE");
      csplineAccuracy->Write(0x0,TObject::kOverwrite);
      sysFile->Close();
    }
  }
    
  f->Close();
  return;
}
