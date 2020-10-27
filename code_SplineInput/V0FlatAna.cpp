
#include "V0FlatAna.h"
#include <iostream>

ClassImp(V0FlatAna);

V0FlatAna::V0FlatAna() {
  // nothing to do here. The class has no member data so
  // nothing to be initialized
}

V0FlatAna::~V0FlatAna() {
  // nothing to do here. The class has no member data so
  // nothing to be initialized
}

void V0FlatAna::Loop()
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

void V0FlatAna::execute() {
  // print the message
  // std::cout << "Hello world!!!" << std::endl;

  Double_t a=1;
  std::cout << "Hello world!!!" << "  "<<a<<std::endl;
  // TH1F *h=new TH1F("h","",0,10);
}

Int_t V0FlatAna::Cut(Long64_t entry)
{
  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;
}
Int_t V0FlatAna::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}
Long64_t V0FlatAna::LoadTree(Long64_t entry)
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
void V0FlatAna::Init(TTree *tree)
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
  //  fChain->SetBranchAddress("v0.fPointAngle", &fPointAngle, &b_fPointAngle);
  //  fChain->SetBranchAddress("kf.fChi2", &fChi2, &b_fChi2);
  fChain->SetBranchAddress("K0Like", &K0Like, &b_K0Like);
  fChain->SetBranchAddress("ELike", &ELike, &b_ELike);
  fChain->SetBranchAddress("LLike", &LLike, &b_LLike);
  fChain->SetBranchAddress("ALLike", &ALLike, &b_ALLike);
  fChain->SetBranchAddress("cleanK0", &cleanK0, &b_cleanK0);
  fChain->SetBranchAddress("cleanGamma", &cleanGamma, &b_cleanGamma);
  fChain->SetBranchAddress("cleanLambda", &cleanLambda, &b_cleanLambda);
  fChain->SetBranchAddress("cleanALambda", &cleanALambda, &b_cleanALambda);
//  fChain->SetBranchAddress("type", &type, &b_type);
//  fChain->SetBranchAddress("centV0", &centV0, &b_centV0);
//  fChain->SetBranchAddress("centITS0", &centITS0, &b_centITS0);
//  fChain->SetBranchAddress("centITS1", &centITS1, &b_centITS1);
//  fChain->SetBranchAddress("tpcClusterMult", &tpcClusterMult, &b_tpcClusterMult);
//  fChain->SetBranchAddress("multSSD", &multSSD, &b_multSSD);
//  fChain->SetBranchAddress("multSDD", &multSDD, &b_multSDD);
  fChain->SetBranchAddress("tpcTrackBeforeClean", &tpcTrackBeforeClean, &b_tpcTrackBeforeClean);
//  fChain->SetBranchAddress("triggerMask", &triggerMask, &b_triggerMask);
//  fChain->SetBranchAddress("isMinBias", &isMinBias, &b_isMinBias);
  fChain->SetBranchAddress("isPileUp",&isPileUp,&b_isPileUp);

  //  fChain->SetBranchAddress("track0status", &track0status, &b_track0status);
  fChain->SetBranchAddress("track0_hasTOF", &track0_hasTOF, &b_track0_hasTOF);
  fChain->SetBranchAddress("track0.fTPCsignal", &track0_fTPCsignal, &b_track0_fTPCsignal);
  fChain->SetBranchAddress("track0.fTPCsignalN", &track0_fTPCsignalN, &b_track0_fTPCsignalN);
  fChain->SetBranchAddress("track0P", &track0P, &b_track0P);
  fChain->SetBranchAddress("track0Eta", &track0Eta, &b_track0Eta);
  fChain->SetBranchAddress("track0Tgl", &track0Tgl, &b_track0Tgl);
