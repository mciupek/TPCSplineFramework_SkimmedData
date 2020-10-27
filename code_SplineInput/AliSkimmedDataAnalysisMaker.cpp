#include <iostream>

#include "V0FlatAna.h"
//#include "AliESDtrack.h"
#include "TrackFlatAna.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "THnSparse.h"
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


  fTree2 = new TTree("fTree2", "Tree for analysis of BetheBloch fit");
  fTree2->Branch("p", &p);
  fTree2->Branch("oneoverpt",&oneoverpt);
  fTree2->Branch("rawtpcsignal",&rawtpcsignal);
  fTree2->Branch("BG", &BG);
  fTree2->Branch("eta",&eta);
  fTree2->Branch("multiplicty",&multiplicty);
  fTree2->Branch("centrality", &centrality);
  fTree2->Branch("PDGcode", &PDGcode);



  
  //  THnSparseF* dEdx_THnF = new THnSparseF(name.Data(), title.Data(), kNdim, binsHistQA, xminHistQA, xmaxHistQA);
  // BinLogAxis(h, 0);


  
  
}
void AliSkimmedDataAnalysisMaker::read(TString fileName)
{
  cout<<"!!!!!!!!!!!!!!!!!read"<<fileName<<endl;
  TFile * infile=new TFile(fileName.Data());
  TTree* Tree=(TTree*) infile->Get("V0Flat");
  V0FlatAna *V0ana=new V0FlatAna();
  V0ana->Init(Tree);

  for(Int_t iV0=0;iV0<Tree->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
	cout<<"working on Event  "<<iV0<<endl;
      V0ana->GetEntry(iV0);

      mh2pTdEdxVspT->Fill(V0ana->track0P,V0ana->track0_fTPCsignal);
      mh2pTdEdxVspT->Fill(V0ana->track1P,V0ana->track1_fTPCsignal);

      if(V0ana->K0Like>0.85)
	{
	  mh2pTdEdxVspT_K0Like->Fill(V0ana->track0P,V0ana->track0_fTPCsignal);
	  mh2pTdEdxVspT_K0Like->Fill(V0ana->track1P,V0ana->track1_fTPCsignal);
	}

      if(V0ana->ELike>0.85)
	{
	  mh2pTdEdxVspT_ELike->Fill(V0ana->track0P,V0ana->track0_fTPCsignal);
	  mh2pTdEdxVspT_ELike->Fill(V0ana->track1P,V0ana->track1_fTPCsignal);
	}
      if(V0ana->LLike>0.85)
	{
	  mh2pTdEdxVspT_LLike->Fill(V0ana->track0P,V0ana->track0_fTPCsignal);
	  mh2pTdEdxVspT_LLike->Fill(V0ana->track1P,V0ana->track1_fTPCsignal);
	}
      if(V0ana->ALLike>0.85)
	{
	  mh2pTdEdxVspT_ALLike->Fill(V0ana->track0P,V0ana->track0_fTPCsignal);
	  mh2pTdEdxVspT_ALLike->Fill(V0ana->track1P,V0ana->track1_fTPCsignal);
	}


      const Int_t numCases = 8;
      Double_t tpcQA[numCases];
      Double_t tofQA[numCases];
      for (Int_t i = 0; i < numCases; i++) {
	tpcQA[i] = 0.;
	tofQA[i] = 0.;
      }

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
      Bool_t isPileUp = kFALSE;


      Double_t processedTPCsignal[8] = { 1,1,1,1,1,1,1,1 };
      Double_t deltaTPCsignal[8] = { -111,-111,-111,-111,-111,-111,-111,-111 };
      //V0
      for (Int_t track = 0; track < 2; track++) {
       

	bool isTreeK0=false;
	bool isTreeGamma=false;
	bool isTreeLambda=false;
	bool isTreeALambda=false;

	if(V0ana->K0Like>0.8&&(V0ana->cleanK0==1))
	  {
	    isTreeK0=true;   
	  }
	if(V0ana->ELike>0.9&&(V0ana->cleanGamma==1))
	  {
	    isTreeGamma=true;   
	  }

	if(V0ana->LLike>0.8&&(V0ana->cleanLambda==1))
	  {
	    isTreeLambda=true;   
	  }

	if(V0ana->ALLike>0.8&&(V0ana->cleanALambda==1))
	  {
	    isTreeALambda=true;   
	  }

	if(!(isTreeK0||isTreeGamma||isTreeLambda||isTreeALambda)) continue;

	if(track==0)
	  {
	    tpcsignal=V0ana->track0_fTPCsignal;

	    precin=V0ana->track0P;
	    tanTheta=V0ana->track0Eta;    
	    tpccluster=V0ana->track0_fTPCsignalN;
      tanTheta=V0ana->track0Eta;    
      dca_r = V0ana->dca0_r;
	    dca_z = V0ana->dca0_z;
 	    nCrossRows = V0ana->nCrossRows0;
      isPileUp = V0ana->isPileUp;

	    tpcQA[0]=V0ana->track0tpcNsigma_el;
	    tofQA[0]=V0ana->track0tofNsigmaElectron;

	    tpcQA[1]=V0ana->track0tpcNsigma_pi;
	    tofQA[1]=V0ana->track0tofNsigmaPion;

	    tpcQA[2]=V0ana->track0tpcNsigma_ka;
	    tofQA[2]=V0ana->track0tofNsigmaKaon;

	    tpcQA[3]=V0ana->track0tpcNsigma_pro;
	    tofQA[3]=V0ana->track0tofNsigmaProton;

	    tpcQA[4]=V0ana->track0tpcNsigma_ka;
	    tofQA[4]=V0ana->track0tofNsigmaKaon;

	    tpcQA[5] = -999;
            tofQA[5] = -999;

            tpcQA[6] = -999;
            tofQA[6] = -999;

            tpcQA[7] = -999;
            tofQA[7] = -999;
    
	          processedTPCsignal[0] = V0ana->track0CorrectedTPCSignalV0_el;
            processedTPCsignal[1] = V0ana->track0CorrectedTPCSignalV0_pi;
	          processedTPCsignal[2] = V0ana->track0CorrectedTPCSignalV0_ka;
            processedTPCsignal[3] = V0ana->track0CorrectedTPCSignalV0_pro;
            processedTPCsignal[5] = -999;	    
            processedTPCsignal[6] = -999;
            processedTPCsignal[7] = -999;
/*
            deltaTPCsignal[0] = (tpcsignal - V0ana->track0GetPileupValue) - V0ana->track0ExpectedTPCSignalV0_el;
            deltaTPCsignal[1] = (tpcsignal - V0ana->track0GetPileupValue) - V0ana->track0ExpectedTPCSignalV0_pi;
            deltaTPCsignal[2] = (tpcsignal - V0ana->track0GetPileupValue) - V0ana->track0ExpectedTPCSignalV0_ka;
            deltaTPCsignal[3] = (tpcsignal - V0ana->track0GetPileupValue) - V0ana->track0ExpectedTPCSignalV0_pro;
	*/    
	  }
	else if(track==1)
	  {
	    tpcsignal=V0ana->track1_fTPCsignal;
	    precin=V0ana->track1P;
	    tanTheta=V0ana->track1Eta;    
            tpccluster=V0ana->track1_fTPCsignalN;
            dca_r = V0ana->dca1_r;
            dca_z = V0ana->dca1_z;
            nCrossRows = V0ana->nCrossRows1;
            isPileUp = V0ana->isPileUp;

	    tpcQA[0]=V0ana->track1tpcNsigma_el;
	    tofQA[0]=V0ana->track1tofNsigmaElectron;

	    tpcQA[1]=V0ana->track1tpcNsigma_pi;
	    tofQA[1]=V0ana->track1tofNsigmaPion;

	    tpcQA[2]=V0ana->track1tpcNsigma_ka;
	    tofQA[2]=V0ana->track1tofNsigmaKaon;

	    tpcQA[3]=V0ana->track1tpcNsigma_pro;
	    tofQA[3]=V0ana->track1tofNsigmaProton;

	    tpcQA[4]=V0ana->track1tpcNsigma_ka;
	    tofQA[4]=V0ana->track1tofNsigmaKaon;

            tpcQA[5] = -999;
            tofQA[5] = -999;

            tpcQA[6] = -999;
            tofQA[6] = -999;

            tpcQA[7] = -999;
            tofQA[7] = -999;

            processedTPCsignal[0] = V0ana->track1CorrectedTPCSignalV0_el;
            processedTPCsignal[1] = V0ana->track1CorrectedTPCSignalV0_pi;
            processedTPCsignal[2] = V0ana->track1CorrectedTPCSignalV0_ka;
            processedTPCsignal[3] = V0ana->track1CorrectedTPCSignalV0_pro;
            processedTPCsignal[5] = -999;           
            processedTPCsignal[6] = -999;
            processedTPCsignal[7] = -999;
/*
            deltaTPCsignal[0] = (tpcsignal - V0ana->track1GetPileupValue) - V0ana->track1ExpectedTPCSignalV0_el;
            deltaTPCsignal[1] = (tpcsignal - V0ana->track1GetPileupValue) - V0ana->track1ExpectedTPCSignalV0_pi;
            deltaTPCsignal[2] = (tpcsignal - V0ana->track1GetPileupValue) - V0ana->track1ExpectedTPCSignalV0_ka;
            deltaTPCsignal[3] = (tpcsignal - V0ana->track1GetPileupValue) - V0ana->track1ExpectedTPCSignalV0_pro;
*/
	  }
	

      if (tpccluster < 50) continue;
      if(TMath::Abs(tanTheta) > 0.9) continue;
      if(TMath::Abs(dca_r) > 3.0) continue;
      if(TMath::Abs(dca_z) > 3.0) continue;
      if(nCrossRows < 70) continue;
      if(isPileUp=1) continue;
	
	if (isTreeK0) {
	  particleID = 2; //pion
	  //	  alicePID=AliPID::kPion;
	  alicePID=2;
	  
	} else if (isTreeGamma) {
	  particleID = 1; // electron
	  //  alicePID=AliPID::kElectron;
	  alicePID=0;//AliPID::kElectron;
	} else if (isTreeLambda) {
	  if (track == 0) {
	    particleID = 3; // proton
	    //	    alicePID=AliPID::kProton;
	    alicePID=4.;
	  } else {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    alicePID=2;//AliPID::kPion;
	  }
	}
	else if (isTreeALambda) {
	  if (track == 0) {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    alicePID=2.;//AliPID::kPion;

	  } else {
	    particleID = 3; // proton
	    //alicePID=AliPID::kProton;
	    alicePID=4;;
	  }
	}
	
	
	if(particleID==1&&isTreeGamma){
	  mh2dEdxVspT_Electron->Fill(precin,tpcsignal);
	}
	if(particleID==2&&(isTreeK0||(isTreeLambda&&track==1)||(isTreeALambda&&track==0))){
	  mh2dEdxVspT_Pion->Fill(precin,tpcsignal);
	}
	if(particleID==3&&(isTreeLambda&&track==0)||(isTreeALambda&&track==1)){
	  //	  mh2dEdxVspT_Proton->Fill(precin,tpcsignal);
	}
       
	nTotESDTracks=V0ana->tpcTrackBeforeClean;
	
	// id 5 is just again Kaons in restricted eta range
	processedTPCsignal[4] = processedTPCsignal[2];
  deltaTPCsignal[4] = deltaTPCsignal[2];

	for(Int_t iPart = 0; iPart < numCases; iPart++) {
	  // Only accept "Kaons" within |eta| < 0.2 for index 4 (eta ~ tanTheta in this eta range)
	  if (iPart == 4 && abs(tanTheta) > 0.2)
	    continue;

	  Double_t vecHistQA[8] = {precin, processedTPCsignal[iPart], (Double_t)particleID, (Double_t)iPart, tpcQA[iPart], tofQA[iPart],
				   (Double_t)nTotESDTracks,TMath::Abs(tanTheta)};
	  //cout << vecHistQA[0] << "\t" <<  vecHistQA[1] << "\t" <<  vecHistQA[2] << "\t" <<  vecHistQA[3] << "\t" <<  vecHistQA[4] << "\t" <<  vecHistQA[5] <<  endl;
	  fHistPidQA->Fill(vecHistQA);

	  if(precin>1.7 && precin<1.76 && iPart==3){                                                                                                                                                                               
	    mh2dEdxVspT_Proton->Fill(nTotESDTracks,tpcsignal);
                                                                                                                                                                                                            
	  }     
	}
      }
      
      
      
      
    }
}

