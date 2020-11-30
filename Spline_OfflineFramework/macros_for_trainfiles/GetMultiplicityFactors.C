// #include "TGraphErrors.h"
// #include "TF1.h"
// #include "TFile.h"
// #include "TDirectoryFile.h"
// #include "TString.h"
// #include "TSystem.h"
// #include "TArrayD.h"
// #include "TArrayI.h"
// 
// using namespace std;

TString GetMultiplicityFactors(TString file, Bool_t getSigmaFactors = kTRUE, Int_t tanThetaBin = 2, Int_t pBin = 9, Bool_t getNewParam = kTRUE) {
  if (file == "") {
    cout << "Specify a file" << endl;
    return "";
  }
  
  TFile* f = new TFile(file.Data(),"READ");
  if (!f) {
    cout << "File " << f << " not found" << endl;
    return "";
  }
  
  const Int_t nSets = 3;
  
  const Int_t nOfParameters_old[nSets] = {5,3,4};
  const Int_t nOfParameters_new[nSets] = {2,2,4};
  
  const TString dirNames_old[nSets] = {TString::Format("tanThetaBin%i",tanThetaBin), "tanThetaFits", TString::Format("tanThetaBin%i",tanThetaBin)};
  const TString dirNames_new[nSets] = {"allTanTheta", "allTanTheta", "allTanTheta"};
  
  const TString graphNames_old[nSets] = {TString::Format("slopes2_tanTheta%i",tanThetaBin), TString::Format("tanThetaSlopes_pBin%i",pBin), TString::Format("sigmaSlopes2_tanTheta%i",tanThetaBin)};
  const TString graphNames_new[nSets] = {"slopes2_Parabel_AllEta", "curves2_Parabel_AllEta", "sigmaSlopes2_Parabel_AllEta"};
  
  const TString fitfuncNames_old[nSets] = {TString::Format("fitFunc_tanTheta%i",tanThetaBin), "pol2", TString::Format("fitFuncSigma_tanTheta%i",tanThetaBin)};
  const TString fitfuncNames_new[nSets] = {"pol1", "pol1", "fitFuncSigma_Parabel_AllEta"};
  
  TArrayI* nOfParameters = new TArrayI(nSets);
  TObjArray* dirNames = new TObjArray();
  TObjArray* graphNames = new TObjArray();
  TObjArray* fitfuncNames = new TObjArray();
  
  if (getNewParam) {
    for (Int_t i=0;i<nSets;i++) {
      nOfParameters->AddAt(nOfParameters_new[i],i);
      dirNames->AddAt(new TObjString(dirNames_new[i]),i);
      graphNames->AddAt(new TObjString(graphNames_new[i]),i);
      fitfuncNames->AddAt(new TObjString(fitfuncNames_new[i]),i);
    }
  }
  else {
    for (Int_t i=0;i<nSets;i++) {
      nOfParameters->AddAt(nOfParameters_old[i],i);
      dirNames->AddAt(new TObjString(dirNames_old[i]),i);
      graphNames->AddAt(new TObjString(graphNames_old[i]),i);
      fitfuncNames->AddAt(new TObjString(fitfuncNames_old[i]),i);
    }
  }
  
  TArrayD* parameters = new TArrayD(nOfParameters->GetSum());
  for (Int_t i=0;i<parameters->GetSize();++i)
    parameters->AddAt(0.0,i);
  
  
  TDirectoryFile* df = 0x0;
  TGraphErrors* g = 0x0;
  Int_t parNumber = 0;
  
  for (Int_t i=0;i<nSets;i++) {
    
    TString dirName = ((TObjString*)dirNames->At(i))->String();
    f->GetObject(dirName.Data(),df);
    if (!df) {
      cout << "Directory " << dirName << " in file " << file << " not found" << endl;
      return "";
    }    
    TString graphName = ((TObjString*)graphNames->At(i))->String();
    df->GetObject(graphName.Data(),g);
    if (!g) {
      cout << "Graph " << graphName << " in Directory " << dirName << " in file " << file << " not found" << endl;
      return "";
    }
    TString fitfuncName = ((TObjString*)fitfuncNames->At(i))->String();
    TF1* func = g->GetFunction(fitfuncName.Data());
    if (!func) {
      cout << "Function " << fitfuncName << " of Graph " << graphName << " in Directory " << dirName << " in file " << file << " not found" << endl;
      return ""; 
    }
    for (Int_t j=0;j<nOfParameters->At(i);++j) {
      if (i == 3 && !getSigmaFactors)
        continue;
      
      if (i == 1 && !getNewParam) {
        if (j==2) {
          Double_t y,thetaOfTanThetaBin;
          g->GetPoint(tanThetaBin,thetaOfTanThetaBin,y);
          parameters->AddAt(thetaOfTanThetaBin,parNumber);
        }
        else
          parameters->AddAt(func->GetParameter(j+1),parNumber);
      }
      else {
        parameters->AddAt(func->GetParameter(j),parNumber);
      }
      parNumber++; 
    }
  }
  
  f->Close();
  
  parNumber = 0;
  TString s = "";
  for (Int_t i=0;i<nSets;++i) {
    for (Int_t j=0;j<nOfParameters->At(i);++j) {
      s.Append(TString::Format("%e,",parameters->At(parNumber)));
      parNumber++;
    }
    s.Remove(s.Length()-1);
    s.Append(";");
  }
  s.Remove(s.Length()-1);
//   
  TString command = TString("dirname $(readlink -f ") + file + TString(")");

  TString sbase = gSystem->GetFromPipe(command.Data());
//   TString s = TString::Format("%e,%e,%e,%e,%e; %e,%e, %e;",par00,par01,par02,par03,par04,par10,par11,thetaOfTanThetaBin);
//   s.Append(TString::Format("%e,%e,%e,%e",par20,par21,par22,par23));
  
  command = TString ("echo \"") + s + TString("\" >| ") + sbase + TString("/multParameters.txt");
  
  
  gSystem->Exec(command.Data());
// 
  return s;
}