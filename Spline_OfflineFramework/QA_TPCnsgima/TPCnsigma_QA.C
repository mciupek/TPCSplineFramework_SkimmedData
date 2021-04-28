void SetMarkerColorStyle(TH1D *h,unsigned int color,int marker){
h->SetLineColor(color);
h->SetMarkerColor(color);
h->SetMarkerStyle(20);
h->SetMarkerSize(0.8);
}

void SetTH2Layout(TH2D *h){
h->GetXaxis()->SetTitle("p (GeV/c)");
h->GetYaxis()->SetTitle("TPCnsigma");
h->GetXaxis()->SetMoreLogLabels(kTRUE);
h->GetXaxis()->SetNoExponent(kTRUE);



}

void TPCnsigma_QA(){

TFile *file = new TFile("TPCresidualPID.root");
THnSparseF* hist = (THnSparseF*) file->Get("fHistPidQA");

hist->GetAxis(6)->SetRangeUser(0,25000);
//hist->GetAxis(7)->SetRangeUser(0,1);

Double_t nTPCsigma_minus = -2.5;
Double_t nTPCsigma_plus = 2.5;

TH2D *histProtonTPC = new TH2D();
TH2D *histProtonTOF = new TH2D();
TH2D *histProtonV0 = new TH2D();
TH2D *histProtonV0plusTOF = new TH2D();

TH2D *histPionTPC = new TH2D();
TH2D *histPionTOF = new TH2D();
TH2D *histPionV0 = new TH2D();
TH2D *histPionV0plusTOF = new TH2D();

TH2D *histElectronTPC = new TH2D();
TH2D *histElectronTOF = new TH2D();
TH2D *histElectronV0 = new TH2D();
TH2D *histElectronV0plusTOF = new TH2D();

TF1 *gausFit = new TF1("gaus","gaus",-1.5,1.5);
TString fitoption = "QNRG2";


TH2D *histKaonTPC = new TH2D();
TH2D *histKaonTOF = new TH2D();

TLine *line_mean = new TLine(0.3,0,10,0);
TLine *line_sigma = new TLine(0.3,1,10,1);
line_mean->SetLineColor(kBlack);
line_mean->SetLineStyle(2);
line_mean->SetLineWidth(3);
line_sigma->SetLineColor(kMagenta);
line_sigma->SetLineStyle(2);
line_sigma->SetLineWidth(3);

TCanvas *canvas_proton = new TCanvas();
canvas_proton->Divide(2,2);

TCanvas *canvas_electron = new TCanvas();
canvas_electron->Divide(2,2);

TCanvas *canvas_kaon = new TCanvas();
canvas_kaon->Divide(2,2);

TCanvas *canvas_pion = new TCanvas();
canvas_pion->Divide(2,2);



for(Int_t i =1; i<5;i++){
canvas_proton->GetPad(i)->SetLogz();
canvas_proton->GetPad(i)->SetLogx();
canvas_electron->GetPad(i)->SetLogz();
canvas_electron->GetPad(i)->SetLogx();
canvas_pion->GetPad(i)->SetLogz();
canvas_pion->GetPad(i)->SetLogx();
canvas_kaon->GetPad(i)->SetLogz();
canvas_kaon->GetPad(i)->SetLogx();
}

//Drawing 2D prjoection for each particle species


//Protons
//Proton_TPC:
hist->GetAxis(2)->SetRangeUser(0,0); // Non-V0s
hist->GetAxis(3)->SetRangeUser(3,3); // protons

histProtonTPC = hist->Projection(4,0);
histProtonTPC->SetName("histProtonTPC");
histProtonTPC->GetXaxis()->SetRangeUser(0.15,0.7);
histProtonTPC->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histProtonTPC);

TObjArray aSlices;
histProtonTPC->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * protonPointsTPC = (TH1D *) aSlices.At(1)->Clone();
TH1D * protonPointsTPC_width = (TH1D *) aSlices.At(2)->Clone();

// Proton TOF:
hist->GetAxis(5)->SetRangeUser(-3,3);
histProtonTOF = hist->Projection(4,0);
histProtonTOF->SetName("histProtonTOF");
histProtonTOF->GetXaxis()->SetRangeUser(0.6,3.5);
histProtonTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histProtonTOF);
histProtonTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * protonPointsTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * protonPointsTOF_width = (TH1D *) aSlices.At(2)->Clone();