//  fChain->SetBranchAddress("track0Phi", &track0Phi, &b_track0Phi);
//  fChain->SetBranchAddress("track0Px", &track0Px, &b_track0Px);
//  fChain->SetBranchAddress("track0Py", &track0Py, &b_track0Py);
//  fChain->SetBranchAddress("track0Pz", &track0Pz, &b_track0Pz);
  fChain->SetBranchAddress("track0tofNsigmaElectron", &track0tofNsigmaElectron, &b_track0tofNsigmaElectron);
  fChain->SetBranchAddress("track0tofNsigmaKaon", &track0tofNsigmaKaon, &b_track0tofNsigmaKaon);
  fChain->SetBranchAddress("track0tofNsigmaProton", &track0tofNsigmaProton, &b_track0tofNsigmaProton);
  fChain->SetBranchAddress("track0tofNsigmaKaon", &track0tofNsigmaKaon, &b_track0tofNsigmaKaon);
  fChain->SetBranchAddress("track0tpcNsigma_el", &track0tpcNsigma_el, &b_track0tpcNsigma_el);
  fChain->SetBranchAddress("track0tpcNsigma_pro", &track0tpcNsigma_pro, &b_track0tpcNsigma_pro);
  fChain->SetBranchAddress("track0tpcNsigma_ka", &track0tpcNsigma_ka, &b_track0tpcNsigma_ka);
  fChain->SetBranchAddress("track0tpcNsigma_pi", &track0tpcNsigma_pi, &b_track0tpcNsigma_pi);
  fChain->SetBranchAddress("track0ExpectedTPCSignalV0_el", &track0ExpectedTPCSignalV0_el, &b_track0ExpectedTPCSignalV0_el);
  fChain->SetBranchAddress("track0ExpectedTPCSignalV0_pro", &track0ExpectedTPCSignalV0_pro, &b_track0ExpectedTPCSignalV0_pro);
  //fChain->SetBranchAddress("track0ExpectedTPCSignalV0_ka", &track0ExpectedTPCSignalV0_ka, &b_track0ExpectedTPCSignalV0_ka);
  //fChain->SetBranchAddress("track0ExpectedTPCSignalV0_pi", &track0ExpectedTPCSignalV0_pi, &b_track0ExpectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track0CorrectedTPCSignalV0_el", &track0CorrectedTPCSignalV0_el, &b_track0CorrectedTPCSignalV0_el);
  fChain->SetBranchAddress("track0CorrectedTPCSignalV0_pro", &track0CorrectedTPCSignalV0_pro, &b_track0CorrectedTPCSignalV0_pro);
  fChain->SetBranchAddress("track0CorrectedTPCSignalV0_ka", &track0CorrectedTPCSignalV0_ka, &b_track0CorrectedTPCSignalV0_ka);
  fChain->SetBranchAddress("track0CorrectedTPCSignalV0_pi", &track0CorrectedTPCSignalV0_pi, &b_track0CorrectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track0GetPileupValue", &track0GetPileupValue, &b_track0GetPileupValue);
  fChain->SetBranchAddress("tpc_cls0", &tpc_cls0, &b_tpc_cls0);
  fChain->SetBranchAddress("dca0_r", &dca0_r, &b_dca0_r);
  fChain->SetBranchAddress("dca0_z", &dca0_z, &b_dca0_z);
  fChain->SetBranchAddress("nCrossRows0", &nCrossRows0, &b_nCrossRows0);

  //  fChain->SetBranchAddress("track1status", &track1status, &b_track1status);
  fChain->SetBranchAddress("track1_hasTOF", &track1_hasTOF, &b_track1_hasTOF);
  fChain->SetBranchAddress("track1.fTPCsignal", &track1_fTPCsignal, &b_track1_fTPCsignal);
  fChain->SetBranchAddress("track1.fTPCsignalN", &track1_fTPCsignalN, &b_track1_fTPCsignalN);
  fChain->SetBranchAddress("track1P", &track1P, &b_track1P);
  fChain->SetBranchAddress("track1Eta", &track1Eta, &b_track1Eta);
  fChain->SetBranchAddress("track1Tgl", &track1Tgl, &b_track1Tgl);
//  fChain->SetBranchAddress("track1Phi", &track1Phi, &b_track1Phi);
//  fChain->SetBranchAddress("track1Px", &track1Px, &b_track1Px);
//  fChain->SetBranchAddress("track1Py", &track1Py, &b_track1Py);
//  fChain->SetBranchAddress("track1Pz", &track1Pz, &b_track1Pz);
  fChain->SetBranchAddress("track1tofNsigmaElectron", &track1tofNsigmaElectron, &b_track1tofNsigmaElectron);
  fChain->SetBranchAddress("track1tofNsigmaKaon", &track1tofNsigmaKaon, &b_track1tofNsigmaKaon);
  fChain->SetBranchAddress("track1tofNsigmaProton", &track1tofNsigmaProton, &b_track1tofNsigmaProton);
  fChain->SetBranchAddress("track1tofNsigmaKaon", &track1tofNsigmaKaon, &b_track1tofNsigmaKaon);
  fChain->SetBranchAddress("track1tpcNsigma_el", &track1tpcNsigma_el, &b_track1tpcNsigma_el);
  fChain->SetBranchAddress("track1tpcNsigma_pro", &track1tpcNsigma_pro, &b_track1tpcNsigma_pro);
  fChain->SetBranchAddress("track1tpcNsigma_ka", &track1tpcNsigma_ka, &b_track1tpcNsigma_ka);
  fChain->SetBranchAddress("track1tpcNsigma_pi", &track1tpcNsigma_pi, &b_track1tpcNsigma_pi);
  fChain->SetBranchAddress("track1ExpectedTPCSignalV0_el", &track1ExpectedTPCSignalV0_el, &b_track1ExpectedTPCSignalV0_el);
  fChain->SetBranchAddress("track1ExpectedTPCSignalV0_pro", &track1ExpectedTPCSignalV0_pro, &b_track1ExpectedTPCSignalV0_pro);
  //fChain->SetBranchAddress("track1ExpectedTPCSignalV0_ka", &track1ExpectedTPCSignalV0_ka, &b_track1ExpectedTPCSignalV0_ka);
  //fChain->SetBranchAddress("track1ExpectedTPCSignalV0_pi", &track1ExpectedTPCSignalV0_pi, &b_track1ExpectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track1CorrectedTPCSignalV0_el", &track1CorrectedTPCSignalV0_el, &b_track1CorrectedTPCSignalV0_el);
  fChain->SetBranchAddress("track1CorrectedTPCSignalV0_pro", &track1CorrectedTPCSignalV0_pro, &b_track1CorrectedTPCSignalV0_pro);
  fChain->SetBranchAddress("track1CorrectedTPCSignalV0_ka", &track1CorrectedTPCSignalV0_ka, &b_track1CorrectedTPCSignalV0_ka);
  fChain->SetBranchAddress("track1CorrectedTPCSignalV0_pi", &track1CorrectedTPCSignalV0_pi, &b_track1CorrectedTPCSignalV0_pi);
  fChain->SetBranchAddress("track1GetPileupValue", &track1GetPileupValue, &b_track1GetPileupValue);
  fChain->SetBranchAddress("tpc_cls1", &tpc_cls1, &b_tpc_cls1);                                     
  fChain->SetBranchAddress("dca1_r", &dca1_r, &b_dca1_r);                                           
  fChain->SetBranchAddress("dca1_z", &dca1_z, &b_dca1_z);                                           
  fChain->SetBranchAddress("nCrossRows1", &nCrossRows1, &b_nCrossRows1); 

  Notify();
}
Bool_t V0FlatAna::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

void V0FlatAna::Show(Long64_t entry)
{
  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);
}

