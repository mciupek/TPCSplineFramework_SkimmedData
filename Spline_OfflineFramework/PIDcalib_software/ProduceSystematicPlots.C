// Bins always 1 higher
// MCID 0 = Electron (TPC < 0.35, TPC+TOF > 0.3)
// MCID 1 = Kaon (TPC < 0.35, TPC+TOF > 0.35)
// MCID 2 = Not used
// MCID 3 = Pion (TPC < 0.3, TPC+TOF > 0.3)
// MCID 4 = Proton (TPC < 0.6, TPC+TOF > 0.6)
// MCID 5 = V0plusTOF electron
// MCID 6 = V0 electron
// MCID 7 = V0 Pion
// MCID 8 = V0 Proton
// 
// Select Species
// 0: Electrons
// 1: Kaons
// 2: Pions 
// 3: Protons   

const Double_t heightFractionForFittingRange = 0.2;

const Int_t nOfSamples = 8;
const Int_t mcIDarray[nOfSamples] = {5, 8, 2, 5, 2, 5, 2, 8};
const Int_t speciesarray[nOfSamples] = {4, 3, 2, 4, 2, 4, 2, 3};
const Double_t pTPCArray[nOfSamples] = {0.25, 0.3, 0.38, 0.6, 0.6, 0.9, 1.7, 3.0};
TString legendNames[nOfSamples] = {"TPC p", "V0 #pi", "TPC+TOF K", "TPC+TOF p", "TPC+TOF K", "TPC+TOF p", "TPC+TOF K", "V0 #pi"};
const Int_t speciescolor[nOfSamples] = {kBlue, kRed, kGreen, kBlue-7, kGreen, kBlue, kGreen-2, kRed};
const Int_t usedEtaCanvases = 2;
const Int_t cut[usedEtaCanvases+1] = {0,4,nOfSamples};
TString canvasNames[usedEtaCanvases] = { "cEtaSystematicslowP", "cEtaSystematicshighP" };