//Proton V0
hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

hist->GetAxis(2)->SetRangeUser(3,3); // V0 protons
hist->GetAxis(3)->SetRangeUser(3,3); // protons

histProtonV0 = hist->Projection(4,0);
histProtonV0->SetName("histProtonV0");
histProtonV0->GetXaxis()->SetRangeUser(0.15,7.);
histProtonV0->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histProtonV0);

histProtonV0->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * protonPointsV0 = (TH1D *) aSlices.At(1)->Clone();
TH1D * protonPointsV0_width = (TH1D *) aSlices.At(2)->Clone();

//ProtonV0plusTOF
hist->GetAxis(5)->SetRangeUser(-3,3);
histProtonV0plusTOF = hist->Projection(4,0);
histProtonV0plusTOF->SetName("histProtonV0plusTOF");
histProtonV0plusTOF->GetXaxis()->SetRangeUser(0.15,7.);
histProtonV0plusTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histProtonV0plusTOF);

histProtonV0plusTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * protonPointsV0plusTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * protonPointsV0plusTOF_width = (TH1D *) aSlices.At(2)->Clone();

hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);


canvas_proton->cd();
canvas_proton->cd(1);
histProtonTPC->Draw("colz");
protonPointsTPC->Draw("same");
protonPointsTPC_width->Draw("same");
canvas_proton->cd(2);
histProtonTOF->Draw("colz");
protonPointsTOF->Draw("same");
protonPointsTOF_width->Draw("same");
canvas_proton->cd(3);
histProtonV0->Draw("colz");
protonPointsV0->Draw("same");
protonPointsV0_width->Draw("same");
canvas_proton->cd(4);
histProtonV0plusTOF->Draw("colz");
protonPointsV0plusTOF->Draw("same");
protonPointsV0plusTOF_width->Draw("same");

//Electron:
//TOF electron
hist->GetAxis(2)->SetRangeUser(0,0); // Non-V0s
hist->GetAxis(3)->SetRangeUser(0,0); // electrons
hist->GetAxis(5)->SetRangeUser(-2.999,2.999); // 3sigma in TOF
histElectronTOF = hist->Projection(4,0);
histElectronTOF->SetName("histElectronTOF");
histElectronTOF->GetXaxis()->SetRangeUser(0.5,3.0);
histElectronTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histElectronTOF);

histElectronTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * electronPointsTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * electronPointsTOF_width = (TH1D *) aSlices.At(2)->Clone();

// ELectronV0:
hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

hist->GetAxis(2)->SetRangeUser(1,1); // V0 electrons
hist->GetAxis(3)->SetRangeUser(0,0); // electrons
histElectronV0 = hist->Projection(4,0);
histElectronV0->SetName("histElectronV0");
histElectronV0->GetXaxis()->SetRangeUser(0.15,5.0);
histElectronV0->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histElectronV0);
histElectronV0->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * electronPointsV0 = (TH1D *) aSlices.At(1)->Clone();
TH1D * electronPointsV0_width = (TH1D *) aSlices.At(2)->Clone();

//ELectronV0 plus TOFbit
hist->GetAxis(5)->SetRangeUser(-3,3);
histElectronV0plusTOF = hist->Projection(4,0);
histElectronV0plusTOF->SetName("histElectronV0plusTOF");
histElectronV0plusTOF->GetXaxis()->SetRangeUser(0.5,5.0);
histElectronV0plusTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histElectronV0plusTOF);

histElectronV0plusTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * electronPointsV0plusTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * electronPointsV0plusTOF_width = (TH1D *) aSlices.At(2)->Clone();

hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

//plotting:
canvas_electron->cd();
canvas_electron->cd(2);
histElectronTOF->Draw("colz");
electronPointsTOF->Draw("same");
electronPointsTOF_width->Draw("same");
canvas_electron->cd(3);
histElectronV0->Draw("colz");
electronPointsV0->Draw("same");
electronPointsV0_width->Draw("same");
canvas_electron->cd(4);
histElectronV0plusTOF->Draw("colz");
electronPointsV0plusTOF->Draw("same");
electronPointsV0plusTOF_width->Draw("same");


