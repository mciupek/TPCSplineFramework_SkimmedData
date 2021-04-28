/*
  //gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");

  .L  $NOTES/JIRA/PWGPP-538/fitdEdxCorrectionFiltered.C+g
  AliDrawStyle::SetDefaults();
  AliDrawStyle::ApplyStyle("figTemplate");
  gStyle->SetOptTitle(1);
  InitTree(5);
  LoadFits();
  enablePileUpCorrection()
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
#include "AliESDtools.h"
#include "AliAnalysisTaskFilteredTree.h"
#include "AliNDLocalRegression.h"
#include "AliPIDtools.h"
#include "AliTPCPIDResponse.h"
#include "AliPIDResponse.h"
#include "AliESDv0.h"
#include "AliESDEvent.h"

TTree * tree =0;
TTree * treeV0=0;
TTree * treeEvent =0;
TTree * treeMap=0;
TTree * treeMapTracks=0;
TTree * treeMapAll=0;
TTree * treeV0Flat=0;
TStopwatch timer;
AliESDtools tools;
Int_t pidHash=0;
TVectorF *centrality=new TVectorF(2);
AliESDv0 *v0 =0;


TMatrixD projectionInfo(5,5);
void InitTree(Int_t nChunks, Int_t buildIndex=1, Int_t run=246272);
void MakeHistograms(Int_t nPoints=100000000);

void fitdEdxCorrectionFiltered(){
  ::Info("fitdEdxCorrectionFiltered","Dummy");
}

void fitPileUpCorection(Int_t nChunks){
  InitTree(nChunks);
  MakeHistograms();
}

Double_t getCentrality(Int_t cindex, Int_t previous){
  static TVectorF lastCentrality(27);
  static Int_t lastIndex=-1;
  Float_t value=0;
  if (previous) {
    value=lastCentrality[cindex];
  }
  else
    value= (*centrality)[cindex];
  if (lastIndex!=treeEvent->GetReadEntry()) {
    lastCentrality = *centrality;
    lastIndex=treeEvent->GetReadEntry();
  }
  return value;
}

void registerPIDResponse(Int_t pidIndex, const char * pileUpCorrection="dEdxFitLight.root", const char* TPCpidResponse="tpcPIDResponse.root", const char*TPCetaMap="tpcPIDEtaMap.root"){

}

void enablePileUpCorrection(){
  AliPIDtools::GetTPCPID(pidHash).SetPileupCorrectionObject(AliPIDtools::GetTPCPID(pidHash).GetPileupCorrectionFromFile("dEdxFitLight.root"));
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
}


void registerPID(Int_t pidIndex=1, int run = 296690, Int_t passNumber  =3, TString recoPass="pass3",
		 const char * aodbPath="/lustre/nyx/alice/users/xbai/work/SkimmedDataAna/Run18/TPCsplines/TreePrduction/lib/OADB",
		 const Char_t *  spline_path="/lustre/nyx/alice/users/mciupek/TPCSpline/Splines/LHC18r/withoutPileup/TPCPIDResponseOADB_2020_07_22_18r_pass3_It8.root",
		 const Char_t *  eta_crrection_path="/lustre/nyx/alice/users/mciupek/TPCSpline/Splines/LHC18r/withoutPileup/TPCetaMaps_2020_07_22_18r_pass3_It8.root",
		 const Char_t *  pileUpCorrection="/lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/PWGPP-538/new/alice/data/2018/LHC18r/pass3/AOD_FilterTrees/fit_version_08062020/dEdxFitLight.root"
		 ){

  cout<<"******************** custmer input **************************"<<endl;
  cout<<aodbPath<<endl;
  cout<<spline_path<<endl;
  cout<<eta_crrection_path<<endl;
  cout<<pileUpCorrection<<endl;
  cout<<"******************** custmer input **************************"<<endl;

  AliESDEvent ev;
  AliPIDResponse *fPIDResponse = new AliPIDResponse;
  fPIDResponse->SetUseTPCMultiplicityCorrection(false);
  fPIDResponse->SetUseTPCEtaCorrection(true);
  fPIDResponse->SetUseTPCPileupCorrection(true);
  
   fPIDResponse->SetCustomTPCpidResponseOADBFile(Form("%s",spline_path));
   fPIDResponse->SetCustomTPCetaMaps(Form("%s",eta_crrection_path));
  fPIDResponse->SetOADBPath(aodbPath);
  fPIDResponse->InitialiseEvent(&ev,passNumber, recoPass, run);

  AliPIDtools::pidAll[pidIndex]=fPIDResponse;
  AliPIDtools::pidTPC[pidIndex]=&(fPIDResponse->GetTPCResponse());


  //  if( pileUpCorrection)AliPIDtools::GetTPCPID(pidIndex).SetPileupCorrectionObject(AliPIDtools::GetTPCPID(pidIndex).GetPileupCorrectionFromFile(pileUpCorrection));

}



void makeAliasesV0(){

  // selection for clean V0s
  treeV0->SetAlias("V0Like","exp(-acos(v0.fPointAngle)*v0.fRr/0.36)*exp(-sqrt(kf.fChi2)/0.5)");
  treeV0->SetAlias("cleanV0","sqrt(v0.GetKFInfo(2,2,2))<3&&v0.GetKFInfo(2,2,1)<0.05");
  treeV0->SetAlias("cleanK0","cleanV0&&K0Selected&&K0Like>0.1");
  treeV0->SetAlias("cleanGamma","cleanV0&&GammaSelected&&ELike>0.1");
  treeV0->SetAlias("cleanLambda","cleanV0&&LambdaSelected&&LLike>0.05");
  treeV0->SetAlias("cleanALambda","cleanV0&&ALambdaSelected&&ALLike>0.1");
  treeV0->SetAlias("pairDCA_InerTPC", "90*(track0.fIp.GetParameterAtRadius(90,5,7)-track1.fIp.GetParameterAtRadius(90,5,7))");

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



  treeV0->SetAlias("track0TPCchi2","track0.fTPCchi2");
  treeV0->SetAlias("track1TPCchi2","track1.fTPCchi2");


  treeV0->SetAlias("track0DCAxy","track0.fD");
  treeV0->SetAlias("track0DCAz","track0.fZ");

  treeV0->SetAlias("track1DCAxy","track1.fD");
  treeV0->SetAlias("track1DCAz","track1.fZ");

  treeV0->SetAlias("track0ncrossrow","(track0.GetTPCClusterInfo(3,1)+0)");
  treeV0->SetAlias("track1ncrossrow","(track1.GetTPCClusterInfo(3,1)+0)");





  


  // treeV0->SetAlias("track0Tgl","track0.fIp.fP[3]");
  // treeV0->SetAlias("track1Tgl","track1.fIp.fP[3]");
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
  



  //
  treeV0->SetAlias("track0tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0)",pidHash));
  treeV0->SetAlias("track0tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0)",pidHash));
  treeV0->SetAlias("track0tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0)",pidHash));

  treeV0->SetAlias("track1tpcNsigma_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0)",pidHash));
  treeV0->SetAlias("track1tpcNsigma_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0)",pidHash));
  treeV0->SetAlias("track1tpcNsigma_pro",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0)",pidHash));
  //
  treeV0->SetAlias("track0ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x1, 0,0+0)",pidHash));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x1, 0,0+0)",pidHash));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x1, 0,0+0)",pidHash));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0x1, 1,0+0)",pidHash));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0x1, 1,0+0)",pidHash));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_pro",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0x1, 1,0+0)",pidHash));


  // No correction

  treeV0->SetAlias("track0tpcNsigma_no_corrected_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,0)",1));
  treeV0->SetAlias("track0tpcNsigma_no_corrected_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,0)",1));
  treeV0->SetAlias("track0tpcNsigma_no_corrected_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,0)",1));

  treeV0->SetAlias("track1tpcNsigma_no_corrected_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,0)",1));
  treeV0->SetAlias("track1tpcNsigma_no_corrected_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,0)",1));
  treeV0->SetAlias("track1tpcNsigma_no_corrected_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,0)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_no_corrected_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_no_corrected_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_no_corrected_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_no_corrected_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,0, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_no_corrected_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,0, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_no_corrected_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,0, 1,0+0)",1));



  // for the corercted new splines, // eta correlation
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,1)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,1)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,1)",1));

  treeV0->SetAlias("track1tpcNsigma_corrected_eta_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,1)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,1)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,1)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,1, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,1, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,1, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,1, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,1, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,1, 1,0+0)",1));

  // for the corercted new splines, // multiplicity correlation

  treeV0->SetAlias("track0tpcNsigma_corrected_multip_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,2)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_multip_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,2)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_multip_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,2)",1));

  treeV0->SetAlias("track1tpcNsigma_corrected_multip_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,2)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_multip_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,2)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_multip_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,2)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_multip_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,2, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_multip_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,2, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_multip_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,2, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_multip_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,2, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_multip_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,2, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_multip_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,2, 1,0+0)",1));


  // for the corercted new splines, // pileup correlation

  treeV0->SetAlias("track0tpcNsigma_corrected_pileup_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,4)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_pileup_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,4)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_pileup_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,4)",1));

  treeV0->SetAlias("track1tpcNsigma_corrected_pileup_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,4)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_pileup_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,4)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_pileup_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,4)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_pileup_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,4, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_pileup_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,4, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_pileup_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,4, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_pileup_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,4, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_pileup_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,4, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_pileup_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,4, 1,0+0)",1));

  
  // for the corercted new splines, // eta and multiplicity correlation

  treeV0->SetAlias("track0tpcNsigma_corrected_eta_multip_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,3)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_multip_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,3)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_multip_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,3)",1));

  treeV0->SetAlias("track1tpcNsigma_corrected_eta_multip_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,3)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_multip_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,3)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_multip_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,3)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_multip_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,3, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_multip_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,3, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_multip_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,3, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_multip_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,3, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_multip_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,3, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_multip_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,3, 1,0+0)",1));

  // for the corercted new splines, // eta and pileup correlation

  treeV0->SetAlias("track0tpcNsigma_corrected_eta_pileup_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,0+0,7)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_pileup_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,0+0,7)",1));
  treeV0->SetAlias("track0tpcNsigma_corrected_eta_pileup_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,0+0,7)",1));

  treeV0->SetAlias("track1tpcNsigma_corrected_eta_pileup_el",Form("AliPIDtools::NumberOfSigmas(%d,1, 0,1+0,7)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_pileup_pi",Form("AliPIDtools::NumberOfSigmas(%d,1, 2,1+0,7)",1));
  treeV0->SetAlias("track1tpcNsigma_corrected_eta_pileup_pr",Form("AliPIDtools::NumberOfSigmas(%d,1, 4,1+0,7)",1));

  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_pileup_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,7, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_pileup_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,7, 0,0+0)",1));
  treeV0->SetAlias("track0ExpectedTPCSignalV0_corrected_eta_pileup_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,7, 0,0+0)",1));

  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_pileup_el",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,0,7, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_pileup_pi",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,2,7, 1,0+0)",1));
  treeV0->SetAlias("track1ExpectedTPCSignalV0_corrected_eta_pileup_pr",Form("AliPIDtools::GetExpectedTPCSignalV0(%d,4,7, 1,0+0)",1));
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
  //   timer.Start();
  treeV0->SetBranchStatus("track0.fCp",0);
  treeV0->SetBranchStatus("track1.fCp",0);
  treeV0->SetBranchStatus("track0.fOp",0);
  treeV0->SetBranchStatus("track1.fOp",0);
  treeV0->SetBranchStatus("track0.fTPCInner",0);
  treeV0->SetBranchStatus("track1.fTPCInner",0);
  treeV0->SetBranchStatus("fileName.",0);
  gROOT->cd();
  //treeV0->RemoveFriend(treeEvent);
  if (entries<0) entries = treeV0->GetEntries();
  TFile *fV0Clean= new TFile("V0tree.root","update");
  TTree * treeClean = treeV0->CopyTree("(cleanK0||cleanGamma||cleanLambda||cleanALambda)","",entries,firstEntry);
  treeClean->Write("V0s");
  delete fV0Clean;
  fV0Clean= new TFile("V0tree.root");
  treeClean = (TTree*)fV0Clean->Get("V0s");
  TList*aliases = treeV0->GetListOfAliases();
  for (Int_t i=0; i<aliases->GetEntries(); i++) treeClean->SetAlias(aliases->At(i)->GetName(), aliases->At(i)->GetTitle());
  treeClean->BuildIndex("gid");
  treeClean->RemoveFriend(treeClean->GetFriend("E"));
  treeClean->AddFriend(treeEvent,"E");

  TString cacheVariables=""
    //"run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
    //"gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
    "nPileUpSum:nPileUpSumCorr:nPileUpPrim:"
    "v0.fPointAngle:kf.fChi2:K0Like:ELike:LLike:ALLike:cleanK0:cleanGamma:cleanLambda:cleanALambda:"
    "centV0:centITS0:centITS1:tpcClusterMult:multSSD:multSDD:tpcTrackBeforeClean:triggerMask:pairDCA_InerTPC:"                 
    "track0.fTPCsignal:track1.fTPCsignal:track0.fTPCsignalN:track1.fTPCsignalN:type:"
    "track0P:track0Pt:track0Eta:track0Phi:track0Px:track0Py:track0Pz:track0DCAxy:track0DCAz:track0ncrossrow:track0TPCchi2:"
    "track1P:track1Pt:track1Eta:track1Phi:track1Px:track1Py:track1Pz:track1DCAxy:track1DCAz:track1ncrossrow:track1TPCchi2:"
    "track0tofNsigmaElectron:track0tofNsigmaPion:track0tofNsigmaKaon:track0tofNsigmaProton:"
    "track1tofNsigmaElectron:track1tofNsigmaPion:track1tofNsigmaKaon:track1tofNsigmaProton:"

    // "track0tpcNsigma_el:track0tpcNsigma_pi:track0tpcNsigma_ka:track0tpcNsigma_pr:"
    // "track1tpcNsigma_el:track1tpcNsigma_pi:track1tpcNsigma_ka:track1tpcNsigma_pr:"
    // "track0ExpectedTPCSignalV0_el:track0ExpectedTPCSignalV0_pi:track0ExpectedTPCSignalV0_ka:track0ExpectedTPCSignalV0_pr:"
    // "track1ExpectedTPCSignalV0_el:track1ExpectedTPCSignalV0_pi:track1ExpectedTPCSignalV0_ka:track1ExpectedTPCSignalV0_pr:"
    //    "track0tpcNsigma_corrected_eta_el:"


    "track0tpcNsigma_no_corrected_el:track0tpcNsigma_no_corrected_pi:track0tpcNsigma_no_corrected_pr:"
    "track1tpcNsigma_no_corrected_el:track1tpcNsigma_no_corrected_pi:track1tpcNsigma_no_corrected_pr:"

    "track0ExpectedTPCSignalV0_no_corrected_el:track0ExpectedTPCSignalV0_no_corrected_pi:track0ExpectedTPCSignalV0_no_corrected_pr:"
    "track1ExpectedTPCSignalV0_no_corrected_el:track1ExpectedTPCSignalV0_no_corrected_pi:track1ExpectedTPCSignalV0_no_corrected_pr:"


    "track0tpcNsigma_corrected_eta_el:track0tpcNsigma_corrected_eta_pi:track0tpcNsigma_corrected_eta_pr:"
    "track1tpcNsigma_corrected_eta_el:track1tpcNsigma_corrected_eta_pi:track1tpcNsigma_corrected_eta_pr:"
    "track0ExpectedTPCSignalV0_corrected_eta_el:track0ExpectedTPCSignalV0_corrected_eta_pi:track0ExpectedTPCSignalV0_corrected_eta_pr:"
    "track1ExpectedTPCSignalV0_corrected_eta_el:track1ExpectedTPCSignalV0_corrected_eta_pi:track1ExpectedTPCSignalV0_corrected_eta_pr:"


    "track0tpcNsigma_corrected_multip_el:track0tpcNsigma_corrected_multip_pi:track0tpcNsigma_corrected_multip_pr:"
    "track1tpcNsigma_corrected_multip_el:track1tpcNsigma_corrected_multip_pi:track1tpcNsigma_corrected_multip_pr:"
    "track0ExpectedTPCSignalV0_corrected_multip_el:track0ExpectedTPCSignalV0_corrected_multip_pi:track0ExpectedTPCSignalV0_corrected_multip_pr:"
    "track1ExpectedTPCSignalV0_corrected_multip_el:track1ExpectedTPCSignalV0_corrected_multip_pi:track1ExpectedTPCSignalV0_corrected_multip_pr:"

    "track0tpcNsigma_corrected_pileup_el:track0tpcNsigma_corrected_pileup_pi:track0tpcNsigma_corrected_pileup_pr:"
    "track1tpcNsigma_corrected_pileup_el:track1tpcNsigma_corrected_pileup_pi:track1tpcNsigma_corrected_pileup_pr:"
    "track0ExpectedTPCSignalV0_corrected_pileup_el:track0ExpectedTPCSignalV0_corrected_pileup_pi:track0ExpectedTPCSignalV0_corrected_pileup_pr:"
    "track1ExpectedTPCSignalV0_corrected_pileup_el:track1ExpectedTPCSignalV0_corrected_pileup_pi:track1ExpectedTPCSignalV0_corrected_pileup_pr:"

    
    
    "track0tpcNsigma_corrected_eta_multip_el:track0tpcNsigma_corrected_eta_multip_pi:track0tpcNsigma_corrected_eta_multip_pr:"
    "track1tpcNsigma_corrected_eta_multip_el:track1tpcNsigma_corrected_eta_multip_pi:track1tpcNsigma_corrected_eta_multip_pr:"
    "track0ExpectedTPCSignalV0_corrected_eta_multip_el:track0ExpectedTPCSignalV0_corrected_eta_multip_pi:track0ExpectedTPCSignalV0_corrected_eta_multip_pr:"
    "track1ExpectedTPCSignalV0_corrected_eta_multip_el:track1ExpectedTPCSignalV0_corrected_eta_multip_pi:track1ExpectedTPCSignalV0_corrected_eta_multip_pr:"

    "track0tpcNsigma_corrected_eta_pileup_el:track0tpcNsigma_corrected_eta_pileup_pi:track0tpcNsigma_corrected_eta_pileup_pr:"
    "track1tpcNsigma_corrected_eta_pileup_el:track1tpcNsigma_corrected_eta_pileup_pi:track1tpcNsigma_corrected_eta_pileup_pr:"
    "track0ExpectedTPCSignalV0_corrected_eta_pileup_el:track0ExpectedTPCSignalV0_corrected_eta_pileup_pi:track0ExpectedTPCSignalV0_corrected_eta_pileup_pr:"
    "track1ExpectedTPCSignalV0_corrected_eta_pileup_el:track1ExpectedTPCSignalV0_corrected_eta_pileup_pi:track1ExpectedTPCSignalV0_corrected_eta_pileup_pr";

  ;
    // to add QA variables
  AliPIDtools::SetFilteredTreeV0(treeClean);
  AliTreePlayer::MakeCacheTreeChunk(treeClean,cacheVariables.Data(),"V0tree.root","V0Flat","1",entries,firstEntry,chunkSize,"update");
  AliPIDtools::SetFilteredTreeV0(treeV0);
  ::Info("cacheCleanV0","End");
  //  timer.Print();
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
  //  timer.Start();
  TString cacheVariables=""
                          "run:intrate:timeStampS:timestamp:bField:triggerMask:"     /// run properties
                          //"gid:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "gid:shiftA:shiftC:shiftM:nPileUpPrim:nPileUpSum:primMult:tpcClusterMult:pileUpOK:" /// pileup event properties
                          "multSSD:multSDD:multSPD:multV0:multT0:spdvz:itsTracklets:tpcMult:tpcTrackBeforeClean:" /// raw multiplicities
                          "multV0A:multV0C:multT0A:multT0C:multITSA:multITSC:"
                          "multV0A0:multV0C0:multV0A1:multV0C1:multITSA0:multITSC0:multITSA1:multITSC1:"
                          "centV0:centITS0:centITS1";                    /// to add QA variables
  AliTreePlayer::MakeCacheTree(treeEvent,cacheVariables.Data(),"EventTree.root","EventFlat","1");
   ::Info("cacheEventFlat","END");
  //   timer.Print();
}

void cacheTrackHighMass(){
   ::Info("cacheTrackHighMass","BEGIN");
  //   timer.Start();
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
  //  timer.Print();
}


void InitTree(Int_t nChunks, Int_t buildIndex, Int_t run) {
  AliPIDtools pid;
  pidHash = pid.LoadPID(run, 1, "pass1", 0);
  // locad tree and make aliases
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
    treeEvent->SetBranchAddress("centrality.",&centrality);
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
  //
  AliPIDtools::SetFilteredTree(tree);
  AliPIDtools::SetFilteredTreeV0(treeV0);
  treeV0->SetBranchAddress("v0.",&v0);
}

void initFlatV0(){
  TFile * fflatV0= TFile::Open("V0Flat.root");
  treeV0Flat = (TTree*)fflatV0->Get("V0Flat");
  if (treeMap!= nullptr) {
    treeV0Flat->SetAlias("fitA", TString(treeMap->GetAlias("hdEdxAShifttMNTglDist.meanGFitA")).ReplaceAll("Center", ""));
    treeV0Flat->SetAlias("fit0", TString(treeMap->GetAlias("hdEdxAShifttMNTglDist.meanGFit0")).ReplaceAll("Center", ""));
    //
    treeV0Flat->SetAlias("fitA0", TString(treeMap->GetAlias("hdEdxAShifttMNTglDist.meanGFitA")).ReplaceAll("Center", "").ReplaceAll("atgl", "abs(track0Tgl)"));
    treeV0Flat->SetAlias("fitA1", TString(treeMap->GetAlias("hdEdxAShifttMNTglDist.meanGFitA")).ReplaceAll("Center", "").ReplaceAll("atgl", "abs(track1Tgl)"));
  }
    treeV0Flat->SetAlias("dEdx0ExpK0", "AliExternalTrackParam::BetheBlochAleph(track0P/0.139)");
    treeV0Flat->SetAlias("dEdx1ExpK0", "AliExternalTrackParam::BetheBlochAleph(track1P/0.139)");
  treeV0Flat->SetAlias("multITSTPC","((multSSD+multSDD)/2.38)");
  treeV0Flat->SetAlias("cut1000","tpcMult-multITSTPC<1000");
  treeV0Flat->SetAlias("cut2000","tpcMult-multITSTPC<2000");
  treeV0Flat->SetAlias("cut4000","tpcMult-multITSTPC<4000");
}

void MakeHistograms(Int_t nPoints){
  //  timer.Start();
  ::Info("MakeHistograms","Begin");
  //
  TString hisString="";
  hisString+="logdEdx:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hdEdxAShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logdEdx:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<-0.0>>hdEdxCShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   //
   hisString+="logTotMax0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hTotMax0AShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logTotMax0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<-0.0>>hTotMax0CShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logTotMax1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hTotMax1AShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logTotMax1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<-0.0>>hTotMax1CShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logTotMax2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hTotMax2AShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
   hisString+="logTotMax2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<-0.0>>hTotMax2CShifttMNTgl(50,-0.5,0.5,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  //
  hisString+="logQMaxMIP0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQMaxMIP0AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQMaxMIP0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQMaxMIP0CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQMaxMIP1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQMaxMIP1AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQMaxMIP1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQMaxMIP1CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQMaxMIP2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQMaxMIP2AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQMaxMIP2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQMaxMIP2CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  //
  hisString+="logQTotMIP0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQTotMIP0AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQTotMIP0:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQTotMIP0CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQTotMIP1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQTotMIP1AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQTotMIP1:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQTotMIP1CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQTotMIP2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl>0.0>>hQTotMIP2AShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="logQTotMIP2:shiftM:nPileUpPrim:primMult:atgl:#itsMIPCutP&tgl<0.0>>hQTotMIP2CShifttMNTgl(70,-0.7,0.7,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";

  //
  hisString+="nFraction0:shiftM:nPileUpPrim:primMult:atgl:#1>>hFraction0ShifttMNTgl(60,0.3,1.1,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="nFraction1:shiftM:nPileUpPrim:primMult:atgl:#1>>hFraction1ShifttMNTgl(60,0.3,1.1,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="nFraction2:shiftM:nPileUpPrim:primMult:atgl:#1>>hFraction2ShifttMNTgl(60,0.3,1.1,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  hisString+="nFraction3:shiftM:nPileUpPrim:primMult:atgl:#1>>hFraction3ShifttMNTgl(60,0.3,1.1,30,-180,180,15,-100,8000,15,0,3500,15,0,1);";
  //
  hisString+="nFraction0:multTPCClusterN:atgl:mdEdx:#1>>hFraction0Mult_Tgl_mdEdx(60,0.3,1.1, 20,0,2,15,0,1,15,0,1);";
  hisString+="nFraction1:multTPCClusterN:atgl:mdEdx:#1>>hFraction1Mult_Tgl_mdEdx(60,0.3,1.1, 20,0,2,15,0,1,15,0,1);";
  hisString+="nFraction2:multTPCClusterN:atgl:mdEdx:#1>>hFraction2Mult_Tgl_mdEdx(60,0.3,1.1, 20,0,2,15,0,1,15,0,1);";
  hisString+="nFraction3:multTPCClusterN:atgl:mdEdx:#1>>hFraction3Mult_Tgl_mdEdx(60,0.3,1.1, 20,0,2,15,0,1,15,0,1);";
  //
  hisString+="normChi2TPC:multTPCClusterN:atgl:mdEdx:#1>>hnormChi2TPCMult_Tgl_mdEdx(60,0.2,4, 20,0,2,15,0,1,15,0,1);";
  hisString+="normChi2ITS:multTPCClusterN:atgl:mdEdx:#1>>hnormChi2ITSMult_Tgl_mdEdx(60,0.2,4, 20,0,2,15,0,1,15,0,1);";


  //
  TObjArray *hisArray = AliTreePlayer::MakeHistograms(tree, hisString, TString("(pileUpOK||nPileUpPrim/primMult<0.3)&&isSelected"), 0, nPoints, 20000, 15);
  for (Int_t i = 0; i < hisArray->GetEntries(); i++) gROOT->Add(hisArray->At(i));
  TTreeSRedirector *pcstream = new TTreeSRedirector("hisdEx.root", "recreate");
  pcstream->GetFile()->cd();
  hisArray->Write("hisArray", TObjArray::kSingleKey);
  delete pcstream;
  ::Info("MakeHistograms","End");
  //  timer.Print();
}

void MakeHistogramsTracks(Int_t nPoints){
  //
  //
  // parameters:
  //    fraction of associated  cluster  - nFractionI
  //    TPC chi2
  //    ITS chi2
  //    TPC-ITS matching  (0,1)
  //    dEdx/dEdxExp       - for pions
  //    TPC DCA rphi, z    -
  // binning variables:
  //     multiplicity       - number of clusters or number of tracks
  //     q/pt               -
  //     inclination angle  - tgl
  //     1/dEdx             - 1/ combined TPC dEdx
  //     local phi
  //     or identified particles = pions uisng ITS

  //  timer.Start();
  ::Info("MakeHistogramsTracks","Begin");

  TString hisString="";
  hisString+="nFraction0:multTPCClusterN:atgl:mdEdx:#1>>hFraction0Mult_Tgl_mdEdx(60,0.3,1.1, 10,0,2, 15,0,1,15,0,1);";
  hisString+="nFraction1:multTPCClusterN:atgl:mdEdx:#1>>hFraction1Mult_Tgl_mdEdx(60,0.3,1.1, 10,0,2, 15,0,1,15,0,1);";
  hisString+="nFraction2:multTPCClusterN:atgl:mdEdx:#1>>hFraction2Mult_Tgl_mdEdx(60,0.3,1.1, 10,0,2, 15,0,1,15,0,1);";
  hisString+="nFraction3:multTPCClusterN:atgl:mdEdx:#1>>hFraction3Mult_Tgl_mdEdx(60,0.3,1.1, 10,0,2, 15,0,1,15,0,1);";
  hisString+="normChi2TPC:multTPCClusterN:atgl:mdEdx:#1>>hnormChi2TPCMult_Tgl_mdEdx(60,0.2,4, 10,0,2, 15,0,1,15,0,1);";
  hisString+="normChi2ITS:multTPCClusterN:atgl:mdEdx:#1>>hnormChi2ITSMult_Tgl_mdEdx(60,0.2,4, 10,0,2, 15,0,1,15,0,1);";
  //
  hisString+="nFraction0:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hFraction0Mult_Tgl_qPt(60,0.3,1.1, 10,0,2, 15,0,1,20,-5,5);";
  hisString+="nFraction1:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hFraction1Mult_Tgl_qPt(60,0.3,1.1, 10,0,2, 15,0,1,20,-5,5);";
  hisString+="nFraction2:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hFraction2Mult_Tgl_qPt(60,0.3,1.1, 10,0,2, 15,0,1,20,-5,5);";
  hisString+="nFraction3:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hFraction3Mult_Tgl_qPt(60,0.3,1.1, 10,0,2, 15,0,1,20,-5,5);";
  hisString+="normChi2TPC:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hnormChi2TPCMult_Tgl_qPt(60,0.2,4, 10,0,2, 15,0,1,20,-5,5);";
  hisString+="normChi2ITS:multTPCClusterN:atgl:qPt:#itsMIPCutP>>hnormChi2ITSMult_Tgl_qPt(60,0.2,4, 10,0,2, 15,0,1,20,-5,5);";
  // resolution
  hisString+="deltaP0:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisDeltaP0_dSec(50,-2,2, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="pullP0:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisPullP0_dSec(50,-2,2, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="deltaP2:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisDeltaP2_dSec(50,-2,2, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="pullP2:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisPullP2_dSec(50,-2,2, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="normChi2TPC:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisnormChi2TPC_dSec(50,0.2,4, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="normChi2ITS:multTPCClusterN:dSector:atgl:qPt:#IsPrim4>>hisnormChi2ITS_dSec(50,0.2,4, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  // dEdxMIP
  hisString+="logdEdx:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisdEdxMIP_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  //
  hisString+="logQMaxMIP0:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQMaxMIP0_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="logQMaxMIP1:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQMaxMIP1_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="logQMaxMIP2:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQMaxMIP2_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="logQTotMIP0:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQTotMIP0_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="logQTotMIP1:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQTotMIP1_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="logQTotMIP2:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisQTotMIP2_dSec(25,-0.6,0.6, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  // Ncl
  hisString+="nFraction0:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisFraction0_dSec(25,0.1,1.1, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="nFraction1:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisFraction1_dSec(25,0.1,1.1, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="nFraction2:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisFraction2_dSec(25,0.1,1.1, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="nFraction3:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisFraction3_dSec(25,0.1,1.1, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";

  hisString+="nCross0:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisnCross0_dSec(35,0.1,70, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="nCross1:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisnCross1_dSec(35,0.1,70, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="nCross2:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisnCross2_dSec(35,0.1,70, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  hisString+="ncrROCA:multTPCClusterN:dSector:atgl:qPt:#pionCut>>hisncrROCA_dSec(50,0.1,160.1, 10,0,2, 40,0,1, 10,0,1, 30, -3,3 );";
  //
  TObjArray *hisArray = AliTreePlayer::MakeHistograms(tree, hisString, TString("isSelected"), 0, nPoints, 20000, 15);
  for (Int_t i = 0; i < hisArray->GetEntries(); i++) gROOT->Add(hisArray->At(i));
  TTreeSRedirector *pcstream = new TTreeSRedirector("hisTracks.root", "recreate");
  pcstream->GetFile()->cd();
  hisArray->Write("hisArray", TObjArray::kSingleKey);
  delete pcstream;
  ::Info("MakeHistogramsTracks","End");
  //  timer.Print();
}


void makeMaps(){
  TFile *f = TFile::Open("hisdEdx.root");
  TObjArray* hisArray=(TObjArray*)f->Get("hisArray");
  TTreeSRedirector *pcstream = new TTreeSRedirector("mapdEdx.root", "recreate");
  projectionInfo(0,0)=0;  projectionInfo(0,1)=0;  projectionInfo(0,2)=0;
  projectionInfo(1,0)=1;  projectionInfo(1,1)=2;  projectionInfo(1,2)=0;
  projectionInfo(2,0)=2;  projectionInfo(2,1)=1;  projectionInfo(2,2)=0;
  projectionInfo(3,0)=3;  projectionInfo(3,1)=2;  projectionInfo(3,2)=0;
  projectionInfo(4,0)=4;  projectionInfo(4,1)=1;  projectionInfo(4,2)=0;
  for (Int_t iHis=0; iHis<hisArray->GetEntries(); iHis++) {
    THn *hisInput = (THn *) hisArray->At(iHis);
    ::Info("MakeResidualDistortionMaps", "%s\t%d\t%d\t%d", hisInput->GetName(), hisInput->GetNdimensions(), hisInput->GetNbins(), Int_t(hisInput->GetEntries()));
    TStatToolkit::MakeDistortionMapFast(hisInput,pcstream,projectionInfo,0,0.02);
  }
  delete pcstream;
}



void makeMapsTracks(){
  TFile *f = TFile::Open("hisTracks.root");
  TObjArray* hisArray=(TObjArray*)f->Get("hisArray");
  TTreeSRedirector *pcstream = new TTreeSRedirector("mapTracks.root", "recreate");
  projectionInfo(0,0)=0;  projectionInfo(0,1)=0;  projectionInfo(0,2)=0;
  projectionInfo(1,0)=1;  projectionInfo(1,1)=1;  projectionInfo(1,2)=0;
  projectionInfo(2,0)=2;  projectionInfo(2,1)=1;  projectionInfo(2,2)=0;
  projectionInfo(3,0)=3;  projectionInfo(3,1)=1;  projectionInfo(3,2)=0;
  projectionInfo(4,0)=4;  projectionInfo(4,1)=1;  projectionInfo(4,2)=0;
  for (Int_t iHis=0; iHis<hisArray->GetEntries(); iHis++) {
    THn *hisInput = (THn *) hisArray->At(iHis);
    ::Info("MakeResidualDistortionMaps", "%s\t%d\t%d\t%d", hisInput->GetName(), hisInput->GetNdimensions(), hisInput->GetNbins(), Int_t(hisInput->GetEntries()));
    TStatToolkit::MakeDistortionMapFast(hisInput,pcstream,projectionInfo,0,0.02);
    if (hisInput->GetNdimensions()<5) continue;
    //
    Int_t proj2[4]={0,1,3,4};
    THn *hisProj2 = hisInput->Projection(4,proj2);
    hisProj2->SetName(Form("%s2",hisInput->GetName()));
    TStatToolkit::MakeDistortionMapFast(hisProj2,pcstream,projectionInfo,0,0.02);
    Int_t proj1[4]={0,2,3,4};
    THn *hisProj1 = hisInput->Projection(4,proj1);
    hisProj1->SetName(Form("%s1",hisInput->GetName()));
    TStatToolkit::MakeDistortionMapFast(hisProj1,pcstream,projectionInfo,0,0.02);
   //
    Int_t proj12[4]={0,3,4};
    THn *hisProj12 = hisInput->Projection(3,proj12);
    hisProj1->SetName(Form("%s1",hisInput->GetName()));
    TStatToolkit::MakeDistortionMapFast(hisProj12,pcstream,projectionInfo,0,0.02);
    delete hisProj2;
    delete hisProj1;
    delete hisProj12;


  }
  delete pcstream;
}






void initMaps(){
  AliDrawStyle::SetDefaults();
  AliDrawStyle::ApplyStyle("figTemplate");
  gStyle->SetOptTitle(1);
  treeMap=AliTreePlayer::LoadTrees("echo mapdEdx.root", "(h.*)", "xxx", ".*", "", "");
  treeMap->SetMarkerStyle(21); treeMap->SetMarkerSize(0.6);
  treeMap->SetAlias("dEdxMapOK", "abs((hdEdxAShifttMNTglDist.meanG-hdEdxCShifttMNTglDist.meanG)-(hdEdxAShifttMNTglDist.binMedian-hdEdxCShifttMNTglDist.binMedian))<0.02&&entries>25&&hdEdxAShifttMNTglDist.rmsG>0&&hdEdxCShifttMNTglDist.rmsG>0");
}

void loadMaps(){
  AliDrawStyle::SetDefaults();
  AliDrawStyle::ApplyStyle("figTemplate");
  gStyle->SetOptTitle(1);
  treeMapAll=AliTreePlayer::LoadTrees("cat mapdEdx.list", "(h.*)", "_Tgl_mdEdx", ".*", "", "");
  treeMapTracks=AliTreePlayer::LoadTrees("cat mapdEdx.list", "(h.*_Tgl_mdEdx.*)", "xxx", ".*", "", "");
  treeMapAll->SetMarkerStyle(21); treeMapAll->SetMarkerSize(0.6);
}


void makeFits(){
  if (!treeMap) initMaps();
  TString range0="(18,0,1800,10,0,1)";
  TString kernel0="400:0.15";
  TString range="(30,-150,150,40,0,8000,15,0,3500,15,0,1)";
  TString kernel="20:400:400:0.15";
  //treeMap->SetAlias("mapOK","entries>25&&abs(hdEdxAShifttMNTglDist.meanG-hdEdxAShifttMNTglDist.binMedian)<0.05");
  treeMap->SetAlias("primMult","primMultCenter+(rndm-0.5)*400");
  treeMap->SetAlias("atgl","atglCenter+(rndm-0.5)*0.1");
  treeMap->SetAlias("shiftM","shiftMCenter+(rndm-0.5)*10");
  treeMap->SetAlias("nPileUpPrim","nPileUpPrimCenter+(rndm-0.5)*100");
  //
  treeMap->SetAlias("FitAOK","hdEdxAShifttMNTglDist.meanG!=0&&abs(hdEdxAShifttMNTglDist.binMedian-hdEdxAShifttMNTglDist.meanG)<0.05");
  treeMap->SetAlias("FitCOK","hdEdxCShifttMNTglDist.meanG!=0&&abs(hdEdxCShifttMNTglDist.binMedian-hdEdxCShifttMNTglDist.meanG)<0.05");
  treeMap->SetAlias("dEdxFitOK","FitAOK&&FitCOK");
  treeMap->SetAlias("statOK","min(hdEdxCShifttMNTglDist.entries,hdEdxAShifttMNTglDist.entries)>10");
  treeMap->SetAlias("hdEdxACShifttMNTglDist.binMedian","(hdEdxAShifttMNTglDist.binMedian+hdEdxAShifttMNTglDist.binMedian)*0.5");
  treeMap->SetAlias("hdEdxACShifttMNTglDist.meanG","(hdEdxAShifttMNTglDist.meanG+hdEdxAShifttMNTglDist.meanG)*0.5");

  ::Info("makeFits","fitASide0");
  AliNDLocalRegression *fitASide0 = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxAShifttMNTglDist.meanGFit0",range0, "hdEdxAShifttMNTglDist.meanG:0.1",  "primMult:atgl","FitAOK&&primMultCenter<1000", kernel0,0.01);
  ::Info("makeFits","fitASideA");
  AliNDLocalRegression *fitASideA = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxAShifttMNTglDist.meanGFitA",range, "hdEdxAShifttMNTglDist.meanG:0.1",  "shiftM:nPileUpPrim:primMult:atgl","FitAOK", kernel,0.05);
  ::Info("makeFits","fitCSide0");
   AliNDLocalRegression *fitCSide0 = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxCShifttMNTglDist.meanGFit0",range0, "hdEdxCShifttMNTglDist.meanG:0.1",  "primMult:atgl","FitCOK&&primMultCenter<1000", kernel0,0.01);
   ::Info("makeFits","fitCSideA");
  AliNDLocalRegression *fitCSideA = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxCShifttMNTglDist.meanGFitA",range, "hdEdxCShifttMNTglDist.meanG:0.1",  "shiftM:nPileUpPrim:primMult:atgl","FitCOK", kernel,0.05);
  //
   ::Info("makeFits","fitASideMed");
  AliNDLocalRegression *fitASideMed = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxAShifttMNTglDist.binMedianFitA",range, "hdEdxAShifttMNTglDist.binMedian:0.1",  "shiftM:nPileUpPrim:primMult:atgl","hdEdxAShifttMNTglDist.entries>10", kernel,0.05);
  ::Info("makeFits","fitCSideMed");
  AliNDLocalRegression *fitCSideMed = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxCShifttMNTglDist.binMedianFitA",range, "hdEdxCShifttMNTglDist.binMedian:0.1",  "shiftM:nPileUpPrim:primMult:atgl","hdEdxAShifttMNTglDist.entries>10", kernel,0.05);
  //
  ::Info("makeFits","fitACSideMed");
  AliNDLocalRegression *fitACSideMed = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxACShifttMNTglDist.binMedianFitA",range, "hdEdxACShifttMNTglDist.binMedian:0.1",  "shiftM:nPileUpPrim:primMult:atgl","statOK", kernel,0.05);
   ::Info("makeFits","fitACSideMeanG");
  AliNDLocalRegression *fitACSideA = AliNDLocalRegression::MakeRegression(treeMap,"hdEdxACShifttMNTglDist.meanGFitA",range, "hdEdxACShifttMNTglDist.meanG:0.1",  "shiftM:nPileUpPrim:primMult:atgl","dEdxFitOK", kernel,0.05);

  TObjArray * cors = AliNDLocalRegression::GetVisualCorrections();
  for (Int_t i=0; i<cors->GetEntriesFast(); i++){
    AliNDLocalRegression * regression=(AliNDLocalRegression *)cors->At(i);
    if (regression){
      regression->CleanCovariance();
    }
  }

  TFile *fout=TFile::Open("dEdxFit.root","recreate");

  fitASide0->Write("hdEdxAShifttMNTglDist_meanGFit0");
  fitASideA->Write("hdEdxAShifttMNTglDist_meanGFitA");
  fitCSide0->Write("hdEdxCShifttMNTglDist_meanGFit0");
  fitCSideA->Write("hdEdxCShifttMNTglDist_meanGFitA");
  fitACSideA->Write("hdEdxACShifttMNTglDist_meanGFitA");
  //
  fitASideMed->Write("hdEdxAShifttMNTglDist_binMedianFitA");
  fitCSideMed->Write("hdEdxCShifttMNTglDist_binMedianFitA");
  fitACSideMed->Write("hdEdxACShifttMNTglDist_binMedianFitA");
  fout->Close();
  //
  fout=TFile::Open("dEdxFitLight.root","recreate");
  fitACSideMed->Write("hdEdxACShifttMNTglDist_binMedianFitA");
  delete fout;
}

void compressRegression(){
  TFile fin("dEdxFitLight.root");
  TFile fout("dEdxFitLight1.root","recreate");
  AliNDLocalRegression*reg = (AliNDLocalRegression*)fin.Get("hdEdxACShifttMNTglDist_binMedianFitA");
  reg->CleanCovariance();
  reg->Write("hdEdxACShifttMNTglDist_binMedianFitA");
  fout.Close();
  //
  {
    TFile fin("dEdxFit.root");
    TFile fout("dEdxFitLightA.root","recreate");
    TList * keys = fin.GetListOfKeys();
    for (Int_t i=0;i<keys->GetEntries(); i++) {
      TString keyName=keys->At(i)->GetName();
      AliNDLocalRegression *reg = (AliNDLocalRegression *) fin.Get(keyName.Data());
      reg->CleanCovariance();
      reg->Write(keyName.Data());
    }
    fout.Close();
  }

}



void LoadFits(){
   if (!treeMap) initMaps();
  TFile *fin=TFile::Open("dEdxFit.root");
  TList * flist= fin->GetListOfKeys();
  TString fitName;
  AliNDLocalRegression *regression=0;
  for (Int_t i=0; i<flist->GetEntries(); i++){
    ::Info("LoadFits","%s", flist->At(i)->GetName());
    fitName=flist->At(i)->GetName();
    regression  = (AliNDLocalRegression *)fin->Get(fitName.Data());
    Int_t hashIndex=regression->GetVisualCorrectionIndex(regression->GetName());
    AliNDLocalRegression::AddVisualCorrection(regression, hashIndex);
    treeMap->SetAlias(regression->GetName(),TString::Format("AliNDLocalRegression::GetCorrND(%d,shiftMCenter,nPileUpPrimCenter,primMultCenter,atglCenter+0)",hashIndex).Data());
    //
    if (tree){
      //tree->SetAlias(regression->GetName(),TString::Format("AliNDLocalRegression::GetCorrND(%d,shiftMCenter,nPileUpPrimCenter,primMultCenter,atglCenter+0)",hashIndex).ReplaceAll("Center","").Data());
      tree->SetAlias(regression->GetName(),TString::Format("%s",treeMap->GetAlias(regression->GetName())).ReplaceAll("Center","").Data());
    }

  }

}



void drawQAK0(){
  treeV0Flat->Draw("track0.fTPCsignal/dEdx0ExpK0:track1.fTPCsignal/dEdx1ExpK0>>his(100,30,80,100,30,80)","type==8&&primMult<500&&nPileUpPrim>1000","colz");
  //treeV0Flat->Draw("((track0.fTPCsignal-0*fitA0)/dEdx0ExpK0):((track1.fTPCsignal-0*fitA1)/dEdx1ExpK0)>>his(100,30,80,100,30,80)","type==8&&primMult<250&&nPileUpPrim<1000","colz")
}



void drawMapStat(){
  gSystem->mkdir("fig");
  treeMap->Draw("hdEdxAShifttMNTglDist.entries:nPileUpPrimCenter:shiftMCenter>>his(30,-180,180,15,0,8000)","","profgoff");
  gPad->SetLogz();
  treeMap->GetHistogram()->GetXaxis()->SetTitle("pile- up z (cm)");
  treeMap->GetHistogram()->GetYaxis()->SetTitle("N_{trNorm}");
  treeMap->GetHistogram()->Draw("colz");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.entries_nPileUpPrimCenter_shiftMCenter.png");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.entries_nPileUpPrimCenter_shiftMCenter.pdf");
}

void drawCorrection(){
  treeMap->Draw("hdEdxAShifttMNTglDist.meanG:hdEdxCShifttMNTglDist.meanG>>his(100,-0.2,0.2,100,-0.2,0.2)","dEdxMapOK","colz");
  gPad->SetLogz(); gPad->SetLogy(0); gPad->SetGridx(0); gPad->SetGridy(0);
  treeMap->GetHistogram()->GetXaxis()->SetTitle("A side Map  dEdx_{corr}(unit)");
  treeMap->GetHistogram()->GetYaxis()->SetTitle("C side Map  dEdx_{corr}(unit)");
  treeMap->GetHistogram()->Draw("colz");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanG_hdEdxCShifttMNTglDist.meanG.pdf");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanG_hdEdxCShifttMNTglDist.meanG.png");
  //
  treeMap->Draw("hdEdxAShifttMNTglDist.meanGFitA:hdEdxCShifttMNTglDist.meanGFitA>>his(100,-0.2,0.2,100,-0.2,0.2)","dEdxMapOK","colz");
  treeMap->GetHistogram()->GetXaxis()->SetTitle("A side Map  dEdx_{corrfit}(unit)");
  treeMap->GetHistogram()->GetYaxis()->SetTitle("C side Map  dEdx_{corrfit}(unit)");
  gPad->SetLogz(); gPad->SetLogy(0); gPad->SetGridx(0); gPad->SetGridy(0);
  treeMap->GetHistogram()->Draw("colz");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanGFit_hdEdxCShifttMNTglDist.meanGFit.pdf");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanGFit_hdEdxCShifttMNTglDist.meanGFit.png");
  //
  // fit checks
  //
  treeMap->Draw("hdEdxAShifttMNTglDist.meanGFitA:hdEdxCShifttMNTglDist.meanGFitA+(hdEdxAShifttMNTglDist.binMedianFitA-hdEdxCShifttMNTglDist.binMedianFitA)>>his(100,-0.2,0.2,100,-0.2,0.2)","dEdxMapOK","colz");
  treeMap->GetHistogram()->GetXaxis()->SetTitle("A side Map  dEdx_{corrfit}-(dEdx_{ACmedianDiff})(unit)");
  treeMap->GetHistogram()->GetYaxis()->SetTitle("C side Map  dEdx_{corrfit}(unit)");
  gPad->SetLogz(); gPad->SetLogy(0); gPad->SetGridx(0); gPad->SetGridy(0);
  treeMap->GetHistogram()->Draw("colz");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanGFit_hdEdxCShifttMNTglDist.meanGFitbinMedian.pdf");
  gPad->SaveAs("fig/hdEdxAShifttMNTglDist.meanGFit_hdEdxCShifttMNTglDist.meanGFitbinMedian.png");
  //treeMap->Draw("hdEdxAShifttMNTglDist.meanGFitA-hdEdxCShifttMNTglDist.meanGFitA-(hdEdxAShifttMNTglDist.binMedianFitA-hdEdxCShifttMNTglDist.binMedianFitA)","dEdxMapOK","colz");
}


void QAK0(){
  gSystem->mkdir("figK0");
  LoadFits();
  initFlatV0();
  treeV0Flat->SetAlias("dEdxCorr0","((track0.fTPCsignal-1*fitA0*50.)/dEdx0ExpK0)");
  treeV0Flat->SetAlias("dEdxCorr1","((track1.fTPCsignal-1*fitA1*50.)/dEdx1ExpK0)");
  treeV0Flat->SetAlias("dEdxOrig0","((track0.fTPCsignal)/dEdx0ExpK0)");
  treeV0Flat->SetAlias("dEdxOrig1","((track1.fTPCsignal)/dEdx1ExpK0)");
  treeV0Flat->SetAlias("cleanK0","K0Like>0.8&&min(track0P,track1P)>0.3&&v0.fPointAngle>0.998");

  {treeV0Flat->Draw("dEdxCorr0:dEdxCorr1>>hisCorrK04000(100,30,80,100,30,80)","cleanK0&&!cut4000","colz");
  gPad->SaveAs("figK0/hisCorrK04000.pdf");
  gPad->SaveAs("figK0/hisCorrK04000.C");}

  {treeV0Flat->Draw("dEdxOrig0:dEdxOrig1>>hisOrigK04000(100,30,80,100,30,80)","cleanK0&&!cut4000","colz");
  gPad->SaveAs("figK0/hisOrigK04000.pdf");
  gPad->SaveAs("figK0/hisOrigK04000.C");}
}
