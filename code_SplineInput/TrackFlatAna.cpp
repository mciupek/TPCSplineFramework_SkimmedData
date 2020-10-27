
#include "TrackFlatAna.h"
#include <iostream>

ClassImp(TrackFlatAna);

TrackFlatAna::TrackFlatAna() {
  // nothing to do here. The class has no member data so
  // nothing to be initialized
}

TrackFlatAna::~TrackFlatAna() {
  // nothing to do here. The class has no member data so
  // nothing to be initialized
}

void TrackFlatAna::Loop()
{
  //   In a ROOT session, you can do:
  //      Root > .L h.C
  //      Root > h t
  //      Root > t.GetEntry(12); // Fill t data members with entry number 12
  //      Root > t.Show();       // Show values of entry 12
  //      Root > t.Show(16);     // Read and show values of entry 16
  //      Root > t.Loop();       // Loop on all entries
  //

  //     This is the loop skeleton where:
  //    jentry is the global entry number in the chain
  //    ientry is the entry number in the current Tree
  //  Note that the argument to GetEntry must be:
  //    jentry for TChain::GetEntry
  //    ientry for TTree::GetEntry and TBranch::GetEntry
  //
  //       To read only selected branches, Insert statements like:
  // METHOD1:
  //    fChain->SetBranchStatus("*",0);  // disable all branches
  //    fChain->SetBranchStatus("branchname",1);  // activate branchname
  // METHOD2: replace line
  //    fChain->GetEntry(jentry);       //read all branches
  //by  b_branchname->GetEntry(ientry); //read only this branch
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntriesFast();
  std::cout<<"total entries" <<nentries<<std::endl;
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    std::cout<< ientry<<std::endl;
    // if (Cut(ientry) < 0) continue;
  }
}

void TrackFlatAna::execute() {
  // print the message
  // std::cout << "Hello world!!!" << std::endl;

  Double_t a=1;
  std::cout << "Hello world!!!" << "  "<<a<<std::endl;
  // TH1F *h=new TH1F("h","",0,10);
}

Int_t TrackFlatAna::Cut(Long64_t entry)
{
  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;
}
Int_t TrackFlatAna::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}
Long64_t TrackFlatAna::LoadTree(Long64_t entry)
{
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;
}
void TrackFlatAna::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);
//  fChain->SetBranchAddress("run", &run, &b_run);
  fChain->SetBranchAddress("fTPCsignal", &fTPCsignal, &b_fTPCsignal);
  //fChain->SetBranchAddress("esdTrack.fITSsignal", &fITSsignal, &b_fITSsignal);
  //  fChain->SetBranchAddress("esdTrack.fTRDsignal", &fTRDsignal, &b_fTRDsignal);
  fChain->SetBranchAddress("trackP", &trackP, &b_trackP);
  fChain->SetBranchAddress("tgl", &tgl, &b_tgl);
  fChain->SetBranchAddress("trackEta", &trackEta, &b_trackEta);
  fChain->SetBranchAddress("tpc_cls", &tpc_cls, &b_tpc_cls);
  fChain->SetBranchAddress("esdTrack.fTPCsignalN", &fTPCsignalN, &b_fTPCsignalN);
  fChain->SetBranchAddress("ntracks", &ntracks, &b_ntracks);
  //  fChain->SetBranchAddress("trackstatus", &trackstatus, &b_trackstatus);
  fChain->SetBranchAddress("track_hasTOF", &track_hasTOF, &b_track_hasTOF);
  fChain->SetBranchAddress("track_tpcNsigma_el", &track_tpcNsigma_el, &b_track_tpcNsigma_el);
  fChain->SetBranchAddress("track_tpcNsigma_pi", &track_tpcNsigma_pi, &b_track_tpcNsigma_pi);
  fChain->SetBranchAddress("track_tpcNsigma_pro", &track_tpcNsigma_pro, &b_track_tpcNsigma_pro);
  fChain->SetBranchAddress("track_tpcNsigma_ka", &track_tpcNsigma_ka, &b_track_tpcNsigma_ka);
  fChain->SetBranchAddress("track_tpcNsigma_deut", &track_tpcNsigma_deut, &b_track_tpcNsigma_deut);
  fChain->SetBranchAddress("track_tpcNsigma_tri", &track_tpcNsigma_tri, &b_track_tpcNsigma_tri);
  fChain->SetBranchAddress("track_tpcNsigma_He3", &track_tpcNsigma_He3, &b_track_tpcNsigma_He3);

  fChain->SetBranchAddress("tracktofNsigmaElectron", &tracktofNsigmaElectron, &b_tracktofNsigmaElectron);
  fChain->SetBranchAddress("tracktofNsigmaKaon", &tracktofNsigmaKaon, &b_tracktofNsigmaKaon);
  fChain->SetBranchAddress("tracktofNsigmaProton", &tracktofNsigmaProton, &b_tracktofNsigmaProton);
  fChain->SetBranchAddress("tracktofNsigmaPion", &tracktofNsigmaPion, &b_tracktofNsigmaPion);
  fChain->SetBranchAddress("tracktofNsigmaDeuteron", &tracktofNsigmaDeuteron, &b_tracktofNsigmaDeuteron);
  fChain->SetBranchAddress("tracktofNsigmaTrition", &tracktofNsigmaTrition, &b_tracktofNsigmaTrition);
  fChain->SetBranchAddress("tracktofNsigmaHelium3", &tracktofNsigmaHelium3, &b_tracktofNsigmaHelium3);

  fChain->SetBranchAddress("track_ExpectedTPCSignalV0_el", &track_ExpectedTPCSignalV0_el, &b_track_ExpectedTPCSignalV0_el);
  fChain->SetBranchAddress("track_ExpectedTPCSignalV0_pi", &track_ExpectedTPCSignalV0_pi, &b_track_ExpectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track_ExpectedTPCSignalV0_pro", &track_ExpectedTPCSignalV0_pro, &b_track_ExpectedTPCSignalV0_pro);
  //fChain->SetBranchAddress("track_ExpectedTPCSignalV0_ka", &track_ExpectedTPCSignalV0_ka, &b_track_ExpectedTPCSignalV0_ka);
  //fChain->SetBranchAddress("track_ExpectedTPCSignalV0_deut", &track_ExpectedTPCSignalV0_deut, &b_track_ExpectedTPCSignalV0_deut);

  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_el", &track_CorrectedTPCSignalV0_el, &b_track_CorrectedTPCSignalV0_el);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_pi", &track_CorrectedTPCSignalV0_pi, &b_track_CorrectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_pro", &track_CorrectedTPCSignalV0_pro, &b_track_CorrectedTPCSignalV0_pro);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_ka", &track_CorrectedTPCSignalV0_ka, &b_track_CorrectedTPCSignalV0_ka);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_deut", &track_CorrectedTPCSignalV0_deut, &b_track_CorrectedTPCSignalV0_deut);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_tri", &track_CorrectedTPCSignalV0_tri, &b_track_CorrectedTPCSignalV0_tri);
  fChain->SetBranchAddress("track_CorrectedTPCSignalV0_He3", &track_CorrectedTPCSignalV0_He3, &b_track_CorrectedTPCSignalV0_He3);

  fChain->SetBranchAddress("track_GetPileupValue", &track_GetPileupValue, &b_track_GetPileupValue);
  fChain->SetBranchAddress("dca_r", &dca_r, &b_dca_r);
  fChain->SetBranchAddress("dca_z", &dca_z, &b_dca_z);
  fChain->SetBranchAddress("nCrossRows", &nCrossRows, &b_nCrossRows);

