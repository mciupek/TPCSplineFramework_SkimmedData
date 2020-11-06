
/*
  //gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");

  .L fitdEdxCorrectionFiltered.C+
  AliDrawStyle::SetDefaults();
  AliDrawStyle::ApplyStyle("figTemplate");
  gStyle->SetOptTitle(1);
  SetUpNewSpline(296062);
  InitTree(1,1,296062);
//  LoadFits();
  cacheCleanV0();
  cacheCleanTrack();
  cacheEventFlat();
*/

/*
  //gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");

  .L fitdEdxCorrectionFiltered.C+
  AliDrawStyle::SetDefaults();
  AliDrawStyle::ApplyStyle("figTemplate");
  gStyle->SetOptTitle(1);
  SetUpNewSpline(295585);
  InitTree(1,1,295585);
//  LoadFits();
  cacheCleanV0_SplineStudies();
  cacheCleanTrack_SplineStudies();
*/


#include "TObjArray.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TFile.h"
#include "AliCDBEntry.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TH1.h"
#include "AliTreePlayer.h"
#include "TTreeStream.h"
#include "TROOT.h"
//#include "AliESDtools.h"
#include "TStopwatch.h"
#include "AliXRDPROOFtoolkit.h"
#include "TChain.h"
#include "TMatrixD.h"
#include "TVectorF.h"
#include "THn.h"
#include "AliDrawStyle.h"
#include "TStatToolkit.h"
#include "TStyle.h"
#include "AliESDEvent.h"
#include "AliESDtools.h"
#include "AliAnalysisTaskFilteredTree.h"
#include "AliNDLocalRegression.h"
#include "AliPIDtools.h"
#include "AliTPCPIDResponse.h"
#include "AliPIDResponse.h"

TTree * tree =0;
TTree * treeV0=0;
TTree * treeEvent =0;
TTree * treeMap=0;
TTree * treeMapTracks=0;
TTree * treeMapAll=0;
TTree * treeV0Flat=0;
TStopwatch timer;
AliESDtools tools;
AliESDv0 *v0 =0;

Int_t pidHash=1;

TMatrixD projectionInfo(5,5);
void InitTree(Int_t nChunks, Int_t buildIndex=1, Int_t run=296690);

void fitdEdxCorrectionFiltered(){
  ::Info("fitdEdxCorrectionFiltered","Dummy");
}


void enablePileUpCorrection(){
  AliPIDtools::GetTPCPID(pidHash).SetPileupCorrectionObject(AliPIDtools::GetTPCPID(pidHash).GetPileupCorrectionFromFile("/lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/PWGPP-538/new/alice/data/2018/LHC18q/pass3/AODFilterTrees01/dEdxFitLight.root"));
}

void SetUpNewSpline(Int_t run){

  Int_t passNumber  = 3;
  TString recoPass="pass3";

//  Char_t *  spline_path="/lustre/nyx/alice/users/xbai/work/SkimmedDataAna/Run18/TPCsplines/splines/TPCPIDResponseOADB_2020_06_22_18r_pass3_It4_woPileup.root";
//  Char_t *  eta_crrection_path="/lustre/nyx/alice/users/xbai/work/SkimmedDataAna/Run18/TPCsplines/splines/TPCetaMaps_2020_06_22_18r_pass3_It4_woPileup.root";

//  cout<<Form("New  spline from %s",spline_path)<<endl;
//  cout<<Form("New eta maps from %s",eta_crrection_path)<<endl;

  AliESDEvent ev;
  AliPIDResponse *fPIDResponse = new AliPIDResponse;
  AliPIDResponse *fPIDResponse2 = new AliPIDResponse;

  fPIDResponse->SetUseTPCMultiplicityCorrection(false);
  fPIDResponse->SetUseTPCEtaCorrection(false);
  fPIDResponse->SetUseTPCPileupCorrection(true);
  
  fPIDResponse->SetCustomTPCpidResponseOADBFile("");
  fPIDResponse->SetCustomTPCetaMaps("");
  fPIDResponse->SetOADBPath("$ALICE_PHYSICS/OADB/");
  fPIDResponse->InitialiseEvent(&ev,passNumber, recoPass, run);

  AliPIDtools::pidTPC[1]=&(fPIDResponse->GetTPCResponse());
  AliPIDtools::pidAll[1]=fPIDResponse;

  fPIDResponse2->SetUseTPCMultiplicityCorrection(false);
  fPIDResponse2->SetUseTPCEtaCorrection(false);
  fPIDResponse2->SetUseTPCPileupCorrection(true);
  
  fPIDResponse2->SetCustomTPCpidResponseOADBFile("/lustre/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/LHC18q/withpileup/TPCPIDResponseOADB_2020_10_12_18q_pass3_It1.root");
  fPIDResponse2->SetCustomTPCetaMaps("/lustre/alice/users/mciupek/TPCSpline/SkimmedData_Framework/Splines/LHC18q/withpileup/TPCetaMaps_2020_10_12_18q_pass3_It1.root");
  fPIDResponse2->SetOADBPath("$ALICE_PHYSICS/OADB/");
  fPIDResponse2->InitialiseEvent(&ev,passNumber, recoPass, run);

  AliPIDtools::pidTPC[2]=&(fPIDResponse2->GetTPCResponse());
  AliPIDtools::pidAll[2]=fPIDResponse2;
  


}


void makeAliasesTracksPID(){
  tree->SetAlias("itsPID012","(AliPIDtools::GetITSPID(pidHash,0,2,0)+AliPIDtools::GetITSPID(pidHash,1,2,0)+AliPIDtools::GetITSPID(pidHash,2,2,0))>0.7");
  //
  tree->SetAlias("dEdxExpEl","AliPIDtools::BetheBlochAleph(pidHash,0+esdTrack.fIp.P(),0)");
  tree->SetAlias("dEdxExpElS","AliPIDtools::GetExpectedTPCSignal(pidHash,0,0,0)");
  //
  tree->SetAlias("dEdxExpPion","AliPIDtools::BetheBlochAleph(pidHash,0+esdTrack.fIp.P(),2)");
  tree->SetAlias("dEdxExpPionS","AliPIDtools::GetExpectedTPCSignal(pidHash,2,0,0+0)");
  tree->SetAlias("logRatioPion","log(0.01+0.99*esdTrack.fTPCsignal/dEdxExpPion)");
  //
  tree->SetAlias("dEdxExp4","AliPIDtools::BetheBlochAleph(pidHash,0+esdTrack.fIp.P(),4)");
  tree->SetAlias("dEdxExp4S","AliPIDtools::GetExpectedTPCSignal(pidHash,4,0,0+0)");
  tree->SetAlias("logRatio4","log(0.01+0.99*esdTrack.fTPCsignal/dEdxExp4)");
  //
  tree->SetAlias("dEdxExp5","AliPIDtools::BetheBlochAleph(pidHash,0+esdTrack.fIp.P(),5)");
  tree->SetAlias("dEdxExp5","AliPIDtools::GetExpectedTPCSignal(pidHash,5,0,0+0)");
  tree->SetAlias("logRatio5","log(0.01+0.99*esdTrack.fTPCsignal/dEdxExp5)");
  //


}

