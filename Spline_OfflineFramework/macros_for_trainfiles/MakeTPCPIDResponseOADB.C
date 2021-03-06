#include "TSpline.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TFile.h"
#include "TError.h"
#include "TROOT.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TH1I.h"
#include "TDirectoryFile.h"
#include "TPRegexp.h"
#include "TClass.h"

#include "AliNDLocalRegression.h"
#include "AliOADBContainer.h"
#include "AliTPCPIDResponse.h"
#include "AliPID.h"
#include "TSystem.h"
#include "./GetMultiplicityFactors.C"


AliOADBContainer fContainer("TPCSplines");
Int_t fFailures=0;

Bool_t AddOADBObjectFromSplineFile(const TString fileName,
                                   const Int_t firstRun, const Int_t lastRun,
                                   const Int_t pass,
                                   const TString dEdxType="",
                                   const TString multCorr="",
                                   const TString resolution="",
                                   const TString pileupDefinition="",
                                   const AliTPCPIDResponse::EMultiplicityEstimator multEstimator = AliTPCPIDResponse::kNumberOfESDTracks);

Bool_t CheckMultiplicityCorrection(const TString& corrections);
TObjArray* SetupSplineArrayFromFile(const TString fileName);

//______________________________________________________________________________
void MakeTPCPIDResponseOADB(TString outfile="$ALICE_PHYSICS/OADB/COMMON/PID/data/TPCPIDResponseOADB.root", TString period = "", TString fileName = "", Int_t firstRun = 0, Int_t lastRun = 0, Int_t pass = 0, TString multParameters = "", TString multBinString = "", TString pileupDefinition="", AliTPCPIDResponse::EMultiplicityEstimator multEstimator=AliTPCPIDResponse::kNTPCTrackBeforeClean )
{

  TString dEdxType = "";
  TString multCorr = "";

  printf("Splines file Name: %s\n",fileName.Data());
  printf("First Run Number: %i\nLast Run Number: %i\nPass: %i\n",firstRun, lastRun,pass);
  printf("Multiplicity Correction String: %s\n",multCorr.Data());

  if (multParameters.Contains(".root"))
    multCorr = GetMultiplicityFactors(multParameters);
  else
    multCorr = multParameters;

AddOADBObjectFromSplineFile(fileName,firstRun,lastRun,pass,dEdxType,multCorr, "", pileupDefinition, multEstimator); // 18q-r 18r range: 296631, 999999, "1"); // LHC18r


/*
  // ---| local test |----------------------------------------------------------
  AddOADBObjectFromSplineFile("/data/Work/data/PID/testTransferFunction/10e/splines_10d.pass4.root", 194480, 195874, "1", "",
                              "-5.906e-06,-5.064e-04,-3.521e-02,2.469e-02,0 ; -5.32e-06, 1.177e-05, -0.5 ; 0.,0.,0.,0.");
*/

  if (fFailures) {
    Error("MakeSplineOADB","Process ended with %d fFailures. Not writing the container", fFailures);
    return;
  }

  fContainer.WriteToFile(outfile.Data());
}

//______________________________________________________________________________
TObjArray* SetupSplineArrayFromFile(const TString fileName)
{
  TFile f(fileName);
  if (!f.IsOpen() || f.IsZombie()) {
    Error("AddOADBObject","Could not open file '%s'",fileName.Data());
    return 0x0;
  }
  gROOT->cd();

  // ===| Spline Array |========================================================
  // A sorted array with the spline objects following the particle nameing convention
  //   in AliPID::EParticleType up to AliPID::kSPECIESC
  //   usually only e, pi, K, p will be available, for the muons the pion spline will be used
  //   for the light nuclei the proton spline will be used. This needs to be set up
  //   in the loading of the splines on order to save space in the file
  // At the position AliPID::kSPECIESC the spline without low momentum corrections is placed

  TObjArray *arr=new TObjArray(AliPID::kSPECIESC+1);
  arr->SetName("Splines");
  arr->SetOwner();

  TIter nextKey(f.GetListOfKeys());

  // ---| loop over all splines in the file |-----------------------------------
  TObject *o=0x0;
  while ( (o=nextKey()) ) {
    TSpline3 *sp=dynamic_cast<TSpline3*>(f.Get(o->GetName()));

    if (!sp) continue;
    const TString splineName(sp->GetName());

    // ---| check for default spline |------------------------------------------
    if (splineName.Contains("ALL")) {
      arr->AddAt(sp, AliPID::kSPECIESC);
      continue;
    }

    // ---| check for particles |------------------------------------------------
    TString partName;
    for (Int_t ispec=0; ispec<Int_t(AliPID::kSPECIESC); ++ispec) {
      partName=AliPID::ParticleName(ispec);
      partName.ToUpper();

      if (splineName.Contains(partName)) {
        arr->AddAt(sp, ispec);
        break;
      }
    }
  }

  return arr;
}