//  fChain->SetBranchAddress("run", &run, &b_run);
//  fChain->SetBranchAddress("intrate", &intrate, &b_intrate);
//  fChain->SetBranchAddress("timeStampS", &timeStampS, &b_timeStampS);
//  fChain->SetBranchAddress("timestamp", &timestamp, &b_timestamp);
//  fChain->SetBranchAddress("bField", &bField, &b_bField);
//  fChain->SetBranchAddress("triggerMask", &triggerMask, &b_triggerMask);
//  fChain->SetBranchAddress("gid", &gid, &b_gid);
//  fChain->SetBranchAddress("shiftA", &shiftA, &b_shiftA);
//  fChain->SetBranchAddress("shiftC", &shiftC, &b_shiftC);
//  fChain->SetBranchAddress("shiftM", &shiftM, &b_shiftM);
//  fChain->SetBranchAddress("nPileUpPrim", &nPileUpPrim, &b_nPileUpPrim);
//  fChain->SetBranchAddress("nPileUpSum", &nPileUpSum, &b_nPileUpSum);
//  fChain->SetBranchAddress("primMult", &primMult, &b_primMult);
//  fChain->SetBranchAddress("tpcClusterMult", &tpcClusterMult, &b_tpcClusterMult);
//  fChain->SetBranchAddress("pileUpOK", &pileUpOK, &b_pileUpOK);
//  fChain->SetBranchAddress("multSSD", &multSSD, &b_multSSD);
//  fChain->SetBranchAddress("multSDD", &multSDD, &b_multSDD);
//  fChain->SetBranchAddress("multSPD", &multSPD, &b_multSPD);
//  fChain->SetBranchAddress("multV0", &multV0, &b_multV0);
//  fChain->SetBranchAddress("multT0", &multT0, &b_multT0);
//  fChain->SetBranchAddress("spdvz", &spdvz, &b_spdvz);
//  fChain->SetBranchAddress("itsTracklets", &itsTracklets, &b_itsTracklets);
//  fChain->SetBranchAddress("tpcMult", &tpcMult, &b_tpcMult);
  fChain->SetBranchAddress("tpcTrackBeforeClean", &tpcTrackBeforeClean, &b_tpcTrackBeforeClean);
  fChain->SetBranchAddress("centV0", &centV0, &b_centV0);
//  fChain->SetBranchAddress("centITS0", &centITS0, &b_centITS0);
//  fChain->SetBranchAddress("centITS1", &centITS1, &b_centITS1);
  fChain->SetBranchAddress("ITSRefit", &ITSRefit, &b_ITSRefit);
  fChain->SetBranchAddress("TPCRefit", &TPCRefit, &b_TPCRefit);
  fChain->SetBranchAddress("Nucleitrigger_OFF", &Nucleitrigger_OFF, &b_Nucleitrigger_OFF);
  fChain->SetBranchAddress("isPileUp",&isPileUp,&b_isPileUp);
//  fChain->SetBranchAddress("isMinBias", &isMinBias, &b_isMinBias);

  Notify();
}
Bool_t TrackFlatAna::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

void TrackFlatAna::Show(Long64_t entry)
{
  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);
}

