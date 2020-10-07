#include "THnSparse.h"
#include "TH3D.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TVirtualFitter.h"
#include "TList.h"
#include "TString.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TMath.h"
#include "TDatime.h"

#include <iostream>
#include <iomanip>

#include "THnSparseDefinitions.h"

Double_t getMedianOfNonZeros(Double_t* input, const Int_t dim)
{
	Double_t values[dim];
	for (Int_t i = 0; i < dim; i++)
		values[i] = 0.0;
	
	Int_t numNonZero = 0;
	
	for (Int_t i = 0; i < dim; i++) {
		if (input[i] > 0) {
			values[numNonZero] = input[i];
			numNonZero++;
		}
	}
	
	return ((numNonZero > 0) ? TMath::Median(numNonZero, values) : 0);
}


//--------------------------------------------------
void prepareHisto(TH2* h)
{
	for (Int_t binX = 1; binX <= h->GetXaxis()->GetNbins(); binX++) {
		
		// Find maximum for fixed x
		Double_t maxBinContent = -1;
		Int_t maxBin = -1;
		for (Int_t binY = 1; binY <= h->GetYaxis()->GetNbins(); binY++) {
			Double_t cont = h->GetBinContent(binX, binY);
			if (cont > maxBinContent) {
				maxBinContent = cont;
				maxBin = binY;
			}
		}
		
		Double_t prevBinContent = -1;
		Double_t currBinContent = -1;
		
		// Start at the maximum, go to the left and remove all bins that fall too steeply (due to TPC cut) by setting large errors
		for (Int_t binY = maxBin; binY >= 1; binY--) {
			currBinContent = h->GetBinContent(binX, binY);
			
			if (currBinContent > 0)   {
				Double_t ratio = prevBinContent / currBinContent;
				
				if (ratio > 5)    {
					for (Int_t binY2 = binY; binY2 >= 1; binY2--) {
						if (h->GetBinContent(binX, binY2) > 0) {
							h->SetBinContent(binX, binY2, 0);
							h->SetBinError(binX, binY2, maxBinContent);
						}
					}
					break;
				}
			}
			
			prevBinContent = currBinContent;
		}
		
		prevBinContent = -1;
		currBinContent = -1;
		
		// Start at the maximum, go to the right and remove all bins that fall too steeply (due to TPC cut) by setting large errors
		for (Int_t binY = maxBin; binY <= h->GetYaxis()->GetNbins(); binY++) {
			currBinContent = h->GetBinContent(binX, binY);
			
			if (currBinContent > 0)   {
				Double_t ratio = prevBinContent / currBinContent;
				
				if (ratio > 5)    {
					for (Int_t binY2 = binY; binY2 <= h->GetYaxis()->GetNbins(); binY2++) {
						if (h->GetBinContent(binX, binY2) > 0) {
							h->SetBinContent(binX, binY2, 0);
							h->SetBinError(binX, binY2, maxBinContent);
						}
					}
					break;
				}
			}
			
			prevBinContent = currBinContent;
		}    
	}
}


//--------------------------------------------------
Bool_t FitHist(TH1 *h, Double_t heightFractionForRange, TString fitOption, Double_t *results, Double_t *resultErrors)
{
	if (!h) return kFALSE;
	if (!results || !resultErrors) return kFALSE;
	
	// Average around maximum bin -> Might catch some outliers
	Int_t maxBin = h->GetMaximumBin();
	Double_t maxVal = h->GetBinContent(maxBin);
	
	if (maxVal < 2) { // It could happen that all entries are in overflow/underflow; don't fit in this case
		return kFALSE;
	}
	
	UChar_t usedBins = 1;
	if (maxBin > 1) {
		maxVal += h->GetBinContent(maxBin - 1);
		usedBins++;
	}
	if (maxBin < h->GetNbinsX()) {
		maxVal += h->GetBinContent(maxBin + 1);
		usedBins++;
	}
	maxVal /= usedBins;
	
	Double_t thresholdFraction = heightFractionForRange * maxVal; 
	Int_t lowThrBin = TMath::Max(1, h->FindFirstBinAbove(thresholdFraction));
	Int_t highThrBin = TMath::Min(h->GetNbinsX(), h->FindLastBinAbove(thresholdFraction));
	
	Double_t lowThreshold = h->GetBinCenter(lowThrBin);
	Double_t highThreshold = h->GetBinCenter(highThrBin);
	
	TFitResultPtr res = h->Fit("gaus", Form("%sS", fitOption.Data()), "", lowThreshold, highThreshold);
	
	if ((Int_t)res == 0) {
		for (Int_t i = 0; i < 3; i++) {
			results[i] = res->GetParams()[i];
			resultErrors[i] = res->GetErrors()[i];
		}
		results[3] = res->Ndf()>0 ? res->Chi2()/res->Ndf() : 0;
		resultErrors[3] = 0;
		
		return kTRUE;
	}
	
	return kFALSE;
}