void makeAliasesTracks(TTree *tree){
  tree->Draw("Bz","1","goff",1);Double_t bz=tree->GetV1()[0];

  tree->SetAlias("dSector",Form("esdTrack.fIp.GetParameterAtRadius(125,%f+0,13)",bz));
  tree->SetAlias("isSelected","abs(tgl)<1&&esdTrack.fITSncls>4&&esdTrack.GetTPCClusterInfo(3,1)>100&&esdTrack.fTPCncls>80&&esdTrack.fTPCsignal>10");
  tree->SetAlias("itsMIPCut","log(esdTrack.fITSsignal/AliExternalTrackParam::BetheBlochSolid(esdTrack.P()/0.13957))<11");
  tree->SetAlias("logdEdx","log(0.02+0.98*esdTrack.fTPCsignal/dEdxExpPion)");
  tree->SetAlias("atgl","abs(tgl+0)");
  tree->SetAlias("multTPCClusterN","tpcClusterMult/2000000."); /// normalize to central event
  tree->SetAlias("itsMIPCutP","log(esdTrack.fITSsignal/AliExternalTrackParam::BetheBlochSolid(esdTrack.P()/0.13957))<11&&esdTrack.P()>0.3&&esdTrack.P()<1.5");
  tree->SetAlias("tofPion2_05","abs(tofNsigma.fElements[2])<2&&abs(tofNsigma.fElements[2])<0.5*min(abs(tofNsigma.fElements[3]),abs(tofNsigma.fElements[4]))");
  tree->SetAlias("pionCut","((itsMIPCut&&esdTrack.P()<0.8)||tofPion2_05)");

  /// for filtered trees
  tree->SetAlias("mdEdx", "50./esdTrack.fTPCsignal");
  tree->SetAlias("dEdxExpPion",Form("AliPIDtools::BetheBlochAleph(%d,0+esdTrack.fIp.P()/0.13597)",pidHash));
  for (Int_t i=0; i<4; i++){
    tree->SetAlias(Form("ratioTotMax%d",i) , Form("fTPCdEdxInfo.GetSignalTot(%d)/fTPCdEdxInfo.GetSignalMax(%d)",i,i));
    tree->SetAlias(Form("logTotMax%d",i) , Form("log(fTPCdEdxInfo.GetSignalTot(%d)/fTPCdEdxInfo.GetSignalMax(%d))",i,i));
    tree->SetAlias(Form("logQMaxMIP%d",i) , Form("log(0.02+0.98*fTPCdEdxInfo.GetSignalTot(%d)/dEdxExpPion)",i));
    tree->SetAlias(Form("logQTotMIP%d",i) , Form("log(0.02+0.98*fTPCdEdxInfo.GetSignalMax(%d)/dEdxExpPion)",i));
    
    tree->SetAlias(Form("logSignalTot%d",i) , Form("fTPCdEdxInfo.GetSignalTot(0+%d)",i));
    tree->SetAlias(Form("logSignalMax%d",i) , Form("fTPCdEdxInfo.GetSignalMax(0+%d)",i));
    tree->SetAlias(Form("signalNcl%d",i) , Form("(fTPCdEdxInfo.GetNumberOfClusters(0+%d))",i));
    tree->SetAlias(Form("signalNcr%d",i) , Form("(fTPCdEdxInfo.GetNumberOfCrossedRows(0+%d))",i));
  }

  tree->SetAlias("ratioTotMax0", "fTPCdEdxInfo.GetSignalTot(0)/fTPCdEdxInfo.GetSignalMax(0)");
  tree->SetAlias("ratioTotMax1", "fTPCdEdxInfo.GetSignalTot(1)/fTPCdEdxInfo.GetSignalMax(1)");
  tree->SetAlias("ratioTotMax2", "fTPCdEdxInfo.GetSignalTot(2)/fTPCdEdxInfo.GetSignalMax(2)");
  tree->SetAlias("ratioTotMax3", "fTPCdEdxInfo.GetSignalTot(3)/fTPCdEdxInfo.GetSignalMax(3)");
  tree->SetAlias("logRatioTot03", "log(fTPCdEdxInfo.GetSignalTot(0)/fTPCdEdxInfo.GetSignalTot(3))");
  tree->SetAlias("logRatioTot13", "log(fTPCdEdxInfo.GetSignalTot(1)/fTPCdEdxInfo.GetSignalTot(3))");
  tree->SetAlias("logRatioTot23", "log(fTPCdEdxInfo.GetSignalTot(2)/fTPCdEdxInfo.GetSignalTot(3))");
  tree->SetAlias("logRatioMax03", "log(fTPCdEdxInfo.GetSignalMax(0)/fTPCdEdxInfo.GetSignalMax(3))");  //
  tree->SetAlias("logRatioMax13", "log(fTPCdEdxInfo.GetSignalMax(1)/fTPCdEdxInfo.GetSignalMax(3))");  //
  tree->SetAlias("logRatioMax23", "log(fTPCdEdxInfo.GetSignalMax(2)/fTPCdEdxInfo.GetSignalMax(3))");
  tree->SetAlias("logRatioTot01", "log(fTPCdEdxInfo.GetSignalTot(0)/fTPCdEdxInfo.GetSignalTot(1))");
  tree->SetAlias("logRatioTot12", "log(fTPCdEdxInfo.GetSignalTot(1)/fTPCdEdxInfo.GetSignalTot(2))");
  tree->SetAlias("logRatioTot02", "log(fTPCdEdxInfo.GetSignalTot(0)/fTPCdEdxInfo.GetSignalTot(2))");
  tree->SetAlias("logRatioMax01", "log(fTPCdEdxInfo.GetSignalMax(0)/fTPCdEdxInfo.GetSignalMax(1))");  //
  tree->SetAlias("logRatioMax12", "log(fTPCdEdxInfo.GetSignalMax(1)/fTPCdEdxInfo.GetSignalMax(2))");  //
  tree->SetAlias("logRatioMax02", "log(fTPCdEdxInfo.GetSignalMax(0)/fTPCdEdxInfo.GetSignalMax(2))");  //
  /// Faction of clusters and n-crossed rows
  tree->SetAlias("nclFractionROCA", "(esdTrack.GetTPCClusterInfo(3,0)+0)");
  tree->SetAlias("nclFractionROC0", "(esdTrack.GetTPCClusterInfo(3,0,0,63)+0)");
  tree->SetAlias("nclFractionROC1", "(esdTrack.GetTPCClusterInfo(3,0,64,128)+0)");
  tree->SetAlias("nclFractionROC2", "(esdTrack.GetTPCClusterInfo(3,0,129,159)+0)");
  tree->SetAlias("nclFractionROC3", "(esdTrack.GetTPCClusterInfo(3,0)+0)");
  tree->SetAlias("ncrROCA", "(esdTrack.GetTPCClusterInfo(3,1)+0)");
  tree->SetAlias("nCross0", "esdTrack.fTPCdEdxInfo.GetNumberOfCrossedRows(0)");
  tree->SetAlias("nCross1", "esdTrack.fTPCdEdxInfo.GetNumberOfCrossedRows(1)");
  tree->SetAlias("nCross2", "esdTrack.fTPCdEdxInfo.GetNumberOfCrossedRows(2)");
  tree->SetAlias("nFraction0", "esdTrack.GetTPCClusterInfo(1,0,0,62)");
  tree->SetAlias("nFraction1", "esdTrack.GetTPCClusterInfo(1,0,63,127)");
  tree->SetAlias("nFraction2", "esdTrack.GetTPCClusterInfo(1,0,127,159)");
  tree->SetAlias("nFraction3", "esdTrack.GetTPCClusterInfo(1,0,0,159)");
  tree->SetAlias("n3Fraction0", "esdTrack.GetTPCClusterInfo(3,0,0,62)");
  tree->SetAlias("n3Fraction1", "esdTrack.GetTPCClusterInfo(3,0,63,127)");
  tree->SetAlias("n3Fraction2", "esdTrack.GetTPCClusterInfo(3,0,127,159)");
  tree->SetAlias("n3Fraction3", "esdTrack.GetTPCClusterInfo(3,0,0,159)");
  //
  for (Int_t i = 0; i < 3; i++) {
    TStatToolkit::AddMetadata(tree, Form("nCross%d.AxisTitle", i), Form("# crossed (ROC%d)", i));
    TStatToolkit::AddMetadata(tree, Form("nclFractionROC%d.AxisTitle", i), Form("fraction of cl (ROC%d)", i));
    TStatToolkit::AddMetadata(tree, Form("nFraction%d.AxisTitle", i), Form("p_{cl1}(ROC%d)", i));
    TStatToolkit::AddMetadata(tree, Form("n3Fraction%d.AxisTitle", i), Form("p_{cl3}(ROC%d)", i));
  }
  for (Int_t i = 0; i < 4; i++) {
    TStatToolkit::AddMetadata(tree, Form("ratioTotMax%d.AxisTitle", i), Form("Q_{max%d}/Q_{tot%d}", i, i));
    TStatToolkit::AddMetadata(tree, Form("logTotMax%d.AxisTitle", i), Form("log(Q_{max%d}/Q_{tot%d})", i, i));
    TStatToolkit::AddMetadata(tree, Form("ratioTotMax%d.Title", i), Form("Q_{max%d}/Q_{tot%d}", i, i));
    TStatToolkit::AddMetadata(tree, Form("logTotMax%d.Title", i), Form("log(Q_{max%d}/Q_{tot%d})", i, i));
  }
  for (Int_t i = 0; i < 3; i++) {
    for (Int_t j = i + 1; j < 4; j++) {
      TStatToolkit::AddMetadata(tree, Form("logRatioMax%d%d.AxisTitle", i, j), Form("log(Q_{MaxRPC%d}/Q_{MaxROC%d})", i, j));
      TStatToolkit::AddMetadata(tree, Form("logRatioTot%d%d.AxisTitle", i, j), Form("log(Q_{TotRPC%d}/Q_{TotROC%d})", i, j));
    }
  }
  //

  tree->SetAlias("track_tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0-1)",1));
  tree->SetAlias("track_tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0-1)",1));
  tree->SetAlias("track_tpcNsigma_ka",Form("AliPIDtools::NumberOfSigmas(%d,1, 3,0-1)",1));
  tree->SetAlias("track_tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0-1)",1));
  tree->SetAlias("track_tpcNsigma_deut",Form("AliPIDtools::NumberOfSigmas(%d,1,5,0-1)",1));
  tree->SetAlias("track_tpcNsigma_tri", Form("AliPIDtools::NumberOfSigmas(%d,1,6,0-1)",1));
  tree->SetAlias("track_tpcNsigma_He3", Form("AliPIDtools::NumberOfSigmas(%d,1,7,0-1)",1));

  tree->SetAlias("track_ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignal(%d,0,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignal(%d,2,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignal(%d,3,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignal(%d,4,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_deut", Form("AliPIDtools::GetExpectedTPCSignal(%d,5,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_tri", Form("AliPIDtools::GetExpectedTPCSignal(%d,6,0x7,0+0)",1));
  tree->SetAlias("track_ExpectedTPCSignalV0_He3", Form("AliPIDtools::GetExpectedTPCSignal(%d,7,0x7,0+0)",1));

  tree->SetAlias("track_CorrectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignal(%d,0,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignal(%d,2,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignal(%d,3,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignal(%d,4,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_deut",Form("AliPIDtools::GetExpectedTPCSignal(%d,5,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_tri", Form("AliPIDtools::GetExpectedTPCSignal(%d,6,0x7,1+0)",1));
  tree->SetAlias("track_CorrectedTPCSignalV0_He3", Form("AliPIDtools::GetExpectedTPCSignal(%d,7,0x7,1+0)",1));

  tree->SetAlias("track_GetPileupValue",Form("AliPIDtools::GetExpectedTPCSignal(%d,0,0x8,1+0)",1));
  tree->SetAlias("Nucleitrigger_OFF","selectionPtMask>0");
  tree->SetAlias("track_hasTOF","(esdTrack.GetStatus()&(0x80000000)) && (esdTrack.GetStatus()&(0x2000))");

  tree->SetAlias("dca_r","esdTrack.fD");
  tree->SetAlias("dca_z","esdTrack.fZ");
  tree->SetAlias("nCrossRows","(esdTrack.GetTPCClusterInfo(3,1)+0)");
}




void makeAliasesV0(){
  // selection for clean V0s
  treeV0->SetAlias("V0Like","exp(-acos(v0.fPointAngle)*v0.fRr/0.36)*exp(-sqrt(kf.fChi2)/0.5)");
  treeV0->SetAlias("cleanV0","sqrt(v0.GetKFInfo(2,2,2))<3&&v0.GetKFInfo(2,2,1)<0.05");
  treeV0->SetAlias("cleanK0","cleanV0&&K0Selected&&K0Like>0.1");
  treeV0->SetAlias("cleanGamma","cleanV0&&GammaSelected&&ELike>0.1");
  treeV0->SetAlias("cleanLambda","cleanV0&&LambdaSelected&&LLike>0.05");
  treeV0->SetAlias("cleanALambda","cleanV0&&ALambdaSelected&&ALLike>0.1");

  treeV0->SetAlias("track0P","track0.fIp.P()");
  treeV0->SetAlias("track1P","track1.fIp.P()");
  treeV0->SetAlias("track0Pt","track0.fIp.Pt()");
  treeV0->SetAlias("track1Pt","track1.fIp.Pt()");

  treeV0->SetAlias("track0Eta","track0.fIp.Eta()");
  treeV0->SetAlias("track1Eta","track1.fIp.Eta()");

  treeV0->SetAlias("track0Phi","track0.fIp.Phi()");
  treeV0->SetAlias("track1Phi","track1.fIp.Phi()");

  treeV0->SetAlias("track0Px","track0.fIp.Px()");
  treeV0->SetAlias("track1Px","track1.fIp.Px()");

  treeV0->SetAlias("track0Py","track0.fIp.Py()");
  treeV0->SetAlias("track1Py","track1.fIp.Py()");

  treeV0->SetAlias("track0Pz","track0.fIp.Pz()");
  treeV0->SetAlias("track1Pz","track1.fIp.Pz()");


   treeV0->SetAlias("track0Tgl","track0.fIp.fP[3]");
   treeV0->SetAlias("track1Tgl","track1.fIp.fP[3]");
  // treeV0->SetAlias("causalityCut","max(v0.fCausality[1],v0.fCausality[0])<0.3");
  // treeV0->SetAlias("causality0","v0.fCausality[0]");
  // treeV0->SetAlias("causality1","v0.fCausality[1]");


  treeV0->SetAlias("track0tofNsigmaElectron","tofNsigma0.fElements[0]");
  treeV0->SetAlias("track0tofNsigmaPion","tofNsigma0.fElements[2]");
  treeV0->SetAlias("track0tofNsigmaKaon","tofNsigma0.fElements[3]");
  treeV0->SetAlias("track0tofNsigmaProton","tofNsigma0.fElements[4]");

  treeV0->SetAlias("track1tofNsigmaElectron","tofNsigma1.fElements[0]");
  treeV0->SetAlias("track1tofNsigmaPion","tofNsigma1.fElements[2]");
  treeV0->SetAlias("track1tofNsigmaKaon","tofNsigma1.fElements[3]");
  treeV0->SetAlias("track1tofNsigmaProton","tofNsigma1.fElements[4]");

  treeV0->SetAlias("track0itsNsigmaElectron","itsNsigma0.fElements[0]");
  treeV0->SetAlias("track0itsNsigmaPion","itsNsigma0.fElements[2]");
  treeV0->SetAlias("track0itsNsigmaKaon","itsNsigma0.fElements[3]");
  treeV0->SetAlias("track0itsNsigmaProton","itsNsigma0.fElements[4]");

  treeV0->SetAlias("track1itsNsigmaElectron","itsNsigma1.fElements[0]");
  treeV0->SetAlias("track1itsNsigmaPion","itsNsigma1.fElements[2]");
  treeV0->SetAlias("track1itsNsigmaKaon","itsNsigma1.fElements[3]");
  treeV0->SetAlias("track1itsNsigmaProton","itsNsigma1.fElements[4]");

  //
//  treeV0->SetAlias("track0tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0)",pidHash));
//  treeV0->SetAlias("track0tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0)",pidHash));
//  treeV0->SetAlias("track0tpcNsigma_ka",Form("AliPIDtools::NumberOfSigmas(%d,1, 3,0+0)",pidHash));
//  treeV0->SetAlias("track0tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0)",pidHash));

//  treeV0->SetAlias("track1tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0)",pidHash));
//  treeV0->SetAlias("track1tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0)",pidHash));
//  treeV0->SetAlias("track1tpcNsigma_ka",Form("AliPIDtools::NumberOfSigmas(%d,1, 3,1+0)",pidHash));
//  treeV0->SetAlias("track1tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0)",pidHash));
  //
//  treeV0->SetAlias("track0ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x1, 0,0+0)",pidHash));
//  treeV0->SetAlias("track0ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x1, 0,0+0)",pidHash));
//  treeV0->SetAlias("track0ExpectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x1, 0,0+0)",pidHash));
//  treeV0->SetAlias("track0ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x1, 0,0+0)",pidHash));

//  treeV0->SetAlias("track1ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x1, 1,0+0)",pidHash));
//  treeV0->SetAlias("track1ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x1, 1,0+0)",pidHash));
// treeV0->SetAlias("track1ExpectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x1, 1,0+0)",pidHash));
//  treeV0->SetAlias("track1ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x1, 1,0+0)",pidHash));

  // for the corercted new splines,
  treeV0->SetAlias("track0tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0)",1));
  treeV0->SetAlias("track0tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0)",1));
  treeV0->SetAlias("track0tpcNsigma_ka",Form("AliPIDtools::NumberOfSigmas(%d,1, 3,0+0)",1));
  treeV0->SetAlias("track0tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0)",1));

  treeV0->SetAlias("track1tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0)",1));
  treeV0->SetAlias("track1tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0)",1));
  treeV0->SetAlias("track1tpcNsigma_ka",Form("AliPIDtools::NumberOfSigmas(%d,1, 3,1+0)",1));
  treeV0->SetAlias("track1tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x7, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x7, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x7, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x7, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x7, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x7, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x7, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x7, 1,0+0)",1));

  treeV0->SetAlias("track0CorrectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x7, 0,1+0)",1));
  treeV0->SetAlias("track0CorrectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x7, 0,1+0)",1));
  treeV0->SetAlias("track0CorrectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x7, 0,1+0)",1));
  treeV0->SetAlias("track0CorrectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x7, 0,1+0)",1));

  treeV0->SetAlias("track1CorrectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x7, 1,1+0)",1));
  treeV0->SetAlias("track1CorrectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x7, 1,1+0)",1));
  treeV0->SetAlias("track1CorrectedTPCSignalV0_ka",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,3,0x7, 1,1+0)",1));
  treeV0->SetAlias("track1CorrectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x7, 1,1+0)",1));

  treeV0->SetAlias("track0GetPileupValue",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x8, 0,1+0)",1));
  treeV0->SetAlias("track1GetPileupValue",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x8, 1,1+0)",1));

  treeV0->SetAlias("dca0_r","track0.fD");
  treeV0->SetAlias("dca0_z","track0.fZ");
  treeV0->SetAlias("nCrossRows0","(track0.GetTPCClusterInfo(3,1)+0)");
  treeV0->SetAlias("dca1_r","track1.fD");                                                                                                                                                                    treeV0->SetAlias("dca1_z","track1.fZ");
  treeV0->SetAlias("nCrossRows1","(track1.GetTPCClusterInfo(3,1)+0)");


    for (Int_t i=0; i<4; i++){
    treeV0->SetAlias(Form("logSignalTot_track0_%d",i) , Form("track0.fTPCdEdxInfo.GetSignalTot(0+%d)",i));
    treeV0->SetAlias(Form("logSignalMax_track0_%d",i) , Form("track0.fTPCdEdxInfo.GetSignalMax(0+%d)",i));
    treeV0->SetAlias(Form("signalNcl_track0_%d",i) , Form("(track0.fTPCdEdxInfo.GetNumberOfClusters(0+%d))",i));
    treeV0->SetAlias(Form("signalNcr_track0_%d",i) , Form("(track0.fTPCdEdxInfo.GetNumberOfCrossedRows(0+%d))",i));

    treeV0->SetAlias(Form("logSignalTot_track1_%d",i) , Form("track1.fTPCdEdxInfo.GetSignalTot(0+%d)",i));
    treeV0->SetAlias(Form("logSignalMax_track1_%d",i) , Form("track1.fTPCdEdxInfo.GetSignalMax(0+%d)",i));
    treeV0->SetAlias(Form("signalNcl_track1_%d",i) , Form("(track1.fTPCdEdxInfo.GetNumberOfClusters(0+%d))",i));
    treeV0->SetAlias(Form("signalNcr_track1_%d",i) , Form("(track1.fTPCdEdxInfo.GetNumberOfCrossedRows(0+%d))",i));

  }


}

