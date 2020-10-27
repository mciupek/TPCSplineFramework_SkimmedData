// void extract(const char* input, const char* splines, const char* DataOrMc, const char* period, const char* pass, const char* collsys){

Double_t Lund(Double_t* xx, Double_t* par)
{
  // bg is beta*gamma
  const Double_t bg = xx[0];

  const Double_t beta2 = bg*bg / (1.0 + bg*bg);
  
  const Double_t a = par[0];
  const Double_t b = par[1];
  const Double_t c = par[2];
  const Double_t e = par[3];
  const Double_t f = par[4];
  
  const Double_t d = TMath::Exp(c*(a - f) / b);
 
  const Double_t powbg = TMath::Power(1.0 + bg, c);
 
  const Double_t value = a / TMath::Power(beta2,e) +
    b/c * TMath::Log(powbg / (1.0 + d*powbg));
    
  return value;
}


//________________________________________________________________________
Double_t SaturatedLund(Double_t* xx, Double_t* par)
{
  const Double_t qq = Lund(xx, par);
  return qq * TMath::Exp(par[5] / qq);
}

void SaturatedLund(TString filepath = ""){
  Double_t parametersBB[6] = { 0. }; 
  parametersBB[0] = 65; //Mainly responsible for the rise of the low x (Higher rise for lower values)
  parametersBB[1] = 20; //Lower minimum for lower values
  parametersBB[2] = 2; //more kink for higher values
  parametersBB[3] = 1.0; //Lower values: Higher rise at low x
  parametersBB[4] = 117; //Rise at high x: Higher values for higher rise
  parametersBB[5] = -10; //Influences the total height of the curve. Will be cancelled out for high x values (but depending on its value they have to be very high)
/*  parametersBB[0] = 30;
  parametersBB[1] = 10;
  parametersBB[2] = 20;
  parametersBB[3] = 0.6;
  parametersBB[4] = 20;
  parametersBB[5] = -10; */ 
  
  //2.6,14.3,-15,2.2,2.7, 0.06};
  TF1* func = funcBB = new TF1("SaturatedLund", AliTPCcalibResidualPID::SaturatedLund, 0.8, 10e3, 6);
  func->SetParameters(parametersBB);
  TCanvas* c = new TCanvas("SaturatedLund","SaturatedLund");
  c->SetLogx(1);
  func->Draw();
  if (!filepath.IsNull()) {
    const Int_t numberOfGraphs = 14;
    TString graphnames[numberOfGraphs] = {"beamDataPoints","protonTpcGraph","protonTofGraph","protonV0Graph","protonV0plusTOFGraph","pionTpcGraph","pionTofGraph","pionV0Graph","pionV0plusTOFGraph", "kaonTpcGraph","kaonTofGraph","electronGraph","electronV0Graph","electronV0plusTOFGraph"};
    TGraphErrors* graphs[numberOfGraphs];
    for (Int_t i=0;i<numberOfGraphs;++i) {
      graphs[i] = 0x0;
    }      
    gROOT->LoadMacro("~/Documents/macros/GetObjectOutOfCanvas.C+");
    TFile *f = new TFile(filepath.Data(),"READ");
    TCanvas* canvDelta_1 = 0x0;
    f->GetObject("canvDelta_1",canvDelta_1);
    for (Int_t i=0;i<numberOfGraphs;++i) {
      graphs[i]= dynamic_cast<TGraphErrors*>(GetObjectOutOfCanvas(canvDelta_1,graphnames[i].Data()));
    }
    for (Int_t i=0;i<numberOfGraphs;++i) {
      if (graphs[i]) {
        cout << graphs[i]->GetMaximum() << endl;
        graphs[i]->Draw();
      }
    }    
  }
  c->BuildLegend();
  return;
}