//______________________________________________________________________________
Bool_t AddOADBObjectFromSplineFile(const TString fileName,
                                   const Int_t firstRun, const Int_t lastRun,
                                   const Int_t pass,
                                   const TString dEdxType,
                                   const TString multCorr,
                                   const TString resolution,
                                   const TString pileupDefinition,
                                   const AliTPCPIDResponse::EMultiplicityEstimator multEstimator)
{

  // ---| Master array for TPC PID response |-----------------------------------
  TObjArray *arrTPCPIDResponse = new TObjArray;
  arrTPCPIDResponse->SetName("TPCPIDResponse");
  arrTPCPIDResponse->SetOwner();

  // ---| Period and pass name deduced from the file name |---------------------
  //TPRegexp regPeriod(".*/((LHC|)[0-9]{2}[a-z].pass[0-9_a-zA-Z]*)/.*");
  //TObjArray* arrPeriod = regPeriod.MatchS(fileName);
  //cout << arrPeriod->GetEntriesFast() << endl;
  //if (arrPeriod && arrPeriod->GetEntriesFast() == 3) {
  //  TString periodName = arrPeriod->At(1)->GetName();
  //  if (! periodName.BeginsWith("LHC") ) periodName.Prepend("LHC");
  //  printf("period name: %s\n", periodName.Data());
  //  arrTPCPIDResponse->SetName(periodName);
  //}
  //else {
  //  printf("Cannot extract period and pass from file name %s\n", fileName.Data());
  //}

  //delete arrPeriod;
  
  // ---| Splines |-------------------------------------------------------------
  TObjArray *arrSplines = SetupSplineArrayFromFile(fileName);
  if (!arrSplines) {
    ++fFailures;
    return kFALSE;
  }
  arrTPCPIDResponse->Add(arrSplines);

  // ---| PID config |----------------------------------------------------------
  if (!dEdxType.IsNull()) {
    TNamed *pidConfig = new TNamed("dEdxType",dEdxType.Data());
    arrTPCPIDResponse->Add(pidConfig);
  }

  // ---| multiplicity correction |---------------------------------------------
  TObjArray *arrMultiplicityCorrection=0x0;
  if (!multCorr.IsNull()) {
    if ( (arrMultiplicityCorrection=AliTPCPIDResponse::GetMultiplicityCorrectionArrayFromString(multCorr)) ) {
      TNamed *multCorrConfig = new TNamed("MultiplicityCorrection",multCorr.Data());
      arrTPCPIDResponse->Add(multCorrConfig);
      delete arrMultiplicityCorrection;
    } else {
      ++fFailures;
    }
  }

  // ---| resolution parametrisation |------------------------------------------
  if (!resolution.IsNull()) {
    TNamed *resolutionParam = new TNamed("dEdxResolution", resolution.Data());
    arrTPCPIDResponse->Add(resolutionParam);
  }

  // ---| Pileup correction |---------------------------------------------------
  if (!pileupDefinition.IsNull()) {
    if (pileupDefinition.BeginsWith("fileName:")) {
      // if the string begins with 'fileName:' add the fileName to the array
      TString fileName(pileupDefinition);
      fileName.ReplaceAll("fileName:", "");
      TNamed *pileupCorrectionFile = new TNamed("PileupCorrectionFile", fileName.Data());
      arrTPCPIDResponse->Add(pileupCorrectionFile);
    } else {
      // else directly load the object from file and add it
      AliNDLocalRegression* pileupCorrection = AliTPCPIDResponse::GetPileupCorrectionFromFile(pileupDefinition);
      arrTPCPIDResponse->Add(pileupCorrection);
    }
  }

  TString pass_name; 
  pass_name.Form("%d",pass);
  fContainer.AppendObject(arrTPCPIDResponse, firstRun, lastRun, pass_name);

  // ---| multiplicity estimator |---
  if (multEstimator != AliTPCPIDResponse::kNumberOfESDTracks) {
    TNamed *multEstimatorDef = new TNamed("MultiplicityEstimator", Form("%d", (Int_t)multEstimator));
    arrTPCPIDResponse->Add(multEstimatorDef);
  }

  return kTRUE;
}

//______________________________________________________________________________
Bool_t CheckMultiplicityCorrection(const TString& corrections)
{
  // the corrections string is supposed to be in the format
  // parMC_0,parMC_1,parMC_2,parMC_3, parMC_4; parMCTT_0,parMCTT_1,parMCTT_2; parMSC_0,parMSC_1,parMSC_2, parMSC_3
  // where parMC are the parameters for AliTPCPIDResponse::SetParameterMultiplicityCorrection
  // parMCTT are the parameters for AliTPCPIDResponse::SetParameterMultiplicityCorrectionTanTheta
  // parMSC are the parameters for AliTPCPIDResponse::SetParameterMultiplicitySigmaCorrection

  const Int_t nSets=3;
  const Int_t nPars[nSets]={5,3,4};

  TObjArray *arrCorrectionSets = corrections.Tokenize(";");
  if (arrCorrectionSets->GetEntriesFast() != nSets){
    Error("CheckMultiplicityCorrection","Number of parameter sets not equal to %d. Please read documentation of this function", nSets);
    ++fFailures;
    delete arrCorrectionSets;
    return kFALSE;
  }

  for (Int_t iset=0 ;iset<nSets; ++iset) {
    TObjArray *arrTmp=((TObjString*)arrCorrectionSets->At(iset))->String().Tokenize(",");
    if (arrTmp->GetEntriesFast() != nPars[iset]){
      Error("CheckMultiplicityCorrection","Number of parameters in set %d not equal to %d. Please read documentation of this function", iset, nPars[iset]);
      ++fFailures;
      delete arrCorrectionSets;
      delete arrTmp;
      return kFALSE;
    }

  }

  delete arrCorrectionSets;
  return kTRUE;
}