void makeAliasesEvent(){
//  treeEvent->SetAlias("shiftA","tpcVertexInfo.fElements[0]");
//  treeEvent->SetAlias("shiftC","tpcVertexInfo.fElements[1]");
//  treeEvent->SetAlias("shiftM","0.5*(tpcVertexInfo.fElements[1]+tpcVertexInfo.fElements[0])-25.");
//  treeEvent->SetAlias("multPA","tpcVertexInfo.fElements[3]");
//  treeEvent->SetAlias("multPC","tpcVertexInfo.fElements[4]");
//  treeEvent->SetAlias("multP","(tpcVertexInfo.fElements[3]+tpcVertexInfo.fElements[4])");
//  treeEvent->SetAlias("nPileUpSum","(tpcVertexInfo.fElements[3]+tpcVertexInfo.fElements[4])");
//  treeEvent->SetAlias("nPileUpSumCorr","(tpcVertexInfo.fElements[3]+tpcVertexInfo.fElements[4])-0.05*multITSTPC");
//  treeEvent->SetAlias("nPileUpPrim","nPileUpSumCorr/(1-abs(shiftM/210))");   /// normalized pileup multiplicity
  //
  treeEvent->SetAlias("shiftA","tpcVertexInfoESD.fElements[0]");
  treeEvent->SetAlias("shiftC","tpcVertexInfoESD.fElements[1]");
  treeEvent->SetAlias("shiftM","0.5*(tpcVertexInfoESD.fElements[1]+tpcVertexInfoESD.fElements[0])-25.");
  treeEvent->SetAlias("multPA","tpcVertexInfoESD.fElements[3]");
  treeEvent->SetAlias("multPC","tpcVertexInfoESD.fElements[4]");
  treeEvent->SetAlias("multP","(tpcVertexInfoESD.fElements[3]+tpcVertexInfoESD.fElements[4])");
  treeEvent->SetAlias("nPileUpSum","(tpcVertexInfoESD.fElements[3]+tpcVertexInfoESD.fElements[4])");
  treeEvent->SetAlias("nPileUpSumCorr","(tpcVertexInfoESD.fElements[3]+tpcVertexInfoESD.fElements[4])-0.05*multITSTPC");
  treeEvent->SetAlias("nPileUpPrim","nPileUpSumCorr/(1-abs(shiftM/210))");   /// normalized pileup multiplicity

  //
  treeEvent->SetAlias("multSSD","(itsClustersPerLayer.fElements[4]+itsClustersPerLayer.fElements[5])");
  treeEvent->SetAlias("multSDD","(itsClustersPerLayer.fElements[2]+itsClustersPerLayer.fElements[3])");
  treeEvent->SetAlias("multSPD","(itsClustersPerLayer.fElements[0]+itsClustersPerLayer.fElements[1])");
  treeEvent->SetAlias("multV0","vZeroMult.Sum()");
  treeEvent->SetAlias("multT0","tZeroMult.Sum()");
  ///
  treeEvent->SetAlias("multV0A","Sum$(vZeroMult.fElements*(Iteration$<32))");
  treeEvent->SetAlias("multV0C","Sum$(vZeroMult.fElements*(Iteration$>=32))");
  treeEvent->SetAlias("multV0A0","Sum$(vZeroMult.fElements*(Iteration$<32)*((Iteration$%2)==0))");
  treeEvent->SetAlias("multV0A1","Sum$(vZeroMult.fElements*(Iteration$<32)*((Iteration$%2)==1))");
  treeEvent->SetAlias("multV0C0","Sum$(vZeroMult.fElements*(Iteration$>=32)*((Iteration$%2)==0))");
  treeEvent->SetAlias("multV0C1","Sum$(vZeroMult.fElements*(Iteration$>=32)*((Iteration$%2)==1))");
  //
  treeEvent->SetAlias("multT0A","Sum$(tZeroMult.fElements*(Iteration$<12))");
  treeEvent->SetAlias("multT0C","Sum$(tZeroMult.fElements*(Iteration$>=12))");
  treeEvent->SetAlias("multITSA","phiCountAITS.Sum()");
  treeEvent->SetAlias("multITSC","phiCountCITS.Sum()");
  treeEvent->SetAlias("multITSA0","Sum$(phiCountAITS.fElements*((Iteration$%2)==0))");
  treeEvent->SetAlias("multITSA1","Sum$(phiCountAITS.fElements*((Iteration$%2)==1))");
  treeEvent->SetAlias("multITSC0","Sum$(phiCountCITS.fElements*((Iteration$%2)==0))");
  treeEvent->SetAlias("multITSC1","Sum$(phiCountCITS.fElements*((Iteration$%2)==1))");

  ///
  treeEvent->SetAlias("pileUpMatchAmp","abs(log((150+multPA)/(150+multPC)))<0.3");
  //treeEvent->SetAlias("pileUpMatchLike","sqrt(((shiftC-shiftA)/11)^2+(log((150+tpcVertexInfoESD.fElements[3])/(150+tpcVertexInfo.fElements[4]))/0.06)^2)");
  //treeEvent->SetAlias("cutTail","abs(tpcVertexInfo.fElements[1]+tpcVertexInfo.fElements[0]-16)>5");
  treeEvent->SetAlias("pileUpMatchLike","sqrt(((shiftC-shiftA)/11)^2+(log((150+tpcVertexInfoESD.fElements[3])/(150+tpcVertexInfoESD.fElements[4]))/0.06)^2)");
  treeEvent->SetAlias("cutTail","abs(tpcVertexInfoESD.fElements[1]+tpcVertexInfoESD.fElements[0]-16)>5");

  treeEvent->SetAlias("pileUpOK","(pileUpMatchLike<5&&cutTail&&multPA/primMult>0.15)");
  //
  treeEvent->SetAlias("centV0","centrality.fElements[0]");
  treeEvent->SetAlias("centITS0","centrality.fElements[1]");
  treeEvent->SetAlias("centITS1","centrality.fElements[2]");
  //
  treeEvent->SetAlias("cutPileUpSharp","abs(multITSTPC/tpcMult-1)<0.2&&abs(multITSTPC/tpcMult)<1000");
  treeEvent->SetAlias("multITSTPC","((multSSD+multSDD)/2.38)");
  treeEvent->SetAlias("cut1000","tpcMult-multITSTPC<1000");
  treeEvent->SetAlias("cut2000","tpcMult-multITSTPC<2000");
  treeEvent->SetAlias("cut4000","tpcMult-multITSTPC<4000");
}