//Kaons
//KaonTPC:
hist->GetAxis(2)->SetRangeUser(0,0); // kaons
hist->GetAxis(3)->SetRangeUser(2,2); // kaons (used for nsigma and for the eta correction of the signal)
histKaonTPC = hist->Projection(4,0);
histKaonTPC->SetName("histKaonTPC");
histKaonTPC->GetXaxis()->SetRangeUser(0.15,8);
histKaonTPC->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histKaonTPC);
histKaonTPC->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * kaonPointsTPC = (TH1D *) aSlices.At(1)->Clone();
TH1D * kaonPointsTPC_width = (TH1D *) aSlices.At(2)->Clone();

//kaonTof
hist->GetAxis(5)->SetRangeUser(-3,3);
histKaonTOF = hist->Projection(4,0);
histKaonTOF->SetName("histKaonTOF");
histKaonTOF->GetXaxis()->SetRangeUser(0.3,1.5);
histKaonTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histKaonTOF);

histKaonTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * kaonPointsTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * kaonPointsTOF_width = (TH1D *) aSlices.At(2)->Clone();

hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

//Drawing:

canvas_kaon->cd();
canvas_kaon->cd(1);
histKaonTPC->Draw("colz");
kaonPointsTPC->Draw("same");
kaonPointsTPC_width->Draw("same");
canvas_kaon->cd(2);
histKaonTOF->Draw("colz");
kaonPointsTOF->Draw("same");
kaonPointsTOF_width->Draw("same");


//Pions
//Pion TPC
hist->GetAxis(2)->SetRangeUser(0,0); // Non-V0s
hist->GetAxis(3)->SetRangeUser(1,1); // pions
histPionTPC = hist->Projection(4,0);
histPionTPC->SetName("histPionTPC");
histPionTPC->GetXaxis()->SetRangeUser(0.15,0.7);
histPionTPC->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histPionTPC);

histPionTPC->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * pionPointsTPC = (TH1D *) aSlices.At(1)->Clone();
TH1D * pionPointsTPC_width = (TH1D *) aSlices.At(2)->Clone();

//pionTof
hist->GetAxis(5)->SetRangeUser(-3,3);
histPionTOF = hist->Projection(4,0);
histPionTOF->SetName("histPionTOF");
histPionTOF->GetXaxis()->SetRangeUser(0.5,1.1);
histPionTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histPionTOF);


histPionTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * pionPointsTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * pionPointsTOF_width = (TH1D *) aSlices.At(2)->Clone();

hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

//pionV0Graph
hist->GetAxis(2)->SetRangeUser(2,2); // V0 pions
hist->GetAxis(3)->SetRangeUser(1,1); // pions
histPionV0 = hist->Projection(4,0);
histPionV0->SetName("histPionV0");
histPionV0->GetXaxis()->SetRangeUser(0.15,7.);
histPionV0->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histPionV0);


histPionV0->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * pionPointsV0 = (TH1D *) aSlices.At(1)->Clone();
TH1D * pionPointsV0_width = (TH1D *) aSlices.At(2)->Clone();

//pionV0plusTOFGraph
hist->GetAxis(5)->SetRangeUser(-3,3);
histPionV0plusTOF = hist->Projection(4,0);
histPionV0plusTOF->SetName("histPionV0plusTOF");
histPionV0plusTOF->GetXaxis()->SetRangeUser(0.15,7.);
histPionV0plusTOF->GetYaxis()->SetRangeUser(nTPCsigma_minus,nTPCsigma_plus);
SetTH2Layout(histPionV0plusTOF);


histPionV0plusTOF->FitSlicesY(gausFit,0,-1,0, fitoption, &aSlices);
TH1D * pionPointsV0plusTOF = (TH1D *) aSlices.At(1)->Clone();
TH1D * pionPointsV0plusTOF_width = (TH1D *) aSlices.At(2)->Clone();

hist->GetAxis(2)->SetRangeUser(0,-1);
hist->GetAxis(3)->SetRangeUser(0,-1);
hist->GetAxis(5)->SetRangeUser(0,-1);