void FitSlicesY(TH2 *hist, Double_t heightFractionForRange, Int_t cutThreshold, TString fitOption, TObjArray *arr)
{
	if (!hist) return;
	if (!arr) return;
	
	
	arr->Clear();
	arr->SetOwner();
	arr->Expand(4);
	
	TAxis *axis=hist->GetXaxis();
	const TArrayD *bins = axis->GetXbins();
	TH1D** hList = new TH1D*[4];
	
	for (Int_t i = 0; i < 4; i++) {
		delete gDirectory->FindObject(Form("%s_%d", hist->GetName(), i));
		
		if (bins->fN == 0) {
			hList[i] = new TH1D(Form("%s_%d", hist->GetName(), i), i < 3 ? Form("Fitted value of par[%d]", i) : "Chi2/NDF",
									  hist->GetNbinsX(), axis->GetXmin(), axis->GetXmax());
		} else {
			hList[i] = new TH1D(Form("%s_%d", hist->GetName(), i), i < 3 ? Form("Fitted value of par[%d]", i) : "Chi2/NDF", hist->GetNbinsX(), bins->fArray);
		}
		
		(*arr)[i] = hList[i];
	}
	
	Double_t results[4] = {0.0, 0.0, 0.0, 0.0 };
	Double_t resultErrors[4] = {0.0, 0.0, 0.0, 0.0 };
	
	TCanvas * hcanv = new TCanvas("hcanv","hcanv",10,10,1000,1100) ;
	
	for (Int_t ibin=0, ibin2=axis->GetFirst(); ibin2<=axis->GetLast(); ++ibin2, ++ibin) {
		TH1 *h=hist->ProjectionY("_temp",ibin,ibin2);
		if (!h)
			continue;
		
		if (h->GetEntries() < cutThreshold) {
			delete h;
			continue;
		}
		cout << __LINE__ << endl;
		h->Draw();
		hcanv->SaveAs(Form("histPi_%d.pdf",ibin));
		Bool_t fitSuccessful = FitHist(h, heightFractionForRange, fitOption, results, resultErrors);
		
		if (fitSuccessful) {
			cout << __LINE__ << endl;
			Int_t resBin = ibin2;
			hList[0]->SetBinContent(resBin,results[0]);
			hList[0]->SetBinError(resBin,resultErrors[0]);
			hList[1]->SetBinContent(resBin,results[1]);
			hList[1]->SetBinError(resBin,resultErrors[1]);
			hList[2]->SetBinContent(resBin,results[2]);
			hList[2]->SetBinError(resBin,resultErrors[2]);
			hList[3]->SetBinContent(resBin,results[3]);
			cout << __LINE__ << endl;
// 				cout << results[0] << "\t" << results[1] << "\t" << results[2] << "\t" << results[3] << endl;
		}
		
		delete h;
	}
}