void cacheCleanV0(Int_t entries=-1, Int_t firstEntry=0, Int_t chunkSize=100000){
  /// cache clean V0 removing non relevant branches
  /// To add  - all dEdx, farctoion of clusters, n-crossed
  ::Info("cacheCleanV0","Start");
   timer.Start();
  treeV0->SetBranchStatus("track0.fCp",0);
  treeV0->SetBranchStatus("track1.fCp",0);
  treeV0->SetBranchStatus("track0.fOp",0);
  treeV0->SetBranchStatus("track1.fOp",0);
  treeV0->SetBranchStatus("track0.fTPCInner",0);
  treeV0->SetBranchStatus("track1.fTPCInner",0);
  treeV0->SetBranchStatus("fileName.",0);
  treeV0->SetAlias("track0status","track0->GetStatus()");
  treeV0->SetAlias("track1status","track1->GetStatus()");
  treeV0->SetAlias("track0_hasTOF","(track0.GetStatus()&(0x80000000)) && (track0.GetStatus()&(0x2000))");
  treeV0->SetAlias("track1_hasTOF","(track1.GetStatus()&(0x80000000)) && (track1.GetStatus()&(0x2000))");
  treeV0->SetAlias("isMinBias","triggerMask&(0x1)");
  treeV0->SetAlias("isPileUp", "(multSDD+multSSD) < (-3000 +0.0099*tpcClusterMult +9.426e-10*tpcClusterMult*tpcClusterMult)");
  treeV0->SetAlias("tpc_cls0","track0.fTPCncls");
  treeV0->SetAlias("tpc_cls1","track1.fTPCncls");

  gROOT->cd();
  treeV0->RemoveFriend(treeEvent);
  if (entries<0) entries = treeV0->GetEntries();
  TTree * treeClean = treeV0->CopyTree("(cleanK0||cleanGamma||cleanLambda||cleanALambda)","",entries,firstEntry);
  treeV0->AddFriend(treeEvent,"E");
  treeClean->BuildIndex("gid");
  treeClean->AddFriend(treeEvent,"E");
  TList*aliases = treeV0->GetListOfAliases();
  for (Int_t i=0; i<aliases->GetEntries(); i++) treeClean->SetAlias(aliases->At(i)->GetName(), aliases->At(i)->GetTitle());

  TString cacheVariables=""
    //"run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
    "gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
    "v0.fPointAngle:kf.fChi2:K0Like:ELike:LLike:ALLike:cleanK0:cleanGamma:cleanLambda:cleanALambda:track0status:track1status:track0_hasTOF:track1_hasTOF:"
    "track0.fTPCsignal:track1.fTPCsignal:track0.fTPCsignalN:track1.fTPCsignalN:type:track0.fITSsignal:track1.fITSsignal:"
    "track0P:track0Pt:track0Eta:track0Phi:track0Px:track0Py:track0Pz:track0Tgl:dca0_r:dca0_z:nCrossRows0:tpc_cls0:"
    "track1P:track1Pt:track1Eta:track1Phi:track1Px:track1Py:track1Pz:track1Tgl:dca1_r:dca1_z:nCrossRows1:tpc_cls1:"
    "track0tofNsigmaElectron:track0tofNsigmaPion:track0tofNsigmaKaon:track0tofNsigmaProton:"
    "track1tofNsigmaElectron:track1tofNsigmaPion:track1tofNsigmaKaon:track1tofNsigmaProton:"
    "track0tpcNsigma_el:track0tpcNsigma_pi:track0tpcNsigma_ka:track0tpcNsigma_pro:"
    "track1tpcNsigma_el:track1tpcNsigma_pi:track1tpcNsigma_ka:track1tpcNsigma_pro:"
    "track0ExpectedTPCSignalV0_el:track0ExpectedTPCSignalV0_pi:track0ExpectedTPCSignalV0_ka:track0ExpectedTPCSignalV0_pro:"
    "track1ExpectedTPCSignalV0_el:track1ExpectedTPCSignalV0_pi:track1ExpectedTPCSignalV0_ka:track1ExpectedTPCSignalV0_pro:"
    "track0CorrectedTPCSignalV0_el:track0CorrectedTPCSignalV0_pi:track0CorrectedTPCSignalV0_ka:track0CorrectedTPCSignalV0_pro:"
    "track1CorrectedTPCSignalV0_el:track1CorrectedTPCSignalV0_pi:track1CorrectedTPCSignalV0_ka:track1CorrectedTPCSignalV0_pro:"
    "track0GetPileupValue:track1GetPileupValue:"
//    "track0tpcNsigma_corrected_el:track0tpcNsigma_corrected_pi:track0tpcNsigma_corrected_ka:track0tpcNsigma_corrected_pro:"
//    "track1tpcNsigma_corrected_el:track1tpcNsigma_corrected_pi:track1tpcNsigma_corrected_ka:track1tpcNsigma_corrected_pro:"
//    "track0ExpectedTPCSignalV0_corrected_el:track0ExpectedTPCSignalV0_corrected_pi:track0ExpectedTPCSignalV0_corrected_ka:track0ExpectedTPCSignalV0_corrected_pro:"
//    "track1ExpectedTPCSignalV0_corrected_el:track1ExpectedTPCSignalV0_corrected_pi:track1ExpectedTPCSignalV0_corrected_ka:track1ExpectedTPCSignalV0_corrected_pro:"
    "centV0:centITS0:centITS1:tpcClusterMult:multSSD:multSDD:tpcTrackBeforeClean:triggerMask:isMinBias:isPileUp:"
    "logSignalTot_track0_0:logSignalTot_track0_1:logSignalTot_track0_2:logSignalTot_track0_3:"
    "logSignalMax_track0_0:logSignalMax_track0_1:logSignalMax_track0_2:logSignalMax_track0_3:"
    "signalNcl_track0_0:signalNcl_track0_1:signalNcl_track0_2:signalNcl_track0_3:"
    "signalNcr_track0_0:signalNcr_track0_1:signalNcr_track0_2:signalNcr_track0_3:"
    "logSignalTot_track1_0:logSignalTot_track1_1:logSignalTot_track1_2:logSignalTot_track1_3:"
    "logSignalMax_track1_0:logSignalMax_track1_1:logSignalMax_track1_2:logSignalMax_track1_3:"
    "signalNcl_track1_0:signalNcl_track1_1:signalNcl_track1_2:signalNcl_track1_3:"
    "signalNcr_track1_0:signalNcr_track1_1:signalNcr_track1_2:signalNcr_track1_3";                    /// to add QA variables




  AliPIDtools::SetFilteredTreeV0(treeClean);
  AliTreePlayer::MakeCacheTreeChunk(treeClean,cacheVariables.Data(),"V0tree.root","V0Flat","isMinBias==1",entries,firstEntry,chunkSize);
  TFile *fV0Clean= new TFile("V0tree.root","update");
  treeClean->Write("V0s");
  fV0Clean->Flush();
  //AliTreePlayer::MakeC
  fV0Clean->Close();
  delete fV0Clean;
  AliPIDtools::SetFilteredTreeV0(treeV0);
  ::Info("cacheCleanV0","End");
  timer.Print();
}
/*

void CacheCleanV0Tracks(){
  /// Machine learning (e.g random forest) classification to be used in near future to  provide more clean sample

  treeClean->SetAlias("acceptGammaITS","(track0.fITSsignal<110&&track1.fITSsignal<110)");
  treeClean->SetAlias("acceptGammaTOF","((tofNsigma0.fElements[0]<-998)||abs(tofNsigma0.fElements[0])<3)&&((tofNsigma1.fElements[0]<-998)||abs(tofNsigma1.fElements[0])<3)");
  treeClean->SetAlias("acceptGamma0","acceptGammaITS&&acceptGammaTOF&&track1.fTPCsignal>65");
  treeClean->SetAlias("acceptGamma1","acceptGammaITS&&acceptGammaTOF&&track0.fTPCsignal>65");
}
*/