void AliSkimmedDataAnalysisMaker::Read_tracktree(TString fileName){
 cout<<"!!!!!!!!!!!!!!!!!read!!!!!!!"<<fileName<<endl;
  TFile * infile=new TFile(fileName.Data());
  TTree* Tree=(TTree*) infile->Get("CleanTrackFlat");
  if(Tree==0x0){
  } else {                                                                                                                                                                          
  TrackFlatAna *TrackAna=new TrackFlatAna();
  TrackAna->Init(Tree);

  for(Int_t iV0=0;iV0 < Tree->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
        cout<<"working on Event  "<<iV0<<endl;
      TrackAna->GetEntry(iV0);

      const Int_t numCases = 8;
      Double_t tpcQA[numCases];
      Double_t tofQA[numCases];
      for (Int_t i = 0; i < numCases; i++) {
        tpcQA[i] = 0.;
        tofQA[i] = 0.;
      }
      
      Int_t particleID = 0;
      Int_t  alicePID=-1;
      Double_t tpcsignal = -1.;
      Double_t tanTheta = -999.;
      Double_t precin = -1.;
      Double_t  nTotESDTracks=-999;
      Double_t tofNsigma=-9999;
      Double_t tpcNsigma=-9999;
      Int_t numberOfCluster = -999;
      Double_t eta = -999;
      Double_t dca_r = -999;
      Double_t dca_z = -999;
      Double_t nCrossRows = -999;
      Bool_t isPileUp = kFALSE;

      numberOfCluster = TrackAna->fTPCsignalN;
      eta = TrackAna->trackEta;
      dca_r = TrackAna->dca_r;
      dca_z = TrackAna->dca_z;
      nCrossRows = TrackAna->nCrossRows;
      isPileUp = TrackAna->isPileUp;


  if (numberOfCluster < 50) continue; 
  if(TMath::Abs(eta) > 0.9) continue;                                                                                                                                                                   
  if(TMath::Abs(dca_r) > 3.0) continue;
  if(TMath::Abs(dca_z) > 3.0) continue;
  if(nCrossRows < 70) continue;
  if(isPileUp ==1) continue;

            tpcsignal=TrackAna->fTPCsignal;
            precin=TrackAna->trackP;
            tanTheta=TrackAna->trackEta;   
            

            tpcQA[0]=TrackAna->track_tpcNsigma_el;
            tofQA[0]=TrackAna->tracktofNsigmaElectron;

            tpcQA[1]=TrackAna->track_tpcNsigma_pi;
            tofQA[1]=TrackAna->tracktofNsigmaPion;

            tpcQA[2]=TrackAna->track_tpcNsigma_ka;
            tofQA[2]=TrackAna->tracktofNsigmaKaon;

            tpcQA[3]=TrackAna->track_tpcNsigma_pro;
            tofQA[3]=TrackAna->tracktofNsigmaProton;

            tpcQA[4]=TrackAna->track_tpcNsigma_ka;
            tofQA[4]=TrackAna->tracktofNsigmaKaon;

            tpcQA[5]= TrackAna->track_tpcNsigma_deut;
	    tofQA[5]= TrackAna->tracktofNsigmaDeuteron;

	    tpcQA[6]= TrackAna->track_tpcNsigma_tri;
            tofQA[6]= TrackAna->tracktofNsigmaTrition;

            tpcQA[7]= TrackAna->track_tpcNsigma_He3;
            tofQA[7]= TrackAna->tracktofNsigmaHelium3;

		


	    Double_t processedTPCsignal[8] = { tpcsignal,tpcsignal, tpcsignal, tpcsignal, tpcsignal, tpcsignal,tpcsignal,tpcsignal };
      Double_t deltaTPCsignal[8] = { -111,-111,-111,-111,-111,-111,-111,-111 };     

        processedTPCsignal[0] = TrackAna->track_CorrectedTPCSignalV0_el;
        processedTPCsignal[1] = TrackAna->track_CorrectedTPCSignalV0_pi;
        processedTPCsignal[2] = TrackAna->track_CorrectedTPCSignalV0_ka;
        processedTPCsignal[3] = TrackAna->track_CorrectedTPCSignalV0_pro;
        processedTPCsignal[5] = TrackAna->track_CorrectedTPCSignalV0_deut;
        processedTPCsignal[6] = TrackAna->track_CorrectedTPCSignalV0_tri;
        processedTPCsignal[7] = TrackAna->track_CorrectedTPCSignalV0_He3;
/*
        deltaTPCsignal[0] = (tpcsignal -TrackAna->track_GetPileupValue) - TrackAna->track_ExpectedTPCSignalV0_el;
        deltaTPCsignal[1] = (tpcsignal -TrackAna->track_GetPileupValue) - TrackAna->track_ExpectedTPCSignalV0_pi;
        deltaTPCsignal[2] = (tpcsignal -TrackAna->track_GetPileupValue) - TrackAna->track_ExpectedTPCSignalV0_ka;
        deltaTPCsignal[3] = (tpcsignal -TrackAna->track_GetPileupValue) - TrackAna->track_ExpectedTPCSignalV0_pro;
*/

        nTotESDTracks=TrackAna->tpcTrackBeforeClean;


        // id 5 is just again Kaons in restricted eta range
        processedTPCsignal[4] = processedTPCsignal[2];
        deltaTPCsignal[4] = deltaTPCsignal[2];

        for(Int_t iPart = 0; iPart < numCases; iPart++) {
          // Only accept "Kaons" within |eta| < 0.2 for index 4 (eta ~ tanTheta in this eta range)
          if (iPart == 4 && abs(tanTheta) > 0.2)
            continue;
          if (iPart == 0 && TrackAna->Nucleitrigger_OFF==0) continue;
          if (iPart == 3 && precin > 1.0){
             if(TrackAna->Nucleitrigger_OFF == 0) continue;
}
          Double_t vecHistQA[8] = {precin, processedTPCsignal[iPart], (Double_t)particleID, (Double_t)iPart, tpcQA[iPart], tofQA[iPart],
                                   (Double_t)nTotESDTracks,TMath::Abs(tanTheta)};
          //cout << vecHistQA[0] << "\t" <<  vecHistQA[1] << "\t" <<  vecHistQA[2] << "\t" <<  vecHistQA[3] << "\t" <<  vecHistQA[4] << "\t" <<  vecHistQA[5] <<  endl;
          fHistPidQA->Fill(vecHistQA);
		}

}
  }

}