void ProduceSystematicPlots(TString file, TString softwarepath = "") {
  gStyle->SetOptStat("");
  TFile *f = new TFile(file.Data(),"OPEN");
  TSubString path = file(0,file.Last(*((TString("/").Data()))));
  TCanvas* c = 0x0;
  TCanvas** carr = 0x0;

  if (file.Contains("splines_QA")) {
    c = (TCanvas*)f->FindObjectAny("canvDelta_2");
    if (!c) 
      return;
    if (file.Contains("SaturatedLund"))
      c->SetName("Residuals_SaturatedLund");
    else if (file.Contains("_Lund"))
      c->SetName("Residuals_Lund");
    else if (file.Contains("ModifiedALEPH"))
      c->SetName("Residuals_ModifiedALEPH");
    else if (file.Contains("_ALEPH"))
      c->SetName("Residuals_ALEPH");
    
  }
  else if (file.Contains("CompareSplines")) {
    c = (TCanvas*)f->FindObjectAny("c");
    if (!c)
      return;
    
    if (file.Contains("LundSaturatedLund"))
      c->SetName("Comparison_Lund_SaturatedLund");
    else if (file.Contains("ModifiedALEPH"))
      c->SetName("Comparison_ModifiedALEPH_SaturatedLund");
    else if (file.Contains("ALEPH"))
      c->SetName("Comparison_ALEPH_SaturatedLund");
  }
  else if (file.Contains("TPCPIDEtaQA.root")) {
    gROOT->LoadMacro((softwarepath + TString("/checkShapeEta.C+")).Data());
    Double_t results[4] = {0.0, 0.0, 0.0, 0.0 };
    Double_t resultErrors[4] = {0.0, 0.0, 0.0, 0.0 };    
// 
    TObjArray* arr = (TObjArray*)f->FindObjectAny("TPCPIDEtaQA");
    THnSparse* hPIDdataAll = (THnSparse*)arr->FindObject("hPIDdataAll");
    //Restrict Multiplicity
    hPIDdataAll->GetAxis(3)->SetRangeUser(0,499);
    
    TLegend* legend = 0x0;    
    
    carr = new TCanvas*[usedEtaCanvases];
    
    TAxis* etaaxe = hPIDdataAll->GetAxis(5);
    Int_t nEtaBins = etaaxe->GetNbins();
    const Float_t* etabins = new Float_t[nEtaBins+1];  
    
    for (Int_t i=0;i<=nEtaBins;i++) {
      etabins[i]=etaaxe->GetBinLowEdge(i+1);
    }

    TH1F** deltaPrimeEta = new TH1F*[nOfSamples];
    
    for (Int_t j=0;j<usedEtaCanvases;j++) {
      carr[j] = new TCanvas(canvasNames[j].Data(),"");
      legend = 0x0;
      legend = new TLegend(0.1, 0.9, 0.4, 0.7); 
      for (Int_t i=cut[j];i<cut[j+1];i++) {
        deltaPrimeEta[i] =  new TH1F(TString::Format("deltaPrimeEta%i,i").Data(), "", nEtaBins, etabins);
        deltaPrimeEta[i]->SetLineColor(speciescolor[i]);
        deltaPrimeEta[i]->SetMarkerColor(speciescolor[i]);
        deltaPrimeEta[i]->SetLineWidth(2);
         
        hPIDdataAll->GetAxis(0)->SetRange(mcIDarray[i],mcIDarray[i]);
        hPIDdataAll->GetAxis(1)->SetRange(speciesarray[i],speciesarray[i]);
        Int_t bin = hPIDdataAll->GetAxis(2)->FindFixBin(pTPCArray[i]);    
        if (bin == 0 || bin == (hPIDdataAll->GetAxis(2)->GetNbins())+1)
          continue;
        
        Double_t plow,phigh;
        if (hPIDdataAll->GetAxis(2)->GetBinCenter(bin) < pTPCArray[i]) {
          hPIDdataAll->GetAxis(2)->SetRange(bin,bin+1);
          plow = hPIDdataAll->GetAxis(2)->GetBinLowEdge(bin);
          phigh = hPIDdataAll->GetAxis(2)->GetBinUpEdge(bin+1);
        }
        else {
          hPIDdataAll->GetAxis(2)->SetRange(bin-1,bin);
          plow = hPIDdataAll->GetAxis(2)->GetBinLowEdge(bin);
          phigh = hPIDdataAll->GetAxis(2)->GetBinUpEdge(bin+1);
        }
        for (Int_t l=0;l<=nEtaBins;l++) {
          hPIDdataAll->GetAxis(5)->SetRange(l,l);
          TH1D* hMeandEdx = hPIDdataAll->Projection(4);
          FitHist(hMeandEdx, heightFractionForFittingRange, "QN", results, resultErrors);  // Defined in checkShapeEta.C
          deltaPrimeEta[i]->SetBinContent(l,results[1]);
          deltaPrimeEta[i]->SetBinError(l,resultErrors[1]);
        }
        gStyle->SetOptStat("");
        deltaPrimeEta[i]->GetYaxis()->SetRangeUser(0.95, 1.05);
        deltaPrimeEta[i]->Draw(i==0 ? "" : "same");
        legend->AddEntry(deltaPrimeEta[i],legendNames[i]+ TString::Format(", %.2f GeV/#it{c} < #it{p}_{TPC} < %.2f GeV/#it{c} ",plow,phigh));
      }
      legend->Draw("same");
    }
  }
  else if (file.Contains("TPCresidualPID")) {
    const Int_t rescolors[4] = {kRed,kGreen,kBlue,kMagenta};
    TString legendNamesRes[4] = {"#pi, TOF+TPC" , "K, TOF+TPC", "p, TOF+TPC", "e, V0+TPC" };
    TCanvas* c = new TCanvas("cResolutionSystematics");
    TF1* func = new TF1("func","gaus");
    //Restrict fit range to +/- 1.5 TPC sigma
    func->SetRange(-1.5.,1.5.);
    TObjArray* arr = f->FindObjectAny("TPCresPID");
    THnSparse* fHistPidQA = (THnSparse*)arr->FindObject("fHistPidQA");
    //Restrict pT
    Double_t minpT = 0.15;
    Double_t maxpT = 10.0;
    fHistPidQA->GetAxis(0)->SetRangeUser(minpT,maxpT);
    Int_t lowbin = fHistPidQA->GetAxis(0)->FindFixBin(minpT);
    Int_t upbin = fHistPidQA->GetAxis(0)->FindFixBin(maxpT);
    TH2D** hMeanSigmaP = new TH2D*[4];
    TH1D** hMeanStdDevP = new TH1D*[4];
    
    TLegend* legend = new TLegend(0.1, 0.9, 0.4, 0.7); 
    
    //TOF particles, set TOF cut
    fHistPidQA->GetAxis(5)->SetRange(3,8);
    //Set particle identification to non-V0s
    fHistPidQA->GetAxis(2)->SetRange(1,1);
    
    for (Int_t i=0;i<=2;++i) {
      fHistPidQA->GetAxis(3)->SetRange(i+2,i+2);
      hMeanSigmaP[i] = fHistPidQA->Projection(4,0);
      hMeanSigmaP[i]->SetNameTitle(Form("%i",i),"");
      hMeanSigmaP[i]->FitSlicesY(func,lowbin,upbin);
      hMeanStdDevP[i] = (TH1D*)gDirectory->Get(Form("%s_2",hMeanSigmaP[i]->GetName()));
      hMeanStdDevP[i]->SetLineColor(rescolors[i]);
      hMeanStdDevP[i]->SetTitle(";#it{p} (GeV/#it{c};width(n#sigma)");
      hMeanStdDevP[i]->GetYaxis()->SetTitleOffset(1.1);
      hMeanStdDevP[i]->SetMarkerColor(rescolors[i]);
      hMeanStdDevP[i]->SetLineWidth(2);
      hMeanStdDevP[i]->GetYaxis()->SetRangeUser(0.9,1.1);
      hMeanStdDevP[i]->GetXaxis()->SetRangeUser(0.15,3.1);
      hMeanStdDevP[i]->Draw(i==0 ? "" : "same");
      legend->AddEntry(hMeanStdDevP[i],legendNamesRes[i]);
    }
    
    //V0 electrons
    Int_t i=3;
    fHistPidQA->GetAxis(5)->SetRange(-1,-1);
    fHistPidQA->GetAxis(2)->SetRange(2,2);
    fHistPidQA->GetAxis(3)->SetRange(1,1);
    hMeanSigmaP[i] = fHistPidQA->Projection(4,0);
    hMeanSigmaP[i]->SetNameTitle(Form("%i",i),"");
    hMeanSigmaP[i]->FitSlicesY(func,lowbin,upbin);
    hMeanStdDevP[i] = (TH1D*)gDirectory->Get(Form("%s_2",hMeanSigmaP[i]->GetName()));
    hMeanStdDevP[i]->SetLineColor(rescolors[i]);
    hMeanStdDevP[i]->SetMarkerColor(rescolors[i]);
    hMeanStdDevP[i]->SetLineWidth(2);    
    hMeanStdDevP[i]->Draw("same"); 
    legend->AddEntry(hMeanStdDevP[i],legendNamesRes[i]);
    legend->Draw("same");
  }
  
  TFile* sysFile = new TFile((path + TString("/SplineSystematics.root")).Data(),"UPDATE");
  gStyle->SetOptStat("");
  if (c)
    c->Write(0x0,TObject::kOverwrite);
  
  if (carr) {
    for (Int_t i=0;i<usedEtaCanvases;i++)
      carr[i]->Write(0x0,TObject::kOverwrite);
  }
  
  sysFile->Close();
  f->Close();

  return; 
}

//     Double_t pT = 1.7;
//     Int_t bin = hPIDdataAll->GetAxis(2)->FindFixBin(pT);
//     
//     if (hPIDdataAll->GetAxis(2)->GetBinCenter(bin) < pT) 
//       hPIDdataAll->GetAxis(2)->SetRange(bin,bin+1);
//     else
//       hPIDdataAll->GetAxis(2)->SetRange(bin-1,bin);    
//     
//     hPIDdataAll->GetAxis(0)->SetRange(2,2);
//     hPIDdataAll->GetAxis(1)->SetRange(2,2);
//     
//     TCanvas* cl = new TCanvas("cl","dummy");
//     cl->cd();
//     hPIDdataAll->Projection(4)->Draw();

