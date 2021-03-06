#include "TH2D.h"
#include "THnSparse.h"
#include "TCanvas.h"
#include "TList.h"
#include "TString.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMath.h"
#include "TObjArray.h"
#include "THnSparse.h"
#include "TSpline.h"

#include <iostream>
#include <iomanip>

//__________________________________________________________________________________________
TSpline3* loadSplines(TFile* file, TObjArray* arr, TString name, Bool_t useOADB)
{
  if (useOADB) {
    if (!arr)
      return 0x0;
    
    return (TSpline3*)arr->FindObject(name.Data());
  }
  
  if (!file)
    return 0x0;
  
  return (TSpline3*)file->Get(name.Data());
}



//__________________________________________________________________________________________
Int_t compareSplines2(TString pathNameSplines1, TString pathNameSplines2,
                     TString period = "LHC10D_PASS2", TString dataType1 = "DATA", TString beamType1 = "PP",
                     TString period2 = "", TString dataType2 = "", TString beamType2 = "",
                     Bool_t useOADBforFirstSplines = kFALSE, Bool_t useOADBforSecondSplines = kFALSE,
                     TString displayNameSplines1 = "", TString displayNameSplines2 = "", TString outputPath = "", TString fileFormat = "pdf", Double_t ybinLow = 0.96, Double_t ybinHigh = 1.04, TString multBinString = "")
{ 
  
  if (!multBinString.IsNull()) {
    TObjArray* arr = 0x0;
    
    TObjArray* multBins = multBinString.Tokenize(":");
    TString fileNameBase = TString(pathNameSplines1).ReplaceAll(".root","");
    
    TCanvas* c = new TCanvas("c", "",  100,10,1380,800);
    c->SetLogx(kTRUE);
    
    TH2D* hDummy = new TH2D("hDummy", Form("; p_{TPC} (GeV/c); %s / %s", "Saturated Lund", "ALEPH"),
                            1000, 0.15, 60, 1000, 0.95, 1.05);
    hDummy->GetYaxis()->SetLabelSize(0.03);
    hDummy->GetYaxis()->SetTitleSize(0.05);
    hDummy->GetYaxis()->SetTitleOffset(1.);
    hDummy->GetYaxis()->SetRangeUser(ybinLow, ybinHigh);
    hDummy->GetXaxis()->SetNoExponent(kTRUE);
    hDummy->GetXaxis()->SetMoreLogLabels(kTRUE);
    hDummy->GetXaxis()->SetTitleSize(0.05);
    hDummy->GetXaxis()->SetLabelSize(0.04);
    hDummy->GetXaxis()->SetTitleOffset(1.1);
    hDummy->SetStats(kFALSE);
    hDummy->Draw("colz");    

    const Int_t nPoints = 20000 * 10;
    const Float_t stepSize = 0.02 * 100;

    TObjString* multLow = (TObjString*)multBins->At(0);
    TObjString* multHigh = (TObjString*)multBins->At(1);
    TFile* f = new TFile(TString::Format("%s_%s_%s.root",fileNameBase.Data(), multLow->GetName(), multHigh->GetName()));    
    TSpline3* splPion2 = loadSplines(f, arr, Form("TSPLINE3_%s_ELECTRON_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), useOADBforFirstSplines); 
    
    Int_t colors[3] = {kMagenta, kMagenta + 2, kMagenta + 4};
    TLegend* legend = new TLegend(0.1, 0.9, 0.4, 0.7);
    for (Int_t i=2;i<multBins->GetEntries();i=i+2) {
      TObjString* multLow = (TObjString*)multBins->At(i);
      TObjString* multHigh = (TObjString*)multBins->At(i+1);
      TFile* f = new TFile(TString::Format("%s_%s_%s.root",fileNameBase.Data(), multLow->GetName(), multHigh->GetName()));
      TSpline3* splPion = loadSplines(f, arr, Form("TSPLINE3_%s_ELECTRON_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), useOADBforFirstSplines);
      TGraph* gPion = new TGraph(nPoints);
      gPion->SetTitle("#pi");
      gPion->SetFillStyle(0);
      gPion->SetFillColor(kWhite);
      for (Int_t j = 0; j < nPoints; j++) {
        gPion->SetPoint(j, (0. + j * stepSize) * 0.000511, (splPion->Eval((0. + j * stepSize)) / splPion2->Eval((0. + j * stepSize))));
      }
      gPion->SetLineColor(colors[i/2-1]);
      gPion->SetMarkerColor(colors[i/2-1]);
      gPion->Draw(i==0 ? "" : "same");
      legend->AddEntry(gPion, TString::Format("%s_%s",multLow->GetName(), multHigh->GetName()));
    }
    legend->Draw("same");
    c->Update();
    c->SaveAs(Form("%s/CompareSplines%s%s%s_Kaons.%s",outputPath.Data(),period.Data(),displayNameSplines1.Data(),displayNameSplines2.Data(),fileFormat.Data()));   
    return 0;
  }
  
  if (period2.IsNull())
    period2 = period;
  
  if (dataType2.IsNull())
    dataType2 = dataType1;
  
  if (beamType2.IsNull())
    beamType2 = beamType1;
  
  if (displayNameSplines1.IsNull())
    displayNameSplines1 = "Spline 1";
  
  if (displayNameSplines2.IsNull())
    displayNameSplines2 = "Spline 2";
  
  TFile* f1 = 0x0;
  f1 = TFile::Open(pathNameSplines1.Data());
  if (!f1)  {
    std::cout << "Failed to open file \"" << pathNameSplines1.Data() << "\"!" << std::endl;
    return -1;
  }
  
  TFile* f2 = 0x0;
  f2 = TFile::Open(pathNameSplines2.Data());
  if (!f2)  {
    std::cout << "Failed to open file \"" << pathNameSplines2.Data() << "\"!" << std::endl;
    return -1;
  }

  TCanvas* c = new TCanvas("c", "",  100,10,1380,800);
  c->SetLogx(kTRUE);
  
  TH2D* hDummy = new TH2D("hDummy", Form("; p_{TPC} (GeV/c); %s / %s", "Saturated Lund (w D.)", "ALEPH (w D.)"),
                          1000, 0.15, 60, 1000, 0.95, 1.05);
  hDummy->GetYaxis()->SetLabelSize(0.03);
  hDummy->GetYaxis()->SetTitleSize(0.05);
  hDummy->GetYaxis()->SetTitleOffset(1.);
  hDummy->GetYaxis()->SetRangeUser(ybinLow, ybinHigh);
  hDummy->GetXaxis()->SetNoExponent(kTRUE);
  hDummy->GetXaxis()->SetMoreLogLabels(kTRUE);
  hDummy->GetXaxis()->SetTitleSize(0.05);
  hDummy->GetXaxis()->SetLabelSize(0.04);
  hDummy->GetXaxis()->SetTitleOffset(1.1);
  hDummy->SetStats(kFALSE);
  hDummy->Draw("colz");
  
  const Int_t nPoints = 20000;
  const Float_t stepSize = 0.02;
  const Float_t stepSizeEl = stepSize * 100.;
  const Int_t nPointsEl = nPoints * 10.;
  
  
  
  TSpline3* splPion2 = 0x0;
  TSpline3* splKaon2 = 0x0;
  TSpline3* splElectron2 = 0x0;
  TSpline3* splProton2 = 0x0;
  TSpline3* splDefault2 = 0x0;
  
  
  TObjArray* arr = 0x0;
  TObjArray* arr2 = 0x0;
  
  if (useOADBforFirstSplines) {
    arr = (TObjArray*)f1->Get("TPCPIDResponse");
    
    if (!arr)  {
      std::cout << "Failed to load first array \"TPCPIDResponse\"!" << std::endl;
      return -1;
    }
  }
  
  if (useOADBforSecondSplines) {
    arr2 = (TObjArray*)f2->Get("TPCPIDResponse");
    
    if (!arr2)  {
      std::cout << "Failed to second array \"TPCPIDResponse\"!" << std::endl;
      return -1;
    }
  }
  
  TSpline3* splPion = loadSplines(f1, arr, Form("TSPLINE3_%s_PION_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), useOADBforFirstSplines);
  splPion2 = loadSplines(f2, arr2, Form("TSPLINE3_%s_PION_%s_%s_MEAN", dataType2.Data(), period2.Data(), beamType2.Data()), useOADBforSecondSplines);
  TGraph* gPion = new TGraph(nPoints);
  gPion->SetTitle("#pi");
  gPion->SetFillStyle(0);
  gPion->SetFillColor(kWhite);
  for (Int_t i = 0; i < nPoints; i++) {
    gPion->SetPoint(i, (0. + i * stepSize) * 0.1396, (splPion->Eval((0. + i * stepSize)) / splPion2->Eval((0. + i * stepSize))));
  }
  gPion->SetLineColor(kRed);
  gPion->SetMarkerColor(kRed);
  gPion->Draw("same");

  TSpline3* splProton = loadSplines(f1, arr, Form("TSPLINE3_%s_PROTON_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), 
                                    useOADBforFirstSplines);
  splProton2 = loadSplines(f2, arr2, Form("TSPLINE3_%s_PROTON_%s_%s_MEAN", dataType2.Data(), period2.Data(), beamType2.Data()), useOADBforSecondSplines);
  TGraph* gProton = new TGraph(nPoints);
  gProton->SetTitle("p");
  gProton->SetFillStyle(0);
  gProton->SetFillColor(kWhite);
  for (Int_t i = 0; i < nPoints; i++) {
    gProton->SetPoint(i, (0. + i * stepSize) * 0.938, (splProton->Eval((0. + i * stepSize)) / splProton2->Eval((0. + i * stepSize))));
  }
  gProton->SetLineColor(kBlue);
  gProton->SetMarkerColor(kBlue);
  gProton->Draw("same");

  TSpline3* splKaon = loadSplines(f1, arr, Form("TSPLINE3_%s_KAON_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), useOADBforFirstSplines);
  splKaon2 = loadSplines(f2, arr2, Form("TSPLINE3_%s_KAON_%s_%s_MEAN", dataType2.Data(), period2.Data(), beamType2.Data()), useOADBforSecondSplines);
  TGraph* gKaon = new TGraph(nPoints);
  gKaon->SetTitle("K");
  gKaon->SetFillStyle(0);
  gKaon->SetFillColor(kWhite);
  for (Int_t i = 0; i < nPoints; i++) {
    gKaon->SetPoint(i, (0. + i * stepSize) * 0.49368, (splKaon->Eval((0. + i * stepSize)) / splKaon2->Eval((0. + i * stepSize))));
  }
  gKaon->SetLineColor(kGreen);
  gKaon->SetMarkerColor(kGreen);
  gKaon->Draw("same");

  TSpline3* splElectron = loadSplines(f1, arr, Form("TSPLINE3_%s_ELECTRON_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), 
                                      useOADBforFirstSplines);
  splElectron2 = loadSplines(f2, arr2, Form("TSPLINE3_%s_ELECTRON_%s_%s_MEAN", dataType2.Data(), period2.Data(), beamType2.Data()),
                             useOADBforSecondSplines);
  TGraph* gElectron = new TGraph(nPoints);
  gElectron->SetTitle("e");
  gElectron->SetFillStyle(0);
  gElectron->SetFillColor(kWhite);
  for (Int_t i = 0; i < nPointsEl; i++) {
    gElectron->SetPoint(i, (0. + i * stepSizeEl) * 0.000511, 
                        (splElectron->Eval((0. + i * stepSizeEl)) / splElectron2->Eval((0. + i * stepSizeEl))));
  }
  gElectron->SetLineColor(kMagenta);
  gElectron->SetMarkerColor(kMagenta);
  gElectron->Draw("same");

  TSpline3* splDefault = loadSplines(f1, arr, Form("TSPLINE3_%s_ALL_%s_%s_MEAN", dataType1.Data(), period.Data(), beamType1.Data()), 
                                      useOADBforFirstSplines);
  splDefault2 = loadSplines(f2, arr2, Form("TSPLINE3_%s_ALL_%s_%s_MEAN", dataType2.Data(), period2.Data(), beamType2.Data()),
                             useOADBforSecondSplines);
  TGraph* gDefault = new TGraph(nPoints);
  gDefault->SetTitle("default (proton mass)");
  gDefault->SetFillStyle(0);
  gDefault->SetFillColor(kWhite);
  for (Int_t i = 0; i < nPoints; i++) {
    gDefault->SetPoint(i, (0. + i * stepSize) * 0.938, 
                        (splDefault->Eval((0. + i * stepSize)) / splDefault2->Eval((0. + i * stepSize))));
  }
  gDefault->SetLineColor(kBlack);
  gDefault->SetMarkerColor(kBlack);
  gDefault->Draw("same");
  
  TLegend* leg = c->BuildLegend();
  leg->GetListOfPrimitives()->RemoveAt(0);
  leg->SetFillColor(kWhite);
  c->SetGridx(kTRUE);
  c->SetGridy(kTRUE);
  
  c->Update();
  c->SaveAs(Form("%s/CompareSplines%s%s%s.%s",outputPath.Data(),period.Data(),displayNameSplines1.Data(),displayNameSplines2.Data(),fileFormat.Data()));
  return 0;
}
