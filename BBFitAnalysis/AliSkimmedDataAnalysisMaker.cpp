#include <iostream>
#include <vector>

#include "V0FlatAna.h"
//#include "AliESDtrack.h"
#include "TrackFlatAna.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TF1.h"
#include "THnSparse.h"
#include "TVectorD.h"
#include "TSpline.h"
//#include "THnSparseF.h"
#include "AliSkimmedDataAnalysisMaker.h"

using namespace std;

ClassImp(AliSkimmedDataAnalysisMaker);

AliSkimmedDataAnalysisMaker::AliSkimmedDataAnalysisMaker( const char * outName_Sparse, const char * outName_Tree) {

  // TFile* inputFile = new TFile("V0tree.root","READ");
  // TTree *tree_v0=  (TTree *) inputFile->Get("V0Flat");

  // V0FlatAna *V0ana=new V0FlatAna();
  // V0ana->Init(tree_v0);

  // V0ana->Loop();

  cout<< outName_Sparse<<endl;
  mOutputFile=new TFile(outName_Sparse,"RECREATE");
  
  mOutputFile_eta = new TFile(outName_Tree,"RECREATE"); 

  mOutputFile_BBFitAnalysis = new TFile("TPCTree_BBFitAnalysis.root","RECREATE"); 
}
AliSkimmedDataAnalysisMaker::~AliSkimmedDataAnalysisMaker(){
  //..
  //..
}
void AliSkimmedDataAnalysisMaker::bookHistogram()
{

  fTree = new TTree("fTree", "Tree for analysis of #eta dependence of TPC signal");
  fTree->Branch("pTPC", &fPtpc);
  //fTree->Branch("pT", &fPt);
  fTree->Branch("dEdx", &fDeDx);
  fTree->Branch("dEdxExpected", &fDeDxExpected);
  fTree->Branch("tanTheta", &fTanTheta);
  //fTree->Branch("sinAlpha", &fSinAlpha);
  //fTree->Branch("y", &fY);
  //TODO fTree->Branch("phiPrime", &fPhiPrime);
  fTree->Branch("tpcSignalN", &fTPCsignalN);
  fTree->Branch("tpcSignalNsubthreshold", &fTPCsignalNsubthreshold);
  fTree->Branch("numTPCClustersInActiveVolume", &fNumTPCClustersInActiveVolume);
  fTree->Branch("pidType", &fPIDtype);
  fTree->Branch("fMultiplicity", &fMultiplicity);


  mh2pTdEdxVspT=new TH2F("mh2pTdEdxVspT","",1500,0,15,500,0,1000);
  mh2pTdEdxVspT_K0Like=new TH2F("mh2pTdEdxVspT_K0Like","",1500,0,15,500,0,1000);
  mh2pTdEdxVspT_ELike=new TH2F("mh2pTdEdxVspT_ELike","",1500,0,15,500,0,1000);
  mh2pTdEdxVspT_LLike=new TH2F("mh2pTdEdxVspT_LLike","",200,0,4,100,0,200);
  mh2pTdEdxVspT_ALLike=new TH2F("mh2pTdEdxVspT_ALLike","",1500,0,15,500,0,1000);

  mh2dEdxVspT_Electron=new TH2F("mh2dEdxVspT_Electron","",1500,0,15,500,0,1000);
  mh2dEdxVspT_Pion=new TH2F("mh2dEdxVspT_Pion","",1500,0,15,500,0,1000);
  mh2dEdxVspT_Proton=new TH2F("mh2dEdxVspT_Proton","",100,0,25000,200,0,200);

  mh1p = new TH1F("test","test",150,-5,5);

  // Pileup event QA
  h_pileupcorrelation = new TH2F("PileupCorrelation","PileupCorrelation",1000,0,6e6,100,0,50000);
  h_pileupcorrelation_bc = new TH2F("PileUpCorrelation_bc","PileUpCorrelation_bc",1000,0,6e6,100,0,50000);
  h_Centrality = new TH1F("h_Centrality_binbias","h_Centrality_minBias",100,0,100);
  h_Centrality_all = new TH1F("h_Centrality_all","h_Centrality_all",100,0,100);

  
  //title.Append(";p (GeV/c);tpc signal;particle ID;assumed particle;nSigmaTPC;nSigmaTOF;centrality");
  TString  name="fHistPidQA";
  TString  title="fHistPidQA";

  //SetAxisNamesFromTitle(fThnspTpc);
  // BinLogAxis(fThnspTpc, 8);
  
  bool IsPbPb=true;
  const Int_t kNdim = 8;
  Int_t    binsHistQA[kNdim] = {135, 1980,    4,    8, 40, 10,  IsPbPb ? 40 : 40,10};
  Double_t xminHistQA[kNdim] = {0.1,   20., -0.5, -0.5, -10., -5.,   0.,0};
  Double_t xmaxHistQA[kNdim] = {50., 2000.,  3.5,  7.5,  10.,  5., IsPbPb ? 25000. : 4000.,1};
  fHistPidQA = new THnSparseF(name.Data(), title.Data(), kNdim, binsHistQA, xminHistQA, xmaxHistQA);

  BinLogAxis(fHistPidQA, 0);
  SetAxisNamesFromTitle(fHistPidQA);

  
  
  const Int_t kNdim_separation_power = 5;
  //p,dedx,eta,centrality,particle
  Int_t    binsHist_separation_power[kNdim_separation_power] = {15, 1980,   20,    100, 4}; 
  Double_t xminHist_separation_power[kNdim_separation_power] = {0,   20.,   -1,    0,  0.};
  Double_t xmaxHist_separation_power[kNdim_separation_power] = {15., 2000.,  1.,  100,  4.};
  fHistPid_separation_power = new THnSparseF("fHistPid_separation_power","fHistPid_separation_power", kNdim_separation_power, binsHist_separation_power, xminHist_separation_power, xmaxHist_separation_power);
  

  //tree for BBfit analysis

for (Int_t i = 0; i<7; i++){
  tpcNsig[i]=-999;
  tofNsig[i]=-999;
}
  fTree2 = new TTree("fTree2", "Tree for analysis of BetheBloch fit");
  fTree2->Branch("p", &p);
  fTree2->Branch("oneoverpt",&oneoverpt);
  fTree2->Branch("rawtpcsignal",&rawtpcsignal);
  fTree2->Branch("itssignal",&itssignal);
  fTree2->Branch("pileupcor_tpcsignal",&pileupcor_tpcsignal);
  fTree2->Branch("BG", &BG);
  fTree2->Branch("eta",&eta);
  fTree2->Branch("tgl",&tgl);
  fTree2->Branch("multiplicty",&multiplicty);
  fTree2->Branch("centrality", &centrality);
  fTree2->Branch("PDGcode", &PDGcode);
  fTree2->Branch("isV0",&isV0);
  fTree2->Branch("isPileUp",&isPileUp);
  fTree2->Branch("dEdxExpected_SatLund_woDeut",&dEdxExpected_SatLund_woDeut);
  fTree2->Branch("dEdxExpected_SatLund_wDeut",&dEdxExpected_SatLund_wDeut);
  fTree2->Branch("dEdxExpected_ALEPH_woDeut",&dEdxExpected_ALEPH_woDeut);
  fTree2->Branch("dEdxExpected_ALEPH_wDeut",&dEdxExpected_ALEPH_wDeut);
  fTree2->Branch("dEdxExpected_SatLund_woDeut_pileUpCut",&dEdxExpected_SatLund_woDeut_pileUpCut);
  //fTree2->Branch("tpcnsigma",&tpcnsigma);
  //fTree2->Branch("tofnsigma",&tofnsigma);
  fTree2->Branch("tpcNsig",&tpcNsig);
  fTree2->Branch("tofNsig",&tofNsig);
  fTree2->Branch("shiftM",&shiftM);
  fTree2->Branch("nPileUpPrim", &nPileUpPrim);
  fTree2->Branch("nPileUpSum", &nPileUpSum);
  fTree2->Branch("primMult",&primMult);
  fTree2->Branch("tpcClusterMult", &tpcClusterMult);
  fTree2->Branch("pileUpOK", &pileUpOK);
  fTree2->Branch("multSSD", &multSSD);
  fTree2->Branch("multSDD", &multSDD);
  fTree2->Branch("SignalTot0", &SignalTot0);
  fTree2->Branch("SignalTot1", &SignalTot1);
  fTree2->Branch("SignalTot2", &SignalTot2);
  fTree2->Branch("SignalTot3", &SignalTot3);
  fTree2->Branch("SignalMax0", &SignalMax0);
  fTree2->Branch("SignalMax1", &SignalMax1);
  fTree2->Branch("SignalMax2", &SignalMax2);
  fTree2->Branch("SignalMax3", &SignalMax3);
  fTree2->Branch("SignalNcl0", &SignalNcl0);
  fTree2->Branch("SignalNcl1", &SignalNcl1);
  fTree2->Branch("SignalNcl2", &SignalNcl2);
  fTree2->Branch("SignalNcl3", &SignalNcl3);
  fTree2->Branch("SignalNcr0", &SignalNcr0);
  fTree2->Branch("SignalNcr1", &SignalNcr1);
  fTree2->Branch("SignalNcr2", &SignalNcr2);
  fTree2->Branch("SignalNcr3", &SignalNcr3);
  fTree2->Branch("tpc_ncls", &tpc_ncls);
  fTree2->Branch("its_ncls", &its_ncls);
  fTree2->Branch("tpc_chi2", &tpc_chi2);
  fTree2->Branch("its_chi2", &its_chi2);


  //  THnSparseF* dEdx_THnF = new THnSparseF(name.Data(), title.Data(), kNdim, binsHistQA, xminHistQA, xmaxHistQA);
  // BinLogAxis(h, 0);
}
void AliSkimmedDataAnalysisMaker::TreeV0_BBFitAnalysis(TString filename_v0){

  cout<<"!!!!!!!!!!!!!!!!!read"<<filename_v0<<endl;
  TFile * infile2=new TFile(filename_v0.Data());
  TTree* Tree2=(TTree*) infile2->Get("V0Flat");
  V0FlatAna *V0ana=new V0FlatAna();
  V0ana->Init(Tree2);

  //Load BB fits
  TFile *fSpl_SatLund_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund.root");
  TSpline3 *spline_SatLund_woDeut_p = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_k = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pi = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_e = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_all = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_SatLund_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLundwDeut.root");
  TSpline3 *spline_SatLund_wDeut_p = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_k = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_pi = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_e = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_all = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_ALEPH_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPH.root");
  TSpline3 *spline_ALEPH_woDeut_p = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_k = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_pi = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_e = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_all = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");  

  TFile *fSpl_ALEPH_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPHwDeut.root");
  TSpline3 *spline_ALEPH_wDeut_p = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_k = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_pi = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_e = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_all = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_SatLund_woDeut_pileupCut = new TFile("/lustre/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund_woDeut_pileupCut.root");
  TSpline3 *spline_SatLund_woDeut_pileupCut_p = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_k = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_pi = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_e = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_all = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN"); 



for(Int_t iV0=0;iV0<Tree2->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
        cout<<"working on Event  "<<iV0<<endl;
      V0ana->GetEntry(iV0);

      Int_t particleID = -1;
      Int_t  alicePID=-1;
      Double_t tpcsignal = -1.;
      Double_t tpcsignal_corrected = -1;
      Double_t tanTheta = -999.;
      Double_t precin = -1.;
      Double_t  nTotESDTracks=-999;
      Double_t tofNsigma=-9999;
      Double_t tpcNsigma=-9999;
      Double_t tpccluster=-999;
      Double_t dca_r = -999;
      Double_t dca_z = -999;
      Double_t nCrossRows = -999;
      tgl = -999;
      tofnsigma = -999;
      tpcnsigma = -999;
      shiftM = -999;
      nPileUpPrim= -999;
      nPileUpSum= -999;
      primMult= -999;
      tpcClusterMult= -999;
      pileUpOK= -999;
      multSSD= -999;
      multSDD= -999;
      SignalTot0 = -999;
      SignalTot1 = -999;
      SignalTot2 = -999;
      SignalTot3 = -999;
      SignalMax0 = -999;
      SignalMax1 = -999;
      SignalMax2 = -999;
      SignalMax3 = -999;
      SignalNcl0 = -999;
      SignalNcl1 = -999;
      SignalNcl2 = -999;
      SignalNcl3 = -999;
      SignalNcr0 = -999;
      SignalNcr1 = -999;
      SignalNcr2 = -999;
      SignalNcr3 = -999;

      const Int_t numCases = 8;
      Double_t tpcQA[numCases];
      Double_t tofQA[numCases];
      for (Int_t i = 0; i < numCases; i++) {
	    tpcQA[i] = -999.;
	    tofQA[i] = -999.;
      }



      Double_t processedTPCsignal[8] = { 1,1,1,1,1,1,1,1 };
      Double_t deltaTPCsignal[8] = { -111,-111,-111,-111,-111,-111,-111,-111 };

      for (Int_t track = 0; track < 2; track++) {


	bool isTreeK0=false;
	bool isTreeGamma=false;
	bool isTreeLambda=false;
	bool isTreeALambda=false;
  tofnsigma = -999;
  tpcnsigma = -999;

	if(V0ana->K0Like>0.0&&(V0ana->cleanK0==1))
	  {
	    isTreeK0=true;   
	  }
	if(V0ana->ELike>0.0&&(V0ana->cleanGamma==1))
	  {
	    isTreeGamma=true;   
	  }

	if(V0ana->LLike>0.0&&(V0ana->cleanLambda==1))
	  {
	    isTreeLambda=true;   
	  }

	if(V0ana->ALLike>0.0&&(V0ana->cleanALambda==1))
	  {
	    isTreeALambda=true;   
	  }

	if(!(isTreeK0||isTreeGamma||isTreeLambda||isTreeALambda)) continue;

	if(track==0)
	  {

  tpccluster=V0ana->track0_fTPCsignalN;
  p = V0ana->track0P;
  oneoverpt = 1/V0ana->track0Pt;
  rawtpcsignal = V0ana->track0_fTPCsignal;
  itssignal = V0ana->track0_fITSsignal;
  pileupcor_tpcsignal = V0ana->track0_fTPCsignal - V0ana->track0GetPileupValue;
  eta = V0ana->track0Eta;
  tgl = V0ana->track0Tgl;
  multiplicty = V0ana->tpcTrackBeforeClean;
  centrality = V0ana->centV0;
  isV0 = 1.;
  isPileUp = V0ana->isPileUp;
  dca_r = V0ana->dca0_r;
	dca_z = V0ana->dca0_z;
  nCrossRows = V0ana->nCrossRows0;
  
  shiftM = V0ana->shiftM;
  nPileUpPrim= V0ana->nPileUpPrim;
  nPileUpSum= V0ana->nPileUpSum;
  primMult= V0ana->primMult;
  tpcClusterMult= V0ana->tpcClusterMult;
  pileUpOK= V0ana->pileUpOK;
  multSSD= V0ana->multSSD;
  multSDD= V0ana->multSDD;
  SignalTot0 = V0ana->logSignalTot_track0_0;
  SignalTot1 = V0ana->logSignalTot_track0_1;
  SignalTot2 = V0ana->logSignalTot_track0_2;
  SignalTot3 = V0ana->logSignalTot_track0_3;

  SignalMax0 = V0ana->logSignalMax_track0_0;
  SignalMax1 = V0ana->logSignalMax_track0_1;
  SignalMax2 = V0ana->logSignalMax_track0_2;
  SignalMax3 = V0ana->logSignalMax_track0_3;

  SignalNcl0 = V0ana->signalNcl_track0_0;
  SignalNcl1 = V0ana->signalNcl_track0_1;
  SignalNcl2 = V0ana->signalNcl_track0_2;
  SignalNcl3 = V0ana->signalNcl_track0_3;

  SignalNcr0 = V0ana->signalNcr_track0_0;
  SignalNcr1 = V0ana->signalNcr_track0_1;
  SignalNcr2 = V0ana->signalNcr_track0_2;
  SignalNcr3 = V0ana->signalNcr_track0_3;

  tpc_ncls = V0ana->tpc_cls0; 
  its_ncls = V0ana->its_cls0;
  tpc_chi2 = V0ana->tpc_chi2_0;
  its_chi2 = V0ana->its_chi2_0;
  
  tpcQA[0]=V0ana->track0tpcNsigma_el;
	tofQA[0]=V0ana->track0tofNsigmaElectron;
  tpcNsig[0] = V0ana->track0tpcNsigma_el;
  tofNsig[0] = V0ana->track0tofNsigmaElectron;

	tpcQA[1]=V0ana->track0tpcNsigma_pi;
	tofQA[1]=V0ana->track0tofNsigmaPion;
  tpcNsig[2]=V0ana->track0tpcNsigma_pi;
	tofNsig[2]=V0ana->track0tofNsigmaPion;

	tpcQA[2]=V0ana->track0tpcNsigma_ka;
	tofQA[2]=V0ana->track0tofNsigmaKaon;
	tpcNsig[3]=V0ana->track0tpcNsigma_ka;
	tofNsig[3]=V0ana->track0tofNsigmaKaon;

	tpcQA[3]=V0ana->track0tpcNsigma_pro;
	tofQA[3]=V0ana->track0tofNsigmaProton;
  tpcNsig[4]=V0ana->track0tpcNsigma_pro;
	tofNsig[4]=V0ana->track0tofNsigmaProton;

	tpcQA[4]=V0ana->track0tpcNsigma_ka;
	tofQA[4]=V0ana->track0tofNsigmaKaon;
  tpcNsig[3]=V0ana->track0tpcNsigma_ka;
	tofNsig[3]=V0ana->track0tofNsigmaKaon;

	tpcQA[5] = -999;
  tofQA[5] = -999;
  tpcNsig[5] = -999;
  tofNsig[5] = -999;

  tpcQA[6] = -999;
  tofQA[6] = -999;
  tpcNsig[6] = -999;
  tofNsig[6] = -999;

  tpcQA[7] = -999;
  tofQA[7] = -999;

  tofnsigma = -999;
  tpcnsigma = -999;
  
	  }
	else if(track==1)
	  {

 	tpccluster=V0ana->track1_fTPCsignalN;
  p = V0ana->track1P;
  oneoverpt = 1/V0ana->track1Pt;
  rawtpcsignal = V0ana->track1_fTPCsignal;
  itssignal = V0ana->track1_fITSsignal;
  pileupcor_tpcsignal = V0ana->track1_fTPCsignal - V0ana->track1GetPileupValue;
  eta = V0ana->track1Eta;
  tgl = V0ana->track1Tgl;
  multiplicty = V0ana->tpcTrackBeforeClean;
  centrality = V0ana->centV0;
  isV0 = 1.;
  isPileUp = V0ana->isPileUp;
  dca_r = V0ana->dca1_r;
  dca_z = V0ana->dca1_z;
  nCrossRows = V0ana->nCrossRows1;

  shiftM = V0ana->shiftM;
  nPileUpPrim= V0ana->nPileUpPrim;
  nPileUpSum= V0ana->nPileUpSum;
  primMult= V0ana->primMult;
  tpcClusterMult= V0ana->tpcClusterMult;
  pileUpOK= V0ana->pileUpOK;
  multSSD= V0ana->multSSD;
  multSDD= V0ana->multSDD;
  
  SignalTot0 = V0ana->logSignalTot_track1_0;
  SignalTot1 = V0ana->logSignalTot_track1_1;
  SignalTot2 = V0ana->logSignalTot_track1_2;
  SignalTot3 = V0ana->logSignalTot_track1_3;

  SignalMax0 = V0ana->logSignalMax_track1_0;
  SignalMax1 = V0ana->logSignalMax_track1_1;
  SignalMax2 = V0ana->logSignalMax_track1_2;
  SignalMax3 = V0ana->logSignalMax_track1_3;

  SignalNcl0 = V0ana->signalNcl_track1_0;
  SignalNcl1 = V0ana->signalNcl_track1_1;
  SignalNcl2 = V0ana->signalNcl_track1_2;
  SignalNcl3 = V0ana->signalNcl_track1_3;

  SignalNcr0 = V0ana->signalNcr_track1_0;
  SignalNcr1 = V0ana->signalNcr_track1_1;
  SignalNcr2 = V0ana->signalNcr_track1_2;
  SignalNcr3 = V0ana->signalNcr_track1_3;

  tpc_ncls = V0ana->tpc_cls1; 
  its_ncls = V0ana->its_cls1;
  tpc_chi2 = V0ana->tpc_chi2_1;
  its_chi2 = V0ana->its_chi2_1;

  tpcQA[0]=V0ana->track1tpcNsigma_el;
	tofQA[0]=V0ana->track1tofNsigmaElectron;
  tpcNsig[0] = V0ana->track1tpcNsigma_el;
  tofNsig[0] = V0ana->track1tofNsigmaElectron;

	tpcQA[1]=V0ana->track1tpcNsigma_pi;
	tofQA[1]=V0ana->track1tofNsigmaPion;
  tpcNsig[2]=V0ana->track1tpcNsigma_pi;
	tofNsig[2]=V0ana->track1tofNsigmaPion;

	tpcQA[2]=V0ana->track1tpcNsigma_ka;
	tofQA[2]=V0ana->track1tofNsigmaKaon;
	tpcNsig[3]=V0ana->track1tpcNsigma_ka;
	tofNsig[3]=V0ana->track1tofNsigmaKaon;

	tpcQA[3]=V0ana->track1tpcNsigma_pro;
	tofQA[3]=V0ana->track1tofNsigmaProton;
  tpcNsig[4]=V0ana->track1tpcNsigma_pro;
	tofNsig[4]=V0ana->track1tofNsigmaProton;

	tpcQA[4]=V0ana->track1tpcNsigma_ka;
	tofQA[4]=V0ana->track1tofNsigmaKaon;
  tpcNsig[3]=V0ana->track1tpcNsigma_ka;
	tofNsig[3]=V0ana->track1tofNsigmaKaon;

	tpcQA[5] = -999;
  tofQA[5] = -999;
  tpcNsig[5] = -999;
  tofNsig[5] = -999;

  tpcQA[6] = -999;
  tofQA[6] = -999;
  tpcNsig[6] = -999;
  tofNsig[6] = -999;

  tpcQA[7] = -999;
  tofQA[7] = -999;

  tofnsigma = -999;
  tpcnsigma = -999;

	  }
	

      if (tpccluster < 50) continue;
      if(TMath::Abs(dca_r) > 3.0) continue;
      if(TMath::Abs(dca_z) > 3.0) continue;
      if(nCrossRows < 70) continue;
	
	if (isTreeK0) {
	  particleID = 2; //pion
	  //	  alicePID=AliPID::kPion;
	  PDGcode=2;
    BG = p/1.39569997787475586e-01;
    dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
    dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
    dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
    dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
    dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
    tofnsigma = tofQA[1];
    tpcnsigma = tpcQA[1];
	  
	} else if (isTreeGamma) {
	  particleID = 1; // electron
	  //  alicePID=AliPID::kElectron;
	  PDGcode=0;//AliPID::kElectron;
    BG = p/5.10998885147273540e-04;
    dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
    dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
    dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
    dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
    dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
    tofnsigma = tofQA[0];
    tpcnsigma = tpcQA[0];

	} else if (isTreeLambda) {
	  if (track == 0) {
	    particleID = 3; // proton
	    //	    alicePID=AliPID::kProton;
	    PDGcode=4.;
      BG = p/9.38271999359130859e-01;
      dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
      dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
      dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
      dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
      dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
      tofnsigma = tofQA[3];
      tpcnsigma = tpcQA[3];
	  } else {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    PDGcode=2;//AliPID::kPion;
      BG = p/1.39569997787475586e-01;
      dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
      dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
      dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
      dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
      dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
      tofnsigma = tofQA[1];
      tpcnsigma = tpcQA[1];
	  }
	}
	else if (isTreeALambda) {
	  if (track == 0) {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    PDGcode=2.;//AliPID::kPion;
      BG = p/1.39569997787475586e-01;
      dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
      dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
      dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
      dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
      dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
      tofnsigma = tofQA[1];
      tpcnsigma = tpcQA[1];

	  } else {
	    particleID = 3; // proton
	    //alicePID=AliPID::kProton;
	    PDGcode=4;;
      BG = p/9.38271999359130859e-01;
      dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
      dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
      dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
      dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
      dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
      tofnsigma = tofQA[3];
      tpcnsigma = tpcQA[3];
	  }
	}
	       
  fTree2->Fill();

}    // end track loop

    } // end treeloop


} //end function


