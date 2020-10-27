
#include "TObject.h"
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "V0FlatAna.h"
#include "TrackFlatAna.h"
//#include "AliESDtrack.h"
class TH2F;
class TH1F;

class TArrayF;
template <class X>
class THnSparseT;
typedef class THnSparseT<TArrayF> THnSparseF;


class AliSkimmedDataAnalysisMaker :public TObject {

  
 public:
  AliSkimmedDataAnalysisMaker(const char*,const char*);
  //const char*
  virtual ~AliSkimmedDataAnalysisMaker();

  void bookHistogram();
  void read(TString filename);
  void Read_tracktree(TString filename);
  void Filltreeformap_track(TString filename_track);
  void Filltreeformap_V0(TString filename_v0);  
  void WriteHistogram();
  void execute();
  void BinLogAxis(THnSparseF *h, Int_t axisNumber);
  void SetAxisNamesFromTitle(const THnSparseF *h);
  void TreeV0_BBFitAnalysis(TString filename_v0);
  void TreePrimary_BBFitAnalysis(TString filename_track);
  

 private:
  TFile*  mOutputFile;
  TFile*  mOutputFile_eta;
  TFile * mOutputFile_BBFitAnalysis;

 TTree* fTree;
 TTree* fTree2;
 TH2F*  mh2pTdEdxVspT;

 TH2F *mh2pTdEdxVspT_K0Like;
 TH2F *mh2pTdEdxVspT_ELike;
 TH2F *mh2pTdEdxVspT_LLike;
 TH2F *mh2pTdEdxVspT_ALLike;

 TH2F *mh2dEdxVspT_Electron;
 TH2F *mh2dEdxVspT_Pion;
 TH2F *mh2dEdxVspT_Proton;

 TH1F *mh1p;
 TH2F *h_pileupcorrelation;
 TH2F *h_pileupcorrelation_bc;
 TH1F *h_Centrality;
 TH1F *h_Centrality_all;

 THnSparseF* fHistPidQA;
 THnSparseF* fHistPid_separation_power;

//tree varialbe
  Double_t fPtpc;
  Double_t fDeDx;
  Double_t fDeDxExpected;
  Double_t fTanTheta;
  Double_t fTPCsignalN;
  Double_t fTPCsignalNsubthreshold;
  Double_t fNumTPCClustersInActiveVolume;
  Double_t fPIDtype;
  Double_t fMultiplicity;

//tree varialbe BB fit check

  Double_t p;
  Double_t oneoverpt;
  Double_t rawtpcsignal;
  Double_t BG;
  Double_t eta;
  Double_t multiplicty;
  Double_t centrality;
  Double_t PDGcode;



  ClassDef(AliSkimmedDataAnalysisMaker,1);
};
