
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

  void bookHistogram(Bool_t isPbPb);
  void read(TString filename, Bool_t enablePileUpCut);
  void Read_tracktree(TString filename,Bool_t enablePileUpCut);
  void Filltreeformap_track(TString filename_track, Bool_t enablePileUpCut);
  void Filltreeformap_V0(TString filename_v0, Bool_t enablePileUpCut);  
  void WriteHistogram();
  void execute();
  void BinLogAxis(THnSparseF *h, Int_t axisNumber);
  void SetAxisNamesFromTitle(const THnSparseF *h);
  

 private:
  TFile*  mOutputFile;
  TFile*  mOutputFile_eta;

 TTree* fTree;
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





  ClassDef(AliSkimmedDataAnalysisMaker,1);
};
