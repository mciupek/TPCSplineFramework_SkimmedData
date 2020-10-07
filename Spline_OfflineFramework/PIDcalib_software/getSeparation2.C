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
#include "TLatex.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraph.h"
#include "TMath.h"
#include "TDatime.h"

#include <iostream>
#include <iomanip>

#include "THnSparseDefinitions.h"


//~ void getSeparation2(TString path = "/media/nschmidt/Daten2/HIWI_SPLINES/LHC15o_pass2_lowIR", TString period = "LHC15o_pass2_lowIR", TString fileName = "AnalysisResults.root") { 
void getSeparation2(TString path = "/media/nschmidt/Daten2/HIWI_SPLINES/LHC15o_pass2_lowIR", TString period = "LHC15o_pass2_lowIR", TString fileName = "AnalysisResults.root") { 
	/* SETTING BASIC STYLE */
	gStyle->SetOptDate(0);
	gStyle->SetOptTitle(1);
	gStyle->SetOptStat(0);
	gStyle->SetOptFit(0);
	Double_t scalefactor = 10;
	
	TString pathes[10];
	pathes[0]="/media/nschmidt/Daten2/HIWI_SPLINES/LHC15f_pass2/Iteration3";
	pathes[1]="/media/nschmidt/Daten2/HIWI_SPLINES/LHC15f_pass2/Iteration3";
	pathes[2]="/media/nschmidt/Daten2/HIWI_SPLINES/LHC15f_pass2/Iteration3";
	
	
	/* OPENING FILE AND LOADING HISTOGRAMS */
	TFile* f = TFile::Open(Form("%s/%s", path.Data(), fileName.Data()));
	if (!f)  {
		cout << "Failed to open file " << Form("%s/%s", path.Data(), fileName.Data()) << "!" << endl;
		return;
	} else {
		cout << "Successfully openend file " << Form("%s/%s", path.Data(), fileName.Data()) << "!" << endl;
	}
	TDirectory* PIDqaDir1=	(TDirectory*)f		->Get("PIDqa");
	TList* PIDqaDir2		=	(TList*)PIDqaDir1	->Get("PIDqa");
	TList* TPCDir			=	(TList*)PIDqaDir2	->FindObject("TPC");
	TList* TPCBasicDir	=	(TList*)TPCDir		->FindObject("TPCBasic");
	TList* TPCV0Dir		=	(TList*)TPCDir		->FindObject("TPCV0");
	
	
	TH2D* hSigP_TPC_std_V0_electron 	= (TH2D*)TPCV0Dir->FindObject("hSigP_TPC_std_V0_electron");
	TH2D* hSigP_TPC_std_V0_pion		= (TH2D*)TPCV0Dir->FindObject("hSigP_TPC_std_V0_pion");
	
	/* CREATING MULTIPURPOSE CANVAS FOR PLOTTING */
	TCanvas * outputCanvas = new TCanvas("outputCanvas","outputCanvas",10,10,1000,1100) ;
	outputCanvas->SetLogy();
	
	TFitResultPtr resPi;
	Double_t resultsPi[4];
	Double_t resultErrorsPi[4];
		
	TFitResultPtr resEl;
		Double_t resultsEl[4];
		Double_t resultErrorsEl[4];
	
	TH1 *hProjection_electron; 
	TH1 *hProjection_pion;
	TF1 *gaus1;
	TF1 *gaus2;
	
	Double_t sepPow ;
	Double_t sepPowFinal[200];
	Int_t sepPowIndex=0;
	Double_t axisPoints[200];
	for(Int_t i=1; i<hSigP_TPC_std_V0_electron->GetNbinsX(); i+=2){
		
		if(i>50) continue;
		
		/* PROJECTING THE 2D HISTOGRAMS ABOVE 500 MeV */
		//~ Double_t GeVbinUsed = .5;
		//~ Double_t GeVbinUsedUpper = 0.03;
		//~ TH1 *hProjection_electron 	= hSigP_TPC_std_V0_electron->ProjectionY("_temp",hSigP_TPC_std_V0_electron->GetXaxis()->FindBin(GeVbinUsed),hSigP_TPC_std_V0_electron->GetXaxis()->FindBin(GeVbinUsed+GeVbinUsedUpper));
	//~ // 	TH1 *hProjection_electron 	= hSigP_TPC_std_V0_electron->ProjectionY("_temp",hSigP_TPC_std_V0_electron->GetXaxis()->FindBin(0.5),hSigP_TPC_std_V0_electron->GetNbinsX());
		//~ TH1 *hProjection_pion 		= hSigP_TPC_std_V0_pion		->ProjectionY("_temp2",hSigP_TPC_std_V0_pion->GetXaxis()->FindBin(GeVbinUsed),hSigP_TPC_std_V0_pion->GetXaxis()->FindBin(GeVbinUsed+GeVbinUsedUpper));
	//~ // 	TH1 *hProjection_pion 		= hSigP_TPC_std_V0_pion		->ProjectionY("_temp2",hSigP_TPC_std_V0_pion->GetXaxis()->FindBin(0.5),hSigP_TPC_std_V0_pion->GetNbinsX());
		hProjection_electron 	= hSigP_TPC_std_V0_electron->ProjectionY(Form("_temp%d",i),i,i+2);
	// 	TH1 *hProjection_electron 	= hSigP_TPC_std_V0_electron->ProjectionY("_temp",hSigP_TPC_std_V0_electron->GetXaxis()->FindBin(0.5),hSigP_TPC_std_V0_electron->GetNbinsX());
		hProjection_pion 		= hSigP_TPC_std_V0_pion		->ProjectionY(Form("_temps%d",i),i,i+2);
	// 	TH1 *hProjection_pion 		= hSigP_TPC_std_V0_pion		->ProjectionY("_temp2",hSigP_TPC_std_V0_pion->GetXaxis()->FindBin(0.5),hSigP_TPC_std_V0_pion->GetNbinsX());
		cout << hProjection_electron->GetEntries() << "\t" << hProjection_pion->GetEntries() << endl;
		if (hProjection_electron->GetEntries()==0||hProjection_pion->GetEntries()==0) continue;
		/* FITTING AND PLOTTING THE ELECTRONS */
		Double_t minGausEl = 60;
		Double_t maxGausEl = 120;
		
		resEl = hProjection_electron->Fit("gaus", "NRSQ", "", minGausEl, maxGausEl);
		if ((Int_t)resEl == 0) {
			for (Int_t i = 0; i < 3; i++) {
				resultsEl[i] = resEl->GetParams()[i];
				resultErrorsEl[i] = resEl->GetErrors()[i];
			}
			resultsEl[3] = resEl->Ndf()>0 ? resEl->Chi2()/resEl->Ndf() : 0;
			resultErrorsEl[3] = 0;
		}
		hProjection_electron->Draw();
		gaus1 = new TF1("gaus1","gaus",minGausEl*scalefactor,maxGausEl/scalefactor);
		gaus1->SetParameters(resultsEl[0],resultsEl[1],resultsEl[2]);
		gaus1->SetLineWidth(5);
		gaus1->Draw("same");
		outputCanvas->SaveAs(Form("%s/histEl.pdf",path.Data()));
		
		/* FITTING AND PLOTTING THE PIONS */
		Double_t minGausPi = 40;
		Double_t maxGausPi = 100;
		
		resPi = hProjection_pion->Fit("gaus", "NRSQ", "", minGausPi, maxGausPi);
		
		if ((Int_t)resPi == 0) {
			for (Int_t i = 0; i < 3; i++) {
				resultsPi[i] = resPi->GetParams()[i];
				resultErrorsPi[i] = resPi->GetErrors()[i];
			}
			resultsPi[3] = resPi->Ndf()>0 ? resPi->Chi2()/resPi->Ndf() : 0;
			resultErrorsPi[3] = 0;
		}
		hProjection_pion->Draw();
		gaus2 = new TF1("gaus2","gaus",minGausPi*scalefactor,maxGausPi/scalefactor);
		gaus2->SetParameters(resultsPi[0],resultsPi[1],resultsPi[2]);
		gaus2->SetLineWidth(5);
		gaus2->Draw("same");
		gaus1->Draw("same");
		hProjection_electron->Draw("same");
		outputCanvas->SaveAs(Form("%s/histPi.pdf",path.Data()));
		
		
		/* CALCULATING THE SEPARATION POWER */
		sepPow = TMath::Abs(resultsEl[1] - resultsPi[1])/(TMath::Abs((resultsEl[2] + resultsPi[2])) / 2.);
		cout << "Separation power of: " << sepPow << " for bin " << i << " ( " << hSigP_TPC_std_V0_electron->GetXaxis()->GetBinCenter(i) << " ) "<< " ( " << hSigP_TPC_std_V0_pion->GetXaxis()->GetBinCenter(i) << " ) " << endl;
		axisPoints[sepPowIndex]= hSigP_TPC_std_V0_pion->GetXaxis()->GetBinCenter(i);
		sepPowFinal[sepPowIndex]=sepPow;
		sepPowIndex++;
		
		if(sepPow < 4.0)
			outputCanvas->SaveAs(Form("%s/histPi%d.pdf",path.Data(),i));
			
			
			
	}
	/* PLOTTING OF THE COMBINED SPECTRA */
	outputCanvas->SetTopMargin(0.01);
	outputCanvas->SetRightMargin(0.03);
	outputCanvas->SetBottomMargin(0.08);
	hProjection_pion->Draw();
	hProjection_pion->SetLineColor(kGreen+2);
	hProjection_pion->SetFillStyle(3244);
	hProjection_pion->SetFillColor(kGreen+2);
	gaus2->Draw("same");
	gaus2->SetLineColor(kCyan+2);
	hProjection_electron->Draw("same");
	hProjection_electron->SetLineColor(kRed+4);
	hProjection_electron->SetFillStyle(3344);
	hProjection_electron->SetFillColor(kRed+4);
	gaus1->Draw("same");
	TLatex* periodTex 		= new TLatex(100.,0.8*hProjection_pion->GetMaximum(),period);
	periodTex->SetTextSize(0.04);
	periodTex->Draw();
	TLatex* separationpower = new TLatex(100.,0.3*hProjection_pion->GetMaximum(),Form("Separation power: %f #sigma",sepPow));
	separationpower->SetTextSize(0.04);
	separationpower->Draw();
	TLatex* momrange = new TLatex(120.,0.11*hProjection_pion->GetMaximum(),"0.5 < TPC mom. < 0.53 GeV/#it{c}");
	momrange->SetTextSize(0.04);
	momrange->Draw();
	TLatex* electronTex 		= new TLatex(resultsEl[1],hProjection_electron->GetMaximum(),"e^{-}");
	TLatex* pionTex			= new TLatex(resultsPi[1],hProjection_pion->GetMaximum(),"#pi");
	electronTex->Draw();
	pionTex->Draw();
	hProjection_pion->SetTitle("");
	
	TLegend* legendYieldBinShifted = new TLegend(0.45,0.55,0.9,0.75);
    legendYieldBinShifted->SetFillColor(0);
    legendYieldBinShifted->SetLineColor(0);
    legendYieldBinShifted->SetTextSize(0.04);
    legendYieldBinShifted->SetTextFont(62);
    legendYieldBinShifted->AddEntry(hProjection_electron,"V0 electrons","l");
    legendYieldBinShifted->AddEntry(hProjection_pion,"V0 pions","l");
    legendYieldBinShifted->AddEntry(gaus1,"Gauss fit V0 electrons","l");
    legendYieldBinShifted->AddEntry(gaus2,"Gauss fit V0 pions","l");
    legendYieldBinShifted->Draw();
	
	outputCanvas->SaveAs(Form("%s/separation.pdf",path.Data()));
	for(Int_t i; i<sepPowIndex+1;i++) cout << sepPowFinal[i] << endl;
	
	TGraph* gr = new TGraph(sepPowIndex,axisPoints,sepPowFinal);
	TCanvas * outputCanvas2 = new TCanvas("outputCanvas2","outputCanva2s",10,10,1000,700) ;
	outputCanvas2->SetTopMargin(0.01);
	outputCanvas2->SetRightMargin(0.01);
	outputCanvas2->SetLogx(1);
	gr->GetXaxis()->SetTitle("#font[62]{p_{TPC} (GeV/#it{c})}");
	gr->SetTitle("");
	gr->SetLineColor(kBlue+2);
	gr->SetLineWidth(2);
	gr->GetYaxis()->SetTitle("#font[62]{sep power e^{-} and #pi}");
	gr->Draw();
	outputCanvas2->SaveAs("canvastest.pdf");
}