TH1D* getSeparationHist(THnSparse* hist, Bool_t fromV0s, TFile* fSave)
{
	TObjArray arr;
	
	Int_t binIDel = 1;
	if (fromV0s)
		binIDel = 7;
	
	Int_t binIDpi = 4;
	if (fromV0s)
		binIDpi = 8;
	
	//hist->GetAxis(kEta)->SetRangeUser(-0.199, 0.199);
	
	// Select and fit electrons
	hist->GetAxis(kMCpid)->SetRange(binIDel,binIDel); // (V0) electrons
	hist->GetAxis(kSelectSpecies)->SetRange(1,1); // Delta_electron
	
	Int_t referenceAxis = 0/*kDeDx*/;
	
	TH2D* histEl = (TH2D*)hist->Projection(referenceAxis, kPtpcInner);
	histEl->SetName(fromV0s ? "histV0El" : "histEl");
	histEl->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	histEl->GetYaxis()->SetTitle(hist->GetAxis(referenceAxis)->GetTitle());
	
	  if (!fromV0s)
	    prepareHisto(histEl);
// 	histEl->FitSlicesY(0,0,-1,0,"R",&arr); 
	  
	  TCanvas * RatioCanvas = new TCanvas("RatioCanvas","RatioCanvas",10,10,1000,1100) ;
// 		RatioCanvas->SetRightMargin(0.00);
// 		RatioCanvas->SetTopMargin(0.00);
// 		RatioCanvas->SetLeftMargin(0.00);
// 		RatioCanvas->SetBottomMargin(0.00);
		RatioCanvas->SetLogx();
		RatioCanvas->SetLogz();
		histEl->Draw("colz");
		RatioCanvas->SaveAs("histEl.pdf");
	  
	  
	  FitSlicesY(histEl,1.3,0,"QNR",&arr);
	TH1D* hMeanEl = (TH1D*)arr.At(1)->Clone(fromV0s ? "hMeanV0El" : "hMeanEl");
// 	for (int i=0;i<hMeanEl->GetNbinsX();i++) cout << hMeanEl->GetBinContent(i) << endl;
	TH1D* hSigmaEl = (TH1D*)arr.At(2)->Clone(fromV0s ? "hSigmaV0El" : "hSigmaEl");
	TH1D* hChi2El = (TH1D*)arr.At(3)->Clone(fromV0s ? "hChi2V0El" : "hChi2El");
	
	hMeanEl->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hMeanEl->GetYaxis()->SetTitle("Mean (Gauss)");
	hSigmaEl->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hSigmaEl->GetYaxis()->SetTitle("#sigma (Gauss)");
	hChi2El->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hChi2El->GetYaxis()->SetTitle("#chi^{2}/NDF (Gauss)");
	
	hMeanEl->SetMarkerStyle(20);
	hSigmaEl->SetMarkerStyle(20);
	
	hMeanEl->Draw();
	RatioCanvas->SaveAs("histMeanEl.pdf");
	hSigmaEl->Draw();
	RatioCanvas->SaveAs("histSigmaEl.pdf");
	hChi2El->Draw();
	RatioCanvas->SaveAs("histChi2El.pdf");

	if (fSave)    {
		fSave->cd();
		
		histEl->Write();
	}
	
	delete histEl;
	
	// Select and fit pions
	hist->GetAxis(kMCpid)->SetRange(binIDpi,binIDpi); // (V0) pions
	hist->GetAxis(kSelectSpecies)->SetRange(3,3); // Delta_pion
	
	TH2D* histPi = (TH2D*)hist->Projection(referenceAxis, kPtpcInner);
	histPi->SetName(fromV0s ? "histV0Pi" : "histPi");
	histPi->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	histPi->GetYaxis()->SetTitle(hist->GetAxis(referenceAxis)->GetTitle());
	
	  if (!fromV0s)
	    prepareHisto(histPi);
	  
		histPi->Draw("colz");
		RatioCanvas->SaveAs("histPi.pdf");
		
//	histPi->FitSlicesY(0,0,-1,0,"QNR",&arr);
// 	FitSlicesY(histPi,2.3,0,"QNR",&arr);
	TH1D* hMeanPi = (TH1D*)arr.At(1)->Clone(fromV0s ? "hMeanV0Pi" : "hMeanPi");
	TH1D* hSigmaPi = (TH1D*)arr.At(2)->Clone(fromV0s ? "hSigmaV0Pi" : "hSigmaPi");
	TH1D* hChi2Pi = (TH1D*)arr.At(3)->Clone(fromV0s ? "hChi2V0Pi" : "hChi2Pi");
	
	hMeanPi->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hMeanPi->GetYaxis()->SetTitle("Mean (Gauss)");
	hSigmaPi->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hSigmaPi->GetYaxis()->SetTitle("#sigma (Gauss)");
	hChi2Pi->GetXaxis()->SetTitle(hist->GetAxis(kPtpcInner)->GetTitle());
	hChi2Pi->GetYaxis()->SetTitle("#chi^{2}/NDF (Gauss)");
	
	hMeanPi->SetMarkerStyle(20);
	hMeanPi->SetLineColor(kRed);
	hMeanPi->SetMarkerColor(kRed);
	hSigmaPi->SetMarkerStyle(20);
	hSigmaPi->SetLineColor(kRed);
	hSigmaPi->SetMarkerColor(kRed);
	hChi2Pi->SetMarkerStyle(20);
	hChi2Pi->SetLineColor(kRed);
	hChi2Pi->SetMarkerColor(kRed);
	
	hMeanPi->Draw();
	RatioCanvas->SaveAs("histMeanPi.pdf");
	hSigmaPi->Draw();
	RatioCanvas->SaveAs("histSigmaPi.pdf");
	hChi2Pi->Draw();
	RatioCanvas->SaveAs("histChi2Pi.pdf");
	
	// Separation
	
	TH1D* hSeparation= (TH1D*)hMeanEl->Clone(fromV0s ? "hSeparationV0" : "hSeparation"); //to get same binning
	hSeparation->SetMarkerStyle(20);
	hSeparation->GetYaxis()->SetTitle("Separation");
	
	const Int_t nBins = hMeanEl->GetNbinsX();
	
	Double_t deltaMean[nBins] ;
	Double_t deltaSigma[nBins];
	
	for(Int_t i = 0 ;i < nBins; i++) {
		deltaMean[i] = TMath::Abs(hMeanEl->GetBinContent(i) - hMeanPi->GetBinContent(i));
		deltaSigma[i] = TMath::Abs((hSigmaEl->GetBinContent(i) + hSigmaPi->GetBinContent(i))) / 2.;
		
		if(TMath::Abs(deltaSigma[i]) < 0.000001)
			continue;
		
		hSeparation->SetBinContent(i, deltaMean[i] / deltaSigma[i]);
	}
	
	
	// Reset ranges
	hist->GetAxis(kMCpid)->SetRange(0,-1); 
	hist->GetAxis(kSelectSpecies)->SetRange(0,-1);
	hist->GetAxis(kEta)->SetRange(0, -1);
	
	if (fSave)    {
		fSave->cd();
		
		histPi->Write();
		
		hMeanEl->Write();
		hMeanPi->Write();
		
		hSigmaEl->Write();
		hSigmaPi->Write();
		
		hChi2El->Write();
		hChi2Pi->Write();
		
		hSeparation->Write();
	}
	
	delete histPi;
	
	return hSeparation;
}