void AliSkimmedDataAnalysisMaker::TreePrimary_BBFitAnalysis(TString filename_track){

 cout<<"!!!!!!!!!!!!!!!!!read!!!!!!!"<<filename_track<<endl;
  TFile * infile=new TFile(filename_track.Data());
  TTree* Tree=(TTree*) infile->Get("CleanTrackFlat");
    if(Tree==0x0){
  } else {                                                                                                                                                                         
  TrackFlatAna *TrackAna=new TrackFlatAna();
  TrackAna->Init(Tree);

  TFile *fSpl_SatLund_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund.root");
  TSpline3 *spline_SatLund_woDeut_p = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_k = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pi = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_e = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_all = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_SatLund_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLundwDeut.root");
  TSpline3 *spline_SatLund_wDeut_p = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_k = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_pi = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_e = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_all = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_ALEPH_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPH.root");
  TSpline3 *spline_ALEPH_woDeut_p = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_k = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_pi = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_e = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_all = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");  

  TFile *fSpl_ALEPH_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPHwDeut.root");
  TSpline3 *spline_ALEPH_wDeut_p = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_k = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_pi = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_e = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_all = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  TFile *fSpl_SatLund_woDeut_pileupCut = new TFile("/lustre/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund_woDeut_pileupCut.root");
  TSpline3 *spline_SatLund_woDeut_pileupCut_p = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_k = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_pi = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_e = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_all = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");


  for(Int_t iV0=0;iV0 < Tree->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
        cout<<"working on Event  "<<iV0<<endl;
      TrackAna->GetEntry(iV0);
  
     const Int_t numCases = 8;
     Double_t tpcQA[numCases];
     Double_t tofQA[numCases];
     for (Int_t i = 0; i < numCases; i++) {
       tpcQA[i] = -999.;
      tofQA[i] = -999.;
    }

  Double_t tpccluster=-999;
  p = -999;
  oneoverpt = -999;
  rawtpcsignal = -999;
  itssignal = -999;
  pileupcor_tpcsignal = -999;
  eta = -999;
  multiplicty = -999;
  centrality = -999;
  isV0 = 0.;
  PDGcode = -999;
  tgl = -999;
  BG = -999;
  tofnsigma = -999;
  tpcnsigma = -999;
  Double_t dca_r = -999;
	Double_t dca_z = -999;
  Double_t nCrossRows = -999;

      shiftM = -999;
      nPileUpPrim= -999;
      nPileUpSum= -999;
      primMult= -999;
      tpcClusterMult= -999;
      pileUpOK= -999;
      multSSD= -999;
      multSDD= -999;
      SignalTot0 = -999;
      SignalTot1 = -999;
      SignalTot2 = -999;
      SignalTot3 = -999;
      SignalMax0 = -999;
      SignalMax1 = -999;
      SignalMax2 = -999;
      SignalMax3 = -999;
      SignalNcl0 = -999;
      SignalNcl1 = -999;
      SignalNcl2 = -999;
      SignalNcl3 = -999;
      SignalNcr0 = -999;
      SignalNcr1 = -999;
      SignalNcr2 = -999;
      SignalNcr3 = -999;
      tpc_ncls = -999; 
      its_ncls = -999;
      tpc_chi2 = -999;
      its_chi2 = -999;

      tpccluster = TrackAna->esdTrack_fTPCsignalN;
    eta = TrackAna->trackEta;
    dca_r = TrackAna->dca_r;
    dca_z = TrackAna->dca_z;
    nCrossRows = TrackAna->nCrossRows;

  if (tpccluster < 50) continue; 
  if(TMath::Abs(eta) > 0.9) continue;                                                                                                                                                                   
  if(TMath::Abs(dca_r) > 3.0) continue;
  if(TMath::Abs(dca_z) > 3.0) continue;
  if(nCrossRows < 70) continue;

  p = TrackAna->trackP;
  oneoverpt = 1/TrackAna->trackPt;
  rawtpcsignal = TrackAna->fTPCsignal;
  itssignal = TrackAna->esdTrack_fITSsignal;
  pileupcor_tpcsignal = TrackAna->fTPCsignal - TrackAna->track_GetPileupValue;
  multiplicty = TrackAna->tpcTrackBeforeClean;
  centrality = TrackAna->centV0;
  isPileUp = TrackAna->isPileUp;
  tgl = TrackAna->tgl;

  shiftM = TrackAna->shiftM;
  nPileUpPrim= TrackAna->nPileUpPrim;
  nPileUpSum= TrackAna->nPileUpSum;
  primMult= TrackAna->primMult;
  tpcClusterMult= TrackAna->tpcClusterMult;
  pileUpOK= TrackAna->pileUpOK;
  multSSD= TrackAna->multSSD;
  multSDD= TrackAna->multSDD;
  SignalTot0 = TrackAna->logSignalTot0;
  SignalTot1 = TrackAna->logSignalTot1;
  SignalTot2 = TrackAna->logSignalTot2;
  SignalTot3 = TrackAna->logSignalTot3;

  SignalMax0 = TrackAna->logSignalMax0;
  SignalMax1 = TrackAna->logSignalMax1;
  SignalMax2 = TrackAna->logSignalMax2;
  SignalMax3 = TrackAna->logSignalMax3;

  SignalNcl0 = TrackAna->signalNcl0;
  SignalNcl1 = TrackAna->signalNcl1;
  SignalNcl2 = TrackAna->signalNcl2;
  SignalNcl3 = TrackAna->signalNcl3;

  SignalNcr0 = TrackAna->signalNcr0;
  SignalNcr1 = TrackAna->signalNcr1;
  SignalNcr2 = TrackAna->signalNcr2;
  SignalNcr3 = TrackAna->signalNcr3;

  tpc_ncls = TrackAna->tpc_cls; 
  its_ncls = TrackAna->its_cls;
  tpc_chi2 = TrackAna->tpc_chi2;
  its_chi2 = TrackAna->its_chi2;

  tpcQA[0]=TrackAna->track_tpcNsigma_el;
  tofQA[0]=TrackAna->tracktofNsigmaElectron;
  tpcNsig[0]=TrackAna->track_tpcNsigma_el;
  tofNsig[0]=TrackAna->tracktofNsigmaElectron;

  tpcQA[1]=TrackAna->track_tpcNsigma_pi;
  tofQA[1]=TrackAna->tracktofNsigmaPion;
  tpcNsig[2]=TrackAna->track_tpcNsigma_pi;
  tofNsig[2]=TrackAna->tracktofNsigmaPion;

  tpcQA[2]=TrackAna->track_tpcNsigma_ka;
  tofQA[2]=TrackAna->tracktofNsigmaKaon;
  tpcNsig[3]=TrackAna->track_tpcNsigma_ka;
  tofNsig[3]=TrackAna->tracktofNsigmaKaon;

  tpcQA[3]=TrackAna->track_tpcNsigma_pro;
  tofQA[3]=TrackAna->tracktofNsigmaProton;
  tpcNsig[4]=TrackAna->track_tpcNsigma_pro;
  tofNsig[4]=TrackAna->tracktofNsigmaProton;

  tpcQA[4]=TrackAna->track_tpcNsigma_ka;
  tofQA[4]=TrackAna->tracktofNsigmaKaon;
  tpcNsig[3]=TrackAna->track_tpcNsigma_ka;
  tofNsig[3]=TrackAna->tracktofNsigmaKaon;

  tpcQA[5]= TrackAna->track_tpcNsigma_deut;
	tofQA[5]= TrackAna->tracktofNsigmaDeuteron;
  tpcNsig[5]= TrackAna->track_tpcNsigma_deut;
	tofNsig[5]= TrackAna->tracktofNsigmaDeuteron;

	tpcQA[6]= TrackAna->track_tpcNsigma_tri;
  tofQA[6]= TrackAna->tracktofNsigmaTrition;
  tpcNsig[6]= TrackAna->track_tpcNsigma_tri;
  tofNsig[6]= TrackAna->tracktofNsigmaTrition;

  //assign PDG code to different particles:


  // pion
  if ((p > 0.5 && p < 1.1 && TMath::Abs(tofQA[1]) < 3.0 && TMath::Abs(tpcQA[1]) <4.0) || p < 0.5 &&TMath::Abs(tpcQA[1]) <4.0){
  PDGcode = 2;
  BG = p/1.39569997787475586e-01;
  dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
  dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
  dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
  dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
  dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
  tofnsigma = tofQA[1];
  tpcnsigma = tpcQA[1];
  }



  //proton
  if ((p < 3.0 && p > 0.6 && TMath::Abs(tofQA[3]) <3.0 && (rawtpcsignal >= 50. / TMath::Power(p, 1.3)))|| (p < 0.6)&&(rawtpcsignal >= 50. / TMath::Power(p, 1.3))){
  if (p > 1.0 ){
    if(TrackAna->Nucleitrigger_OFF == 0) continue;
  }
  PDGcode = 4;
  BG = p/9.38271999359130859e-01;
  dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
  dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
  dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
  dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
  dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
  tofnsigma = tofQA[3];
  tpcnsigma = tpcQA[3];
  }


  //Deuteron
  if ((p < 1.5 && TMath::Abs(tpcQA[5]) < 5.0 ) || (p<2.0 && p>1.5 &&TMath::Abs(tofQA[5])<3.0 && TMath::Abs(tpcQA[5]) < 5.0 )){
  PDGcode = 5;
  BG = p/1.87561297416687012e+00;
  dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
  dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
  dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
  dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
  dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
  tofnsigma = tofQA[5];
  tpcnsigma = tpcQA[5];
  }

//Trition
  if ((p < 1.5 && TMath::Abs(tpcQA[6]) < 5.0 ) || (p<2.0 && p>1.5 &&TMath::Abs(tofQA[6])<3.0 && TMath::Abs(tpcQA[6]) < 5.0 )){
  PDGcode = 6;
  BG = p/2.80925011634826660e+00;
  dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
  dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
  dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
  dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
  dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
  tofnsigma = tofQA[6];
  tpcnsigma = tpcQA[6];
  }


    //kaon
  if ((p > 0.4 && p < 1.5 && TMath::Abs(tofQA[2]) < 3.0 && TMath::Abs(tpcQA[2]) < 5.0) || (p < 0.4 && TMath::Abs(tpcQA[2]) < 5.0 )){
  PDGcode = 3;
  BG = p/4.93676990270614624e-01;
  dEdxExpected_SatLund_woDeut = 50. * spline_SatLund_woDeut_all->Eval(BG);
  dEdxExpected_SatLund_wDeut = 50. * spline_SatLund_wDeut_all->Eval(BG);
  dEdxExpected_ALEPH_woDeut = 50. * spline_ALEPH_woDeut_all->Eval(BG);
  dEdxExpected_ALEPH_wDeut = 50. * spline_ALEPH_wDeut_all->Eval(BG);
  dEdxExpected_SatLund_woDeut_pileUpCut = 50. *spline_SatLund_woDeut_pileupCut_all->Eval(BG);
  tofnsigma = tofQA[2];
  tpcnsigma = tpcQA[2];
  }

if (PDGcode==-999) continue;

fTree2->Fill();
    }
  }

}