void AliSkimmedDataAnalysisMaker::Filltreeformap_track(TString filename_track){
//Starting with no V0:

 cout<<"!!!!!!!!!!!!!!!!!read!!!!!!!"<<filename_track<<endl;
  TFile * infile=new TFile(filename_track.Data());
  TTree* Tree=(TTree*) infile->Get("CleanTrackFlat");
  if(Tree==0x0){                                                                                                                                                                                           
  } else {         
  TrackFlatAna *TrackAna=new TrackFlatAna();
  TrackAna->Init(Tree);

  for(Int_t iV0=0;iV0 < Tree->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
        cout<<"working on Event  "<<iV0<<endl;
      TrackAna->GetEntry(iV0);

      Double_t dca_r = -999;
      Double_t dca_z = -999;
      Double_t nCrossRows = -999;
      fPtpc = -999;
      Bool_t isPileUp = kFALSE;

  dca_r = TrackAna->dca_r;
  dca_z = TrackAna->dca_z;
  nCrossRows = TrackAna->nCrossRows;
  isPileUp = TrackAna->isPileUp;
 

  if(TMath::Abs(TrackAna->trackEta) > 0.9) continue;
    if(TMath::Abs(dca_r) > 3.0) continue;
    if(TMath::Abs(dca_z) > 3.0) continue;
    if(nCrossRows < 70) continue;
    if(isPileUp ==1) continue;

            mh1p->Fill(dca_z);
 
  fPtpc = TrackAna->trackP;
  fDeDxExpected = -1;
  fTPCsignalNsubthreshold = 200; 
  fNumTPCClustersInActiveVolume = 200.;
  fPIDtype = -1; 

   if(fPtpc > 1.0){
     if(TrackAna->Nucleitrigger_OFF == 0) continue;
     } 



  if(fPtpc > 5.) continue;

//  fDeDx = (TrackAna->fTPCsignal - TrackAna->track_GetPileupValue);
  fDeDx = TrackAna->fTPCsignal;
  Bool_t hasTOF     = kFALSE;  

  if(TrackAna->track_hasTOF==1)
      hasTOF = kTRUE;
 



        if (fPtpc < 4.0 && //TODO was 2.7 // Do not accept non-V0s above this threshold -> High contamination!!!
            (fDeDx >= 50. / TMath::Power(fPtpc, 1.3))) {// Pattern recognition instead of TPC cut to be ~independent of old TPC expected dEdx
 
         if (fPtpc < 0.6) {
            fPIDtype = 1;
         //cout << fPtpc << endl; 
          }
          // fPtpc >= 0.6
          else if (hasTOF && TMath::Abs(TrackAna->tracktofNsigmaProton) < 3.0) {
              fPIDtype = 3;
          }
          else
            continue; // Reject particle
        }

if(fPIDtype == -1) continue;


  fMultiplicity = TrackAna->tpcTrackBeforeClean;
  fDeDxExpected = TrackAna->track_ExpectedTPCSignalV0_pro;
  fTanTheta = TrackAna->tgl;
  fTPCsignalN = TrackAna->fTPCsignalN;
  fTPCsignalNsubthreshold = 200;


 fTree->Fill();

}
  } // Check for working file

}