void getSeparation(TString path = "/hera/alice/maschmid/train/V011.pPb/2015-11-17_1243.23219/mergedPeriods/pPb/5.023ATeV/LHC13c.pass2", Double_t multiplicityStepSize = 500/*-1 for all multiplicities*/,
						 Double_t maxMultiplicity = 2000,
						 Int_t onlyEtaShapeComparison = 0, TString fileName = "maschmid_PIDetaAdv.root", 
						 TString listName = "maschmid_PIDetaAdv", Int_t momentumAxis = kPtpcInner/* = kPt*/, Int_t binTypePt = kPtBinTypePPMult) { 
	
	TDatime daTime;
	TString saveFileName = Form("outputCheckShapeEtaAdv_%04d_%02d_%02d__%02d_%02d.root", daTime.GetYear(), daTime.GetMonth(), 
										 daTime.GetDay(), daTime.GetHour(), daTime.GetMinute());
	
	TFile* fSave = TFile::Open(Form("%s/%s", path.Data(), saveFileName.Data()), "recreate");
	if (!fSave) {
		std::cout << "Failed to open save file \"" << Form("%s/%s", path.Data(), saveFileName.Data()) << "\"!" << std::endl;
		return;
	}
	
	Int_t nPtBins = -1;
	const Double_t *binsPt = 0x0;
	
	
	
	Int_t cutForFitting = 10;
	Double_t heightFractionForFittingRange = 0.1;
	
	TList* histList = 0x0;
	
	TFile* f = TFile::Open(Form("%s/%s", path.Data(), fileName.Data()));
	if (!f)  {
		std::cout << "Failed to open file \"" << Form("%s/%s", path.Data(), fileName.Data()) << "\"!" << std::endl;
		return;
	}
	
	THnSparse* hPIDdata = 0x0;
	
	
	histList = (TList*)(f->Get(listName.Data()));
	if (!histList) {
		std::cout << "Failed to load list \"" << listName.Data() << "\"!" << std::endl;
		return;
	}
	
	// Extract the data histogram
	hPIDdata = dynamic_cast<THnSparse*>(histList->FindObject("hPIDdataAll"));
	if (!hPIDdata) {
		hPIDdata = dynamic_cast<THnSparse*>(f->Get(Form("%s/hPIDdataAll", listName.Data())));
		if (!hPIDdata)  {
			std::cout << "Failed to load data histo!" << std::endl;
			return;
		}
	}
	
	// Set proper errors
	hPIDdata->Sumw2();
	Long64_t nBinsTHnSparse = hPIDdata->GetNbins();
	Double_t binContent = 0;
	
	for (Long64_t bin = 0; bin < nBinsTHnSparse; bin++) {
		binContent = hPIDdata->GetBinContent(bin);
		hPIDdata->SetBinError(bin, TMath::Sqrt(binContent));
	}
	
	
	getSeparationHist(hPIDdata, kFALSE, fSave);
	//getSeparationHist(hPIDdata, kTRUE, fSave);

}



