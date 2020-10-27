#ifndef V0FLATANA_H
#define V0FLATANA_H


#include "TObject.h"
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class V0FlatAna :public TObject {

 public:

  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain

  // Declaration of leaf types


Double_t K0Like;
Double_t ELike;
Double_t LLike;
Double_t ALLike;
Double_t cleanK0;
Double_t cleanGamma;
Double_t cleanLambda;
Double_t cleanALambda;
//Double_t type;
//Double_t centV0;
//Double_t centITS0;
//Double_t centITS1;
//Double_t tpcClusterMult;
//Double_t multSSD;
//Double_t multSDD;
Double_t tpcTrackBeforeClean;
//Double_t triggerMask;
//Double_t isMinBias;
Double_t isPileUp;

//Double_t track0status;
Double_t track0_hasTOF;
Double_t track0_fTPCsignal;
Double_t track0_fTPCsignalN;
Double_t track0P;
Double_t track0Eta;
Double_t track0Tgl;
//Double_t track0Phi;
//Double_t track0Px;
//Double_t track0Py;
//Double_t track0Pz;
Double_t track0tofNsigmaElectron;
Double_t track0tofNsigmaKaon;
Double_t track0tofNsigmaProton;
Double_t track0tofNsigmaPion;
Double_t track0tpcNsigma_el;
Double_t track0tpcNsigma_pro;
Double_t track0tpcNsigma_ka;
Double_t track0tpcNsigma_pi;
Double_t track0ExpectedTPCSignalV0_el;
Double_t track0ExpectedTPCSignalV0_pro;
//Double_t track0ExpectedTPCSignalV0_ka;
//Double_t track0ExpectedTPCSignalV0_pi;
Double_t track0CorrectedTPCSignalV0_el;
Double_t track0CorrectedTPCSignalV0_pro;
Double_t track0CorrectedTPCSignalV0_ka;
Double_t track0CorrectedTPCSignalV0_pi;
Double_t track0GetPileupValue;
Double_t dca0_r;
Double_t dca0_z;
Double_t nCrossRows0;
Double_t tpc_cls0;

 //Double_t track1status;
Double_t track1_hasTOF;
Double_t track1_fTPCsignal;
Double_t track1_fTPCsignalN;
Double_t track1P;
Double_t track1Eta;
Double_t track1Tgl;
//Double_t track1Phi;
//Double_t track1Px;
//Double_t track1Py;
//Double_t track1Pz;
Double_t track1tofNsigmaElectron;
Double_t track1tofNsigmaKaon;
Double_t track1tofNsigmaProton;
Double_t track1tofNsigmaPion;
Double_t track1tpcNsigma_el;
Double_t track1tpcNsigma_pro;
Double_t track1tpcNsigma_ka;
Double_t track1tpcNsigma_pi;
Double_t track1ExpectedTPCSignalV0_el;
Double_t track1ExpectedTPCSignalV0_pro;
//Double_t track1ExpectedTPCSignalV0_ka;
//Double_t track1ExpectedTPCSignalV0_pi;
Double_t track1CorrectedTPCSignalV0_el;
Double_t track1CorrectedTPCSignalV0_pro;
Double_t track1CorrectedTPCSignalV0_ka;
Double_t track1CorrectedTPCSignalV0_pi;
Double_t track1GetPileupValue;
 Double_t dca1_r;                                                         
 Double_t dca1_z;                                                                
 Double_t nCrossRows1; 
 Double_t tpc_cls1;
  // List of branches

//TBranch *b_fPointAngle;					//!
//TBranch *b_fChi2;					//!
TBranch *b_K0Like;					//!
TBranch *b_ELike;					//!
TBranch *b_LLike;					//!
TBranch *b_ALLike;					//!
TBranch *b_cleanK0;					//!
TBranch *b_cleanGamma;					//!
TBranch *b_cleanLambda;					//!
TBranch *b_cleanALambda;				//!
//TBranch *b_type;					//!
//TBranch *b_centV0;					//!
//TBranch *b_centITS0;					//!
//TBranch *b_centITS1;					//!
//TBranch *b_tpcClusterMult;				//!
//TBranch *b_multSSD;					//!
//TBranch *b_multSDD;					//!
TBranch *b_tpcTrackBeforeClean;				//!
//TBranch *b_triggerMask;					//!
//TBranch *b_isMinBias;					//!
TBranch *b_isPileUp;            //!

TBranch *b_track0status;				//!
TBranch *b_track0_hasTOF;				//!
TBranch *b_track0_fTPCsignal;				//!
TBranch *b_track0_fTPCsignalN;				//!
TBranch *b_track0P;					//!
TBranch *b_track0Eta;					//!
TBranch *b_track0Tgl;					//!
//TBranch *b_track0Phi;					//!
//TBranch *b_track0Px;					//!
//TBranch *b_track0Py;					//!
//TBranch *b_track0Pz;					//!
TBranch *b_track0tofNsigmaElectron;			//!
TBranch *b_track0tofNsigmaKaon;				//!
TBranch *b_track0tofNsigmaProton;			//!
TBranch *b_track0tofNsigmaPion;				//!
TBranch *b_track0tpcNsigma_el;				//!
TBranch *b_track0tpcNsigma_pro;				//!
TBranch *b_track0tpcNsigma_ka;				//!
TBranch *b_track0tpcNsigma_pi;				//!
TBranch *b_track0ExpectedTPCSignalV0_el;		//!
TBranch *b_track0ExpectedTPCSignalV0_pro;		//!
//TBranch *b_track0ExpectedTPCSignalV0_ka;		//!
//TBranch *b_track0ExpectedTPCSignalV0_pi;		//!
TBranch *b_track0CorrectedTPCSignalV0_el;		//!
TBranch *b_track0CorrectedTPCSignalV0_pro;		//!
TBranch *b_track0CorrectedTPCSignalV0_ka;		//!
TBranch *b_track0CorrectedTPCSignalV0_pi;		//!
TBranch *b_track0GetPileupValue;			//!
 TBranch *b_dca0_r;                                     //!
 TBranch *b_dca0_z;                                     //!
 TBranch *b_nCrossRows0;                                //!
 TBranch *b_tpc_cls0;                                   //!

 //TBranch *b_track1status;				//!
TBranch *b_track1_hasTOF;                               //!
TBranch *b_track1_fTPCsignal;				//!
TBranch *b_track1_fTPCsignalN;				//!
TBranch *b_track1P;					//!
TBranch *b_track1Eta;					//!
TBranch *b_track1Tgl;					//!
//TBranch *b_track1Phi;					//!
//TBranch *b_track1Px;					//!
//TBranch *b_track1Py;					//!
//TBranch *b_track1Pz;					//!
TBranch *b_track1tofNsigmaElectron;			//!
TBranch *b_track1tofNsigmaKaon;				//!
TBranch *b_track1tofNsigmaProton;			//!
TBranch *b_track1tofNsigmaPion;				//!
TBranch *b_track1tpcNsigma_el;				//!
TBranch *b_track1tpcNsigma_pro;				//!
TBranch *b_track1tpcNsigma_ka;				//!
TBranch *b_track1tpcNsigma_pi;				//!
TBranch *b_track1ExpectedTPCSignalV0_el;		//!
TBranch *b_track1ExpectedTPCSignalV0_pro;		//!
//TBranch *b_track1ExpectedTPCSignalV0_ka;		//!
//TBranch *b_track1ExpectedTPCSignalV0_pi;		//!
TBranch *b_track1CorrectedTPCSignalV0_el;		//!
TBranch *b_track1CorrectedTPCSignalV0_pro;		//!
TBranch *b_track1CorrectedTPCSignalV0_ka;		//!
TBranch *b_track1CorrectedTPCSignalV0_pi;		//!
TBranch *b_track1GetPileupValue;                     //!
TBranch *b_dca1_r;                                     //!
TBranch *b_dca1_z;                                     //!
TBranch *b_nCrossRows1;                                //!
TBranch *b_tpc_cls1;                                   //!

  V0FlatAna();
  ~V0FlatAna();

  virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
  
  void execute();
  ClassDef(V0FlatAna, 1);
};


#endif