canvas_pion->cd();
canvas_pion->cd(1);
histPionTPC->Draw("colz");
pionPointsTPC->Draw("same");
pionPointsTPC_width->Draw("same");
canvas_pion->cd(2);
histPionTOF->Draw("colz");
pionPointsTOF->Draw("same");
pionPointsTOF_width->Draw("same");
canvas_pion->cd(3);
histPionV0->Draw("colz");
pionPointsV0->Draw("same");
pionPointsV0_width->Draw("same");
canvas_pion->cd(4);
histPionV0plusTOF->Draw("colz");
pionPointsV0plusTOF->Draw("same");
pionPointsV0plusTOF_width->Draw("same");

//Plottoing mean for all particles:

SetMarkerColorStyle(protonPointsTPC_width,kMagenta,20);
SetMarkerColorStyle(protonPointsTOF_width,kMagenta,20);
SetMarkerColorStyle(protonPointsV0_width,kMagenta,20);
SetMarkerColorStyle(protonPointsV0plusTOF_width,kMagenta,20);

SetMarkerColorStyle(pionPointsTPC_width,kMagenta,20);
SetMarkerColorStyle(pionPointsTOF_width,kMagenta,20);
SetMarkerColorStyle(pionPointsV0_width,kMagenta,20);
SetMarkerColorStyle(pionPointsV0plusTOF_width,kMagenta,20);

SetMarkerColorStyle(electronPointsTOF_width,kMagenta,20);
SetMarkerColorStyle(electronPointsV0_width,kMagenta,20);
SetMarkerColorStyle(electronPointsV0plusTOF_width,kMagenta,20);

SetMarkerColorStyle(kaonPointsTPC_width,kMagenta,20);
SetMarkerColorStyle(kaonPointsTOF_width,kMagenta,20);


SetMarkerColorStyle(protonPointsTPC,kBlack,20);
SetMarkerColorStyle(protonPointsTOF,kBlack,20);
SetMarkerColorStyle(protonPointsV0,kBlack,20);
SetMarkerColorStyle(protonPointsV0plusTOF,kBlack,20);

SetMarkerColorStyle(pionPointsTPC,kBlack,20);
SetMarkerColorStyle(pionPointsTOF,kBlack,20);
SetMarkerColorStyle(pionPointsV0,kBlack,20);
SetMarkerColorStyle(pionPointsV0plusTOF,kBlack,20);

SetMarkerColorStyle(electronPointsTOF,kBlack,20);
SetMarkerColorStyle(electronPointsV0,kBlack,20);
SetMarkerColorStyle(electronPointsV0plusTOF,kBlack,20);

SetMarkerColorStyle(kaonPointsTPC,kBlack,20);
SetMarkerColorStyle(kaonPointsTOF,kBlack,20);

canvas_proton->SaveAs("proton_tpcnsigma.pdf");
canvas_kaon->SaveAs("kaon_tpcnsigma.pdf");
canvas_pion->SaveAs("pion_tpcnsigma.pdf");
canvas_electron->SaveAs("electron_tpcnsigma.pdf");


/*
TCanvas *proton_mean = new TCanvas();
gStyle->SetOptStat(0);
histProtonV0->SetTitle("Proton from #Lambda/#bar{#Lambda} decays");
histProtonV0->Draw("colz");
protonPointsV0->Draw("same");
protonPointsV0_width->Draw("same");
line_mean->Draw("same");
line_sigma->Draw("same");
proton_mean->SaveAs("ProtonV0_mean.pdf");


TCanvas *proton_mean = new TCanvas();
protonPointsV0->Draw("");
protonPointsTPC->Draw("same");
protonPointsTOF->Draw("same");
protonPointsV0plusTOF->Draw("same");

TCanvas *pion_mean = new TCanvas();
//pionPointsV0->Draw("");
//pionPointsTPC->Draw("same");
//pionPointsTOF->Draw("same");
pionPointsV0plusTOF->Draw("same");

TCanvas *electron_mean = new TCanvas();
electronPointsV0->Draw("");
electronPointsTOF->Draw("same");
electronPointsV0plusTOF->Draw("same");

TCanvas *kaon_mean = new TCanvas();
kaonPointsTPC->Draw("");
kaonPointsTOF->Draw("same");
*/
}