void AliSkimmedDataAnalysisMaker::WriteHistogram()
{

const Int_t nPar = 6;
  Double_t from = 0.1;
  Double_t to = 20000;
  TF1 *funcBB_SatLund_woDeut, *funcBB_SatLund_wDeut, *funcBB_SatLund_woDeut_pileUpCut;
  TF1 *funcBB_ALEPH_woDeut,*funcBB_ALEPH_wDeut;


  TFile *fSpl_SatLund_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund.root");
  TSpline3 *spline_SatLund_woDeut_p = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_k = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pi = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_e = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_all = (TSpline3*)fSpl_SatLund_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  funcBB_SatLund_woDeut = new TF1("SaturatedLund_woDeut", SaturatedLund, from, to, nPar);
  Double_t parametersBB_SatLund_woDeut[nPar] = {6194823.624020,117934.231008,0.702757,0.091025,6628748.318540,-75231816.241975};
  Double_t parameterErrorsBB_SatLund_woDeut[nPar] = {226.418758,232.996274,0.001245,0.000097,222.772041,2634.137710};
  funcBB_SatLund_woDeut->SetParameters(parametersBB_SatLund_woDeut);
  funcBB_SatLund_woDeut->SetParErrors(parameterErrorsBB_SatLund_woDeut);


  TFile *fSpl_SatLund_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLundwDeut.root");
  TSpline3 *spline_SatLund_wDeut_p = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_k = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_pi = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_e = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_wDeut_all = (TSpline3*)fSpl_SatLund_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  funcBB_SatLund_wDeut = new TF1("SaturatedLund_wDeut", SaturatedLund, from, to, nPar);
  Double_t parametersBB_SatLund_wDeut[nPar] = {46.827317, 8.896280, 1.041734,0.969011,93.105222,-17.099605};
  Double_t parameterErrorsBB_SatLund_wDeut[nPar] = {0.033727,0.013812,0.006575,0.000261,0.055071,0.058089};
  funcBB_SatLund_wDeut->SetParameters(parametersBB_SatLund_wDeut);
  funcBB_SatLund_wDeut->SetParErrors(parameterErrorsBB_SatLund_wDeut);

  TFile *fSpl_ALEPH_woDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPH.root");
  TSpline3 *spline_ALEPH_woDeut_p = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_k = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_pi = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_e = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_woDeut_all = (TSpline3*)fSpl_ALEPH_woDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN"); 

  funcBB_ALEPH_woDeut = new TF1("ALEPH_woDeut", Aleph, from, to, nPar);
  Double_t parametersBB_ALEPH_woDeut[nPar] = {8.891597,4.737140,-4.990538,2.280344,0.959991,1.000000};
  Double_t parameterErrorsBB_ALEPH_woDeut[nPar] = {0.084563,0.037765,0.043996,0.002562,0.007093,0.000000};
  funcBB_ALEPH_woDeut->SetParameters(parametersBB_ALEPH_woDeut);
  funcBB_ALEPH_woDeut->SetParErrors(parameterErrorsBB_ALEPH_woDeut); 

  TFile *fSpl_ALEPH_wDeut = new TFile("/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_ALEPHwDeut.root");
  TSpline3 *spline_ALEPH_wDeut_p = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_k = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_pi = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_e = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_ALEPH_wDeut_all = (TSpline3*)fSpl_ALEPH_wDeut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");

  funcBB_ALEPH_wDeut = new TF1("ALEPH_wDeut", Aleph, from, to, nPar);
  Double_t parametersBB_ALEPH_wDeut[nPar] = {7.541188,5.476465,-5.793323,2.247418,1.087683,1.000000};
  Double_t parameterErrorsBB_ALEPH_wDeut[nPar] = {0.030989,0.019520,0.022252,0.000408,0.003660,0.000000};
  funcBB_ALEPH_wDeut->SetParameters(parametersBB_ALEPH_wDeut);
  funcBB_ALEPH_wDeut->SetParErrors(parameterErrorsBB_ALEPH_wDeut); 

  TFile *fSpl_SatLund_woDeut_pileupCut = new TFile("/lustre/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/Spline_Testing/splines_SaturatedLund_woDeut_pileupCut.root");
  TSpline3 *spline_SatLund_woDeut_pileupCut_p = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PROTON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_k = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_KAON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_pi = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_PION_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_e = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ELECTRON_LHC18q_pass3_PBPB_MEAN");
  TSpline3 *spline_SatLund_woDeut_pileupCut_all = (TSpline3*)fSpl_SatLund_woDeut_pileupCut->Get("TSPLINE3_DATA_ALL_LHC18q_pass3_PBPB_MEAN");
  
  funcBB_SatLund_woDeut_pileUpCut = new TF1("SaturatedLund_SatLund_woDeut_pileUpCut", SaturatedLund, from, to, nPar);
  Double_t parametersBB_SatLund_woDeut_pileUpCut[nPar] = {2016573293.630999,24609491.878779,0.774286,0.061786,2112251047.394641,-36132847568.040344};
  Double_t parameterErrorsBB_SatLund_woDeut_pileUpCut[nPar] = {21373.662565,20015.650491,0.000720,0.000026,22514.422129,367799.742386};
  funcBB_SatLund_woDeut_pileUpCut->SetParameters(parametersBB_SatLund_woDeut_pileUpCut);
  funcBB_SatLund_woDeut_pileUpCut->SetParErrors(parameterErrorsBB_SatLund_woDeut_pileUpCut);
  
      cout<<"  write"<<endl;

    mOutputFile_BBFitAnalysis->cd();
    fTree2->Write();
    mOutputFile_BBFitAnalysis->mkdir("SaturatedLund_woDeut");
    mOutputFile_BBFitAnalysis->cd("SaturatedLund_woDeut");
    spline_SatLund_woDeut_e->Write();
    spline_SatLund_woDeut_pi->Write();
    spline_SatLund_woDeut_k->Write();
    spline_SatLund_woDeut_p->Write();
    spline_SatLund_woDeut_all->Write();
    funcBB_SatLund_woDeut->Write();
        
        mOutputFile_BBFitAnalysis->mkdir("SaturatedLund_wDeut");
    mOutputFile_BBFitAnalysis->cd("SaturatedLund_wDeut");
    spline_SatLund_wDeut_e->Write();
    spline_SatLund_wDeut_pi->Write();
    spline_SatLund_wDeut_k->Write();
    spline_SatLund_wDeut_p->Write();
    spline_SatLund_wDeut_all->Write();
    funcBB_SatLund_wDeut->Write();
        
        mOutputFile_BBFitAnalysis->mkdir("SaturatedLund_woDeut_pileUpCut");
    mOutputFile_BBFitAnalysis->cd("SaturatedLund_woDeut_pileUpCut");
    spline_SatLund_woDeut_pileupCut_e->Write();
    spline_SatLund_woDeut_pileupCut_pi->Write();
    spline_SatLund_woDeut_pileupCut_k->Write();
    spline_SatLund_woDeut_pileupCut_p->Write();
    spline_SatLund_woDeut_pileupCut_all->Write();
    funcBB_SatLund_woDeut_pileUpCut->Write();
   /*     
        mOutputFile_BBFitAnalysis->mkdir("ALEPH_woDeut");
    mOutputFile_BBFitAnalysis->cd("ALEPH_woDeut");
    spline_ALEPH_woDeut_e->Write();
    spline_ALEPH_woDeut_pi->Write();
    spline_ALEPH_woDeut_k->Write();
    spline_ALEPH_woDeut_p->Write();
    spline_ALEPH_woDeut_all->Write();
    funcBB_ALEPH_woDeut->Write();
        
    mOutputFile_BBFitAnalysis->mkdir("ALEPH_wDeut");
    mOutputFile_BBFitAnalysis->cd("ALEPH_wDeut");
    spline_ALEPH_wDeut_e->Write();
    spline_ALEPH_wDeut_pi->Write();zz
    spline_ALEPH_wDeut_k->Write();
    spline_ALEPH_wDeut_p->Write();
    spline_ALEPH_wDeut_all->Write();
    funcBB_ALEPH_wDeut->Write();
    */
    mOutputFile_BBFitAnalysis->Close();
}
  