void AliSkimmedDataAnalysisMaker::Filltreeformap_V0(TString filename_v0){
//V0 filling

  cout<<"!!!!!!!!!!!!!!!!!read"<<filename_v0<<endl;
  TFile * infile2=new TFile(filename_v0.Data());
  TTree* Tree2=(TTree*) infile2->Get("V0Flat");
  V0FlatAna *V0ana=new V0FlatAna();
  V0ana->Init(Tree2);

  for(Int_t iV0=0;iV0<Tree2->GetEntries();iV0++) 
    {
      if(iV0%10000==0)
        cout<<"working on Event  "<<iV0<<endl;
      V0ana->GetEntry(iV0);

      for (Int_t track = 0; track < 2; track++) {

      Double_t itsClusterMult = -999;
      Double_t tpcClusterMult = -999;
      Bool_t   isPileUp =kFALSE;
      Double_t dca_r = -999;
      Double_t dca_z = -999;
      Double_t nCrossRows = -999;
  


        bool isTreeLambda=false;
        bool isTreeALambda=false;
        bool isPr = false;

        if(V0ana->LLike>0.80&&(V0ana->cleanLambda==1))
          {
            isTreeLambda=true;   
          }

        if(V0ana->ALLike>0.80&&(V0ana->cleanALambda==1))
          {
            isTreeALambda=true;   
          }

        if(!(isTreeLambda||isTreeALambda)) continue;

        //Selecting protons for Eta correction
        if (isTreeLambda) {
          if (track == 0) {
            isPr=true; // proton
            //      alicePID=AliPID::kProton;
          } else {
            isPr=false; // pion
            //      alicePID=AliPID::kPion;
          }
        }
        else if (isTreeALambda) {
          if (track == 0) {
            isPr=false; // pion
            //      alicePID=AliPID::kPion;

          } else {
            isPr=true; // proton
          }
        }


	if(isPr==false) continue;


Double_t etasel= -999;
Double_t tofsel = -999;
Bool_t hasTOF_track = 0;
fPtpc = -1;
fDeDxExpected = -1;
fDeDx = -1;
fMultiplicity = -999;
dca_r = -999;
dca_z = -999;
nCrossRows = -999;

fTPCsignalNsubthreshold = 200; 
fNumTPCClustersInActiveVolume = 200.;
UInt_t status = -999;
fPIDtype = -3;

if(track==0){
etasel= V0ana->track0Eta;
tofsel= V0ana->track0tofNsigmaProton;
hasTOF_track= V0ana->track0_hasTOF;
dca_r = V0ana->dca0_r;
dca_z = V0ana->dca0_z;
nCrossRows = V0ana->nCrossRows0;
fPtpc = V0ana->track0P;
//fDeDx = (V0ana->track0_fTPCsignal - V0ana->track0GetPileupValue)d;
fDeDx = V0ana->track0_fTPCsignal;
//status = V0ana->track0status;
fTanTheta = V0ana->track0Tgl;
fTPCsignalN = V0ana->track0_fTPCsignalN;
fDeDxExpected = V0ana->track0ExpectedTPCSignalV0_pro;
fMultiplicity = V0ana->tpcTrackBeforeClean;                                                                                                                                                                
fTPCsignalNsubthreshold = 200;
isPileUp = V0ana->isPileUp;
}

 else if(track==1){
etasel= V0ana->track1Eta;
tofsel= V0ana->track1tofNsigmaProton;
hasTOF_track= V0ana->track1_hasTOF;
dca_r = V0ana->dca1_r;
dca_z = V0ana->dca1_z;
nCrossRows = V0ana->nCrossRows1;
fPtpc= V0ana->track1P;
fDeDx = V0ana->track1_fTPCsignal;
// fDeDx = (V0ana->track1_fTPCsignal - V0ana->track1GetPileupValue);
//status = V0ana->track1status;
fTanTheta = V0ana->track1Tgl;
fTPCsignalN = V0ana->track1_fTPCsignalN;
fDeDxExpected = V0ana->track1ExpectedTPCSignalV0_pro;
fMultiplicity = V0ana->tpcTrackBeforeClean;                                                                                                                                                                
fTPCsignalNsubthreshold = 200;
isPileUp = V0ana->isPileUp;
}

if(TMath::Abs(etasel) > 0.9) continue;
    if(TMath::Abs(dca_r) > 3.0) continue;
    if(TMath::Abs(dca_z) > 3.0) continue;
    if(nCrossRows < 70) continue;
    if(isPileUp ==1) continue;

  if(fPtpc > 5.) continue;

  Bool_t hasTOF     = kFALSE;  

  if(hasTOF_track==1)
      hasTOF = kTRUE;

  if (!hasTOF) {
    fPIDtype = 2;
  }
  else {
    if (TMath::Abs(tofsel) < 3.0)
      fPIDtype = 4;
    else
      fPIDtype = 5;
  }

 
 if(fPtpc>1.7 && fPtpc<1.76){
   mh2dEdxVspT_Proton->Fill(fMultiplicity,fDeDx);                                                                                                                                                               
}

//if ((fPIDtype == -3)) continue;
fTree->Fill();
} //track loop


} //tree loop


}