void cacheEventFlat(){
  ::Info("cacheEventFlat","BEGIN");
  timer.Start();
  treeEvent->SetAlias("isMinBias","triggerMask&(0x1)");
  treeEvent->SetAlias("isPileUp", "(multSDD+multSSD) < (-3000 +0.0099*tpcClusterMult +9.426e-10*tpcClusterMult*tpcClusterMult)");

  TString cacheVariables=""
                          "run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
                          //"gid:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "multSSD:multSDD:multSPD:multV0:multT0:spdvz:itsTracklets:tpcMult:tpcTrackBeforeClean:" /// raw multiplicities
                          "multV0A:multV0C:multT0A:multT0C:multITSA:multITSC:"
                          "multV0A0:multV0C0:multV0A1:multV0C1:multITSA0:multITSC0:multITSA1:multITSC1:"
                          "centV0:centITS0:centITS1";                    /// to add QA variables
  AliTreePlayer::MakeCacheTree(treeEvent,cacheVariables.Data(),"EventTree.root","EventFlat","isMinBias==1");
   ::Info("cacheEventFlat","END");
   timer.Print();
}

void cacheCleanTrack(){
   ::Info("cacheCleanTrack","BEGIN");
   timer.Start();
  tree->SetAlias("trackP","esdTrack.fIp.P()");
  tree->SetAlias("trackPt","esdTrack.fIp.Pt()");
  tree->SetAlias("tgl","esdTrack.fIp.fP[3]");
  tree->SetAlias("fTPCsignal","esdTrack.fTPCsignal");
  tree->SetAlias("trackEta","esdTrack.Eta()");
  tree->SetAlias("trackstatus","esdTrack.GetStatus()");
  tree->SetAlias("tracktofNsigmaElectron","tofNsigma.fElements[0]");
  tree->SetAlias("tracktofNsigmaPion","tofNsigma.fElements[2]");
  tree->SetAlias("tracktofNsigmaKaon","tofNsigma.fElements[3]");
  tree->SetAlias("tracktofNsigmaProton","tofNsigma.fElements[4]");
  tree->SetAlias("tracktofNsigmaDeuteron","tofNsigma.fElements[5]");
  tree->SetAlias("tracktofNsigmaTrition","tofNsigma.fElements[6]");
  tree->SetAlias("tracktofNsigmaHelium3","tofNsigma.fElements[7]");

  tree->SetAlias("trackitsNsigmaElectron","itsNsigma.fElements[0]");
  tree->SetAlias("trackitsNsigmaPion","itsNsigma.fElements[2]");
  tree->SetAlias("trackitsNsigmaKaon","itsNsigma.fElements[3]");
  tree->SetAlias("trackitsNsigmaProton","itsNsigma.fElements[4]");
  tree->SetAlias("trackitsNsigmaDeuteron","itsNsigma.fElements[5]");
  tree->SetAlias("trackitsNsigmaTrition","itsNsigma.fElements[6]");
  tree->SetAlias("trackitsNsigmaHelium3","itsNsigma.fElements[7]");

  tree->SetAlias("isMinBias","triggerMask&(0x1)");
  tree->SetAlias("isPileUp", "(multSDD+multSSD) < (-3000 +0.0099*tpcClusterMult +9.426e-10*tpcClusterMult*tpcClusterMult)");
  tree->SetAlias("tpc_cls","esdTrack.fTPCncls");

  TString cacheVariables=""
                          "fTPCsignal:esdTrack.fITSsignal:esdTrack.fTRDsignal:esdTrack.fTPCsignalN:trackP:trackPt:tgl:trackEta:tpc_cls:ntracks:trackstatus:"
                          "track_tpcNsigma_el:track_tpcNsigma_pi:track_tpcNsigma_ka:track_tpcNsigma_pro:track_tpcNsigma_deut:track_tpcNsigma_tri:track_tpcNsigma_He3:"
                          "tracktofNsigmaElectron:tracktofNsigmaPion:tracktofNsigmaKaon:tracktofNsigmaProton:tracktofNsigmaDeuteron:tracktofNsigmaTrition:tracktofNsigmaHelium3:"
                          "track_ExpectedTPCSignalV0_el:track_ExpectedTPCSignalV0_pi:track_ExpectedTPCSignalV0_ka:track_ExpectedTPCSignalV0_pro:track_ExpectedTPCSignalV0_deut:"
                          "track_ExpectedTPCSignalV0_tri:track_ExpectedTPCSignalV0_He3:"
		                      "track_CorrectedTPCSignalV0_el:track_CorrectedTPCSignalV0_pi:track_CorrectedTPCSignalV0_ka:track_CorrectedTPCSignalV0_pro:track_CorrectedTPCSignalV0_deut:"
                          "track_CorrectedTPCSignalV0_tri:track_CorrectedTPCSignalV0_He3:"
                          "track_GetPileupValue:dca_r:dca_z:nCrossRows:"
			                    "run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
                          //"gid:shiftM:nPileUpPrim:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "multSSD:multSDD:multSPD:multV0:multT0:spdvz:itsTracklets:tpcMult:tpcTrackBeforeClean:" /// raw multiplicities
                          "centV0:centITS0:centITS1:"
			                    "TPCRefit:ITSRefit:Nucleitrigger_OFF:track_hasTOF:isMinBias:isPileUp:"
                          "logSignalTot0:logSignalTot1:logSignalTot2:logSignalTot3:"
                          "logSignalMax0:logSignalMax1:logSignalMax2:logSignalMax3:"
                          "signalNcl0:signalNcl1:signalNcl2:signalNcl3:"
                          "signalNcr0:signalNcr1:signalNcr2:signalNcr3";                                        /// to add QA variables

  AliTreePlayer::MakeCacheTree(tree,cacheVariables.Data(),"CleanTrack.root","CleanTrackFlat","isMinBias==1");
 ::Info("cacheTrackHighMass","END");
   timer.Print();



}