void AliSkimmedDataAnalysisMaker::BinLogAxis(THnSparseF *h, Int_t axisNumber)
{
  //
  // Method for the correct logarithmic binning of histograms
  //
  TAxis *axis = h->GetAxis(axisNumber);
  int bins = axis->GetNbins();

  Double_t from = axis->GetXmin();
  Double_t to = axis->GetXmax();
  Double_t *newBins = new Double_t[bins + 1];

  newBins[0] = from;
  Double_t factor = pow(to/from, 1./bins);

  for (int i = 1; i <= bins; i++) {
    newBins[i] = factor * newBins[i-1];
  }
  axis->Set(bins, newBins);
  delete [] newBins;

}
void AliSkimmedDataAnalysisMaker::SetAxisNamesFromTitle(const THnSparseF *h)
{
  // set the histogram axis names from the axis titles
  for (Int_t i=0; i<h->GetNdimensions(); ++i) {
    h->GetAxis(i)->SetName(h->GetAxis(i)->GetTitle());
  }
}

Double_t AliSkimmedDataAnalysisMaker::Lund(Double_t* xx, Double_t* par)
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
Double_t AliSkimmedDataAnalysisMaker::SaturatedLund(Double_t* xx, Double_t* par)
{
  const Double_t qq = Lund(xx, par);
  return qq * TMath::Exp(par[5] / qq);
}

Double_t AliSkimmedDataAnalysisMaker::Aleph(Double_t* xx, Double_t* par) {
  const Double_t bg = xx[0];

  const Double_t a0 = par[0];
  const Double_t a1 = par[1];
  const Double_t a2 = par[2];
  const Double_t a3 = par[3];
  const Double_t a4 = par[4];
  const Double_t a5 = par[5];

  const Double_t beta = TMath::Sqrt(bg*bg / (1.0 + bg*bg));
  const Double_t powbetaa3 = TMath::Power(beta,a3);

  const Double_t value = a0/powbetaa3 * (a1 - a2 - a5 * powbetaa3 - TMath::Log(1.0 + TMath::Power(bg, -a4)*TMath::Exp(-a2)));

  return value;
}