void AliSkimmedDataAnalysisMaker::TreeV0_BBFitAnalysis(TString filename_v0){

  cout<<"!!!!!!!!!!!!!!!!!read"<<filename_v0<<endl;
  TFile * infile2=new TFile(filename_v0.Data());
  TTree* Tree2=(TTree*) infile2->Get("V0Flat");
  V0FlatAna *V0ana=new V0FlatAna();
  V0ana->Init(Tree2);

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


      Double_t processedTPCsignal[8] = { 1,1,1,1,1,1,1,1 };
      Double_t deltaTPCsignal[8] = { -111,-111,-111,-111,-111,-111,-111,-111 };

      for (Int_t track = 0; track < 2; track++) {


	bool isTreeK0=false;
	bool isTreeGamma=false;
	bool isTreeLambda=false;
	bool isTreeALambda=false;

	if(V0ana->K0Like>0.8&&(V0ana->cleanK0==1))
	  {
	    isTreeK0=true;   
	  }
	if(V0ana->ELike>0.9&&(V0ana->cleanGamma==1))
	  {
	    isTreeGamma=true;   
	  }

	if(V0ana->LLike>0.8&&(V0ana->cleanLambda==1))
	  {
	    isTreeLambda=true;   
	  }

	if(V0ana->ALLike>0.8&&(V0ana->cleanALambda==1))
	  {
	    isTreeALambda=true;   
	  }

	if(!(isTreeK0||isTreeGamma||isTreeLambda||isTreeALambda)) continue;

	if(track==0)
	  {

  tpccluster=V0ana->track0_fTPCsignalN;
  p = V0ana->track0P;
  oneoverpt = 1;
  rawtpcsignal = V0ana->track0_fTPCsignal;
  eta = V0ana->track0Eta;
  multiplicty = V0ana->tpcTrackBeforeClean;
  centrality = 1;
  dca_r = V0ana->dca0_r;
	dca_z = V0ana->dca0_z;
  nCrossRows = V0ana->nCrossRows0;
  
	  }
	else if(track==1)
	  {

 	tpccluster=V0ana->track1_fTPCsignalN;
  p = V0ana->track1P;
  oneoverpt = 1;
  rawtpcsignal = V0ana->track1_fTPCsignal;
  eta = V0ana->track1Eta;
  multiplicty = V0ana->tpcTrackBeforeClean;
  centrality = 1;
  dca_r = V0ana->dca1_r;
  dca_z = V0ana->dca1_z;
  nCrossRows = V0ana->nCrossRows1;

	  }
	

      if (tpccluster < 50) continue;
      if(TMath::Abs(dca_r) > 3.0) continue;
      if(TMath::Abs(dca_z) > 3.0) continue;
      if(nCrossRows < 70) continue;
	
	if (isTreeK0) {
	  particleID = 2; //pion
	  //	  alicePID=AliPID::kPion;
	  PDGcode=2;
	  
	} else if (isTreeGamma) {
	  particleID = 1; // electron
	  //  alicePID=AliPID::kElectron;
	  PDGcode=0;//AliPID::kElectron;
	} else if (isTreeLambda) {
	  if (track == 0) {
	    particleID = 3; // proton
	    //	    alicePID=AliPID::kProton;
	    PDGcode=4.;
	  } else {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    PDGcode=2;//AliPID::kPion;
	  }
	}
	else if (isTreeALambda) {
	  if (track == 0) {
	    particleID = 2; // pion
	    //	    alicePID=AliPID::kPion;
	    PDGcode=2.;//AliPID::kPion;

	  } else {
	    particleID = 3; // proton
	    //alicePID=AliPID::kProton;
	    PDGcode=4;;
	  }
	}
	       
  fTree2->Fill();

}    // end track loop

    } // end treeloop


} //end function


void AliSkimmedDataAnalysisMaker::TreePrimary_BBFitAnalysis(TString filename_track){



}

void AliSkimmedDataAnalysisMaker::WriteHistogram()
{

    cout<<"  write"<<endl;
    mOutputFile->cd();
    mh2pTdEdxVspT->Write();

    mh2pTdEdxVspT_K0Like->Write();
    mh2pTdEdxVspT_ELike->Write();
    mh2pTdEdxVspT_LLike->Write();
    mh2pTdEdxVspT_ALLike->Write();

    mh2dEdxVspT_Electron->Write();
    mh2dEdxVspT_Pion->Write();
    mh2dEdxVspT_Proton->Write();
    h_pileupcorrelation->Write();
    h_pileupcorrelation_bc->Write();
    h_Centrality->Write();
    h_Centrality_all->Write();

    fHistPidQA->Write();
//    fTree->Write();
    fHistPid_separation_power->Write();

    mOutputFile_eta->cd();
    fTree->Write();

    mh1p->Write();
    mOutputFile_eta->Close();

    mOutputFile_BBFitAnalysis->cd();
    fTree2->Write();
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