void cacheTrackHighMass(){
   ::Info("cacheTrackHighMass","BEGIN");
   timer.Start();
  tree->SetAlias("cutHighdEdx","log(esdTrack.fTPCsignal/(-0.4+AliExternalTrackParam::BetheBlochAleph(esdTrack.fIp.P()/0.93)))>4.6");
  tree->SetAlias("trackP","esdTrack.fIp.P()");
  tree->SetAlias("tgl","esdTrack.fP[3]");
  TString cacheVariables=""
                         "esdTrack.fTPCsignal:esdTrack.fITSsignal:esdTrack.fTRDsignal:trackP:tgl:"
                          "run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
                          //"gid:shiftM:nPileUpPrim:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "multSSD:multSDD:multSPD:multV0:multT0:spdvz:itsTracklets:tpcMult:tpcTrackBeforeClean:" /// raw multiplicities
                          "centV0:centITS0:centITS1";                    /// to add QA variables
  AliTreePlayer::MakeCacheTree(tree,cacheVariables.Data(),"HighMassTree.root","HighMassFlat","isSelected&&(selectionPIDMask&1)>0&&cutHighdEdx");
 ::Info("cacheTrackHighMass","END");
   timer.Print();
}


void InitTree(Int_t nChunks, Int_t buildIndex, Int_t run) {
  AliPIDtools pid;
  //pidHash = pid.LoadPID(run, 3, "pass3", 0);
   //locad tree and make aliases
  tree = AliXRDPROOFtoolkit::MakeChain("filtered.list", "highPt", 0, nChunks);
  treeV0 = AliXRDPROOFtoolkit::MakeChain("filtered.list", "V0s", 0, nChunks);
  treeEvent = AliXRDPROOFtoolkit::MakeChain("filtered.list", "events", 0, nChunks);
  if (treeEvent->GetEntries()<=0) treeEvent=NULL;
  tree->SetCacheSize(500000000);
  treeV0->SetCacheSize(500000000);
  if (treeEvent) treeEvent->SetCacheSize(500000000);
  AliAnalysisTaskFilteredTree::SetDefaultAliasesHighPt(tree);
  AliAnalysisTaskFilteredTree::SetDefaultAliasesV0(treeV0);
  makeAliasesV0();
  if (treeEvent&& buildIndex > 0) {
    treeEvent->BuildIndex("gid");
    tree->BuildIndex("gid");
    treeV0->BuildIndex("gid");
    tree->AddFriend(treeEvent, "E");
    treeV0->AddFriend(treeEvent, "E");
  }
  makeAliasesTracks(tree);
  makeAliasesV0();
  if (treeEvent) makeAliasesEvent();
  /// 1.) Exercise example : trigger exceptional pileup - and check them
  if (treeEvent&&buildIndex > 1) {
    tree->Draw(">>entryList", "primMult>50&&tpcMult>50&&primMult/tpcMult<0.5", "entrylist");
    tree->SetEntryList((TEntryList *) gROOT->FindObject("entryList"));
  }
  ///
  tree->SetMarkerSize(0.6);
  tree->SetMarkerStyle(21);
  tree->SetAlias("pidHash",Form("(%d+0)",pidHash));
  treeV0->SetAlias("pidHash",Form("(%d+0)",pidHash));
  //tree->SetAlias("primMult","SPDVertex.fNContributors");
  AliPIDtools::SetFilteredTree(tree);
  AliPIDtools::SetFilteredTreeV0(treeV0);

  //
}
