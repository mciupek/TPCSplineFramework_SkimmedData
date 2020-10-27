#ifndef TRACKFLATANA_H
#define TRACKFLATANA_H


#include "TObject.h"
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

class TrackFlatAna :public TObject {

 public:

  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain

  // Declaration of leaf types

Double_t fTPCsignal;
Double_t fITSsignal;
//Double_t fTRDsignal;
Double_t trackP;
Double_t tgl;
Double_t trackEta;
Double_t tpc_cls;
Double_t fTPCsignalN;
Double_t ntracks;
//Double_t trackstatus;
Double_t track_hasTOF;
Double_t track_tpcNsigma_el;
Double_t track_tpcNsigma_pi;
Double_t track_tpcNsigma_pro;
Double_t track_tpcNsigma_ka;
Double_t track_tpcNsigma_deut;
Double_t track_tpcNsigma_tri;
Double_t track_tpcNsigma_He3;

Double_t tracktofNsigmaElectron;
Double_t tracktofNsigmaKaon;
Double_t tracktofNsigmaProton;
Double_t tracktofNsigmaPion;
Double_t tracktofNsigmaDeuteron;
Double_t tracktofNsigmaTrition;
Double_t tracktofNsigmaHelium3;

Double_t track_ExpectedTPCSignalV0_el;
Double_t track_ExpectedTPCSignalV0_pi;
Double_t track_ExpectedTPCSignalV0_pro;
//Double_t track_ExpectedTPCSignalV0_ka;
//Double_t track_ExpectedTPCSignalV0_deut;

Double_t track_CorrectedTPCSignalV0_el;
Double_t track_CorrectedTPCSignalV0_pi;
Double_t track_CorrectedTPCSignalV0_pro;
Double_t track_CorrectedTPCSignalV0_ka;
Double_t track_CorrectedTPCSignalV0_deut;
Double_t track_CorrectedTPCSignalV0_tri;
Double_t track_CorrectedTPCSignalV0_He3;

Double_t track_GetPileupValue;
Double_t dca_r;
Double_t dca_z;
Double_t nCrossRows;

//Double_t run;
//Double_t intrate;
//Double_t timeStampS;
//Double_t timestamp;
//Double_t bField;
//Double_t triggerMask;
//Double_t gid;
//Double_t shiftA;
//Double_t shiftC;
//Double_t shiftM;
//Double_t nPileUpPrim;
//Double_t nPileUpSum;
//Double_t primMult;
//Double_t tpcClusterMult;
//Double_t pileUpOK;
//Double_t multSSD;
//Double_t multSDD;
Double_t multSPD;
//Double_t multV0;
//Double_t multT0;
//Double_t spdvz;
//Double_t itsTracklets;
//Double_t tpcMult;
Double_t tpcTrackBeforeClean;
Double_t centV0;
//Double_t centITS0;
//Double_t centITS1;
Double_t ITSRefit;
Double_t TPCRefit;
Double_t Nucleitrigger_OFF;
//Double_t isMinBias;
Double_t isPileUp;

//List of Branches;

TBranch *b_fTPCsignal;					//!
TBranch *b_fITSsignal;					//!
//TBranch *b_fTRDsignal;					//!
TBranch *b_trackP;					//!
TBranch *b_tgl;						//!
TBranch *b_trackEta;					//!
TBranch *b_tpc_cls;					//!
 TBranch *b_fTPCsignalN;                                //!
TBranch *b_ntracks;					//!
//TBranch *b_trackstatus;					//!
TBranch *b_track_hasTOF;				//!
TBranch *b_track_tpcNsigma_el;				//!
TBranch *b_track_tpcNsigma_pi;				//!
TBranch *b_track_tpcNsigma_pro;				//!
TBranch *b_track_tpcNsigma_ka;				//!
TBranch *b_track_tpcNsigma_deut;                          //!
TBranch *b_track_tpcNsigma_tri;                        //!
TBranch *b_track_tpcNsigma_He3;                        //!

TBranch *b_tracktofNsigmaElectron;			//!
TBranch *b_tracktofNsigmaKaon;				//!
TBranch *b_tracktofNsigmaProton;			//!
TBranch *b_tracktofNsigmaPion;				//!
TBranch *b_tracktofNsigmaDeuteron;                          //!
TBranch  *b_tracktofNsigmaTrition;			//!
TBranch *b_tracktofNsigmaHelium3;			//!

TBranch *b_track_ExpectedTPCSignalV0_el;		//!
TBranch *b_track_ExpectedTPCSignalV0_pi;		//!
TBranch *b_track_ExpectedTPCSignalV0_pro;		//!
//TBranch *b_track_ExpectedTPCSignalV0_ka;		//!
//TBranch *b_track_ExpectedTPCSignalV0_deut;                //!

TBranch *b_track_CorrectedTPCSignalV0_el;		//!
TBranch *b_track_CorrectedTPCSignalV0_pi;		//!
TBranch *b_track_CorrectedTPCSignalV0_pro;		//!
TBranch *b_track_CorrectedTPCSignalV0_ka;		//!
TBranch *b_track_CorrectedTPCSignalV0_deut;               //!
TBranch *b_track_CorrectedTPCSignalV0_tri;             //!
TBranch *b_track_CorrectedTPCSignalV0_He3;            //!
 
TBranch *b_track_GetPileupValue;			//!
 TBranch *b_dca_r;                                      //!
 TBranch *b_dca_z;                                      //!
 TBranch * b_nCrossRows;                                //!

//TBranch *b_run;						//!
//TBranch *b_intrate;					//!
//TBranch *b_timeStampS;                                  //!
//TBranch *b_timestamp;                                   //!
//TBranch *b_bField;					//!
//TBranch *b_triggerMask;					//!
//TBranch *b_gid;						//!
//TBranch *b_shiftA;					//!
//TBranch *b_shiftC;					//!
//TBranch *b_shiftM;					//!
//TBranch *b_nPileUpPrim;					//!
//TBranch *b_nPileUpSum;					//!
//TBranch *b_primMult;					//!
//TBranch *b_tpcClusterMult;				//!
//TBranch *b_pileUpOK;					//!
//TBranch *b_multSSD;					//!
//TBranch *b_multSDD;					//!
TBranch *b_multSPD;					//!
//TBranch *b_multV0;					//!
//TBranch *b_multT0;					//!
//TBranch *b_spdvz;					//!
//TBranch *b_itsTracklets;				//!
//TBranch *b_tpcMult;					//!
TBranch *b_tpcTrackBeforeClean;			//!
TBranch *b_centV0;					//!
TBranch *b_centITS0;					//!
TBranch *b_centITS1;					//!
TBranch *b_ITSRefit;					//!
TBranch *b_TPCRefit;					//!
TBranch *b_Nucleitrigger_OFF;				//!
//TBranch *b_isMinBias;					//!
TBranch *b_isPileUp;          //!

  TrackFlatAna();
  ~TrackFlatAna();

  virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
  
  void execute();
  ClassDef(TrackFlatAna, 1);
};


#endif
