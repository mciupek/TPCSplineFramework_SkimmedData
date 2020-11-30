#include "TTree.h"
#include "TH2D.h"
#include "TList.h"
#include "TString.h"
#include "TFile.h"
#include "TGraph.h"
#include "TMath.h"
#include "TDatime.h"
#include "TSpline.h"
#include "TF1.h"
#include "TArrayI.h"
#include "TObjString.h"

#include "TSystem.h"

#include "AliTPCPIDResponse.h"

#include "AliPID.h"
#include "AliESDtrack.h"

#include <iostream>

#include "ProgressBar.h"

TSpline3* getSplineFromFile(TString fileName, TString splineName);

Bool_t SetMultiplicityCorrectionFromString(const TString& multiplicityData, TF1* corrFuncMult, TF1* seccorrFuncMult, TF1* corrFuncSigmaMult);
TObjArray* GetMultiplicityCorrectionArrayFromString(const TString& corrections);

Double_t getEtaCorrection(TH2D* hMap, Double_t tanTheta, Double_t dEdx)
{
  if (!hMap)
    return 1.;

  if (dEdx < 1.)
    return 1.;


  //TODO WARNING: Here we can use tanTheta directly but in data we have to extract tanTheta from thetaGlobal!!!

  /*
  // For ESD tracks, the local tanTheta could be used (esdTrack->GetInnerParam()->GetTgl()).
  // However, this value is not available for AODs and, thus, not for AliVTrack.
  // Fortunately, the following formula allows to approximate the local tanTheta with the
  // global theta angle -> This is for by far most of the tracks the same, but gives at
  // maybe the percent level differences within +- 0.2 in tanTheta -> Which is still ok.
  Double_t tanThetaLocal = TMath::Tan(-thetaGlobal + TMath::Pi() / 2.0);
  */
  Int_t binX = hMap->GetXaxis()->FindBin(tanTheta);
  Int_t binY = hMap->GetYaxis()->FindBin(1. / dEdx);

  if (binX == 0)
    binX = 1;
  if (binX > hMap->GetXaxis()->GetNbins())
    binX = hMap->GetXaxis()->GetNbins();

  if (binY == 0)
    binY = 1;
  if (binY > hMap->GetYaxis()->GetNbins())
    binY = hMap->GetYaxis()->GetNbins();

  return hMap->GetBinContent(binX, binY);
}

Int_t correctShapeEtaTree(Bool_t correctData, TString pathMap, TString fileNameMap, TString mapSuffix,
                          Bool_t recalculateExpecteddEdx, TString pathNameSplinesFile, TString prSplinesName,
                          TString pathTree,
                          Bool_t hasMultiplicity,
                          Bool_t correctMult,
                          TString fileNameTree = "bhess_PIDetaTree.root", TString treeName = "fTree", TString multCorrFactor = "")
{
  const Double_t massProton = AliPID::ParticleMass(AliPID::kProton);

  TString pathNameSplines = pathNameSplinesFile;
  pathNameSplines = gSystem->DirName(pathNameSplines.Data());

  if (!correctData && !recalculateExpecteddEdx) {
    std::cout << "Nothing to be done: Correction AND recalculation of expected dEdx are disabled!" << std::endl;
    return -1;
  }
  // Extract the splines, if desired
  TSpline3* splPr = 0x0;
  AliTPCPIDResponse* resp = 0x0;

  if (recalculateExpecteddEdx) {
    std::cout << "Loading splines to recalculate expected dEdx!" << std::endl << std::endl;
    if (pathNameSplinesFile.Contains("OADB")) {
      resp = new AliTPCPIDResponse();
      resp->InitFromOADB(1,0,"0",pathNameSplinesFile.Data());
    }
    else {
      splPr = getSplineFromFile(pathNameSplinesFile, prSplinesName);
    }
  }
  else
    std::cout << "Taking dEdxExpected from Tree..." << std::endl << std::endl;

  if (correctMult)
    std::cout << "Correcting multiplicity dependence..." << std::endl << std::endl;
  else
    std::cout << "NOT correcting multiplicity dependence..." << std::endl << std::endl;

  // Extract the correction map, if desired
  TH2D* hMap = 0x0;
  if (correctData) {
    std::cout << "Loading map to correct dEdx (NOT dEdx_expected in this special case!)!" << std::endl << std::endl;

    TFile* fMap = TFile::Open(Form("%s/%s", pathMap.Data(), fileNameMap.Data()));
    if (!fMap)  {
      std::cout << "Failed to open file \"" << Form("%s/%s", pathMap.Data(), fileNameMap.Data()) << "\"!" << std::endl;
      return -1;
    }

    hMap = dynamic_cast<TH2D*>(fMap->Get(Form("hRefined%s", mapSuffix.Data())));
    if (!hMap) {
      std::cout << "Failed to load correction map!" << std::endl;
      return -1;
    }
  }
  else
    std::cout << "NOT correcting eta dependence..." << std::endl << std::endl;

  // Extract the data Tree
  TFile* fTree = TFile::Open(Form("%s/%s", pathTree.Data(), fileNameTree.Data()));
  if (!fTree)  {
    std::cout << "Failed to open file \"" << Form("%s/%s", pathTree.Data(), fileNameTree.Data()) << "\"!" << std::endl;
    return -1;
  }

  TTree* tree = dynamic_cast<TTree*>(fTree->Get(treeName.Data()));
  if (!tree) {
    std::cout << "Failed to load data tree!" << std::endl << std::endl;
    return -1;
  }

  Long64_t nTreeEntries = tree->GetEntriesFast();

  Double_t pTPC = 0.; // Momentum at TPC inner wall
  //Double_t pT = 0.;
  Double_t dEdx = 0.; // Measured dE/dx
  Double_t dEdxExpected = 0.; // Expected dE/dx according to parametrisation
  Double_t tanTheta = 0.; // Tangens of (local) theta at TPC inner wall
  //Double_t sinAlpha = 0.; // Sine of (local) phi at TPC inner wall
  //Double_t y = 0.; // Local Y
  Double_t phiPrime = 0; // Phi prime
  Double_t tpcSignalN = 0; // Number of TPC clusters for PID
  Double_t multiplicity = 0;
  Double_t  pidType = 0; // Type of identification (TPC dEdx, V0, ...)

  tree->SetBranchAddress("pTPC", &pTPC);
  //tree->SetBranchAddress("pT", &Pt);
  tree->SetBranchAddress("dEdx", &dEdx);
  if (!recalculateExpecteddEdx)
    tree->SetBranchAddress("dEdxExpected", &dEdxExpected);
  tree->SetBranchAddress("tanTheta", &tanTheta);
  //tree->SetBranchAddress("sinAlpha", &sinAlpha);
  //tree->SetBranchAddress("y", &y);
  //TODO not needed for the moment tree->SetBranchAddress("phiPrime", &phiPrime);
  tree->SetBranchAddress("tpcSignalN", &tpcSignalN);
  if (hasMultiplicity)
    tree->SetBranchAddress("fMultiplicity", &multiplicity);
  tree->SetBranchAddress("pidType", &pidType);

  // Output file
  TDatime daTime;
  TString savefileName = "";
  if (correctData) {
    savefileName = Form("%s_%sCorrectedWithMap_%s___%04d_%02d_%02d__%02d_%02d.root", fileNameTree.ReplaceAll(".root", "").Data(),
                        correctMult ? "multiplicityCorrected_" : "",
                        fileNameMap.ReplaceAll(".root", "").Data(), daTime.GetYear(),
                        daTime.GetMonth(), daTime.GetDay(), daTime.GetHour(), daTime.GetMinute());
  }
  else if (recalculateExpecteddEdx) {
    savefileName = Form("%s_%sNewSplines___%04d_%02d_%02d__%02d_%02d.root", fileNameTree.ReplaceAll(".root", "").Data(),
                        correctMult ? "multiplicityCorrected_" : "",
                        daTime.GetYear(), daTime.GetMonth(), daTime.GetDay(), daTime.GetHour(), daTime.GetMinute());
  }
  else
    return -1;


  TFile* fSave = TFile::Open(Form("%s/%s", pathNameSplines.Data(), savefileName.Data()), "recreate");
  if (!fSave) {
    std::cout << "Failed to open save file \"" << Form("%s/%s", pathTree.Data(), savefileName.Data()) << "\"!" << std::endl;
    return -1;
  }


  fSave->cd();
  TTree* treeCorrected = new TTree("fTree", "Tree for analysis of #eta dependence of TPC signal; #eta corrected");
  treeCorrected->Write(0, TObject::kOverwrite);

  treeCorrected->Branch("pTPC", &pTPC);
  //treeCorrected->Branch("pT", &Pt);
  treeCorrected->Branch("dEdx", &dEdx);
  treeCorrected->Branch("dEdxExpected", &dEdxExpected);
  treeCorrected->Branch("tpcSignalN", &tpcSignalN);

  treeCorrected->Branch("tanTheta", &tanTheta);
  //treeCorrected->Branch("sinAlpha", &sinAlpha);
  //treeCorrected->Branch("y", &y);
  treeCorrected->Branch("phiPrime", &phiPrime);
  if (hasMultiplicity)
    treeCorrected->Branch("fMultiplicity", &multiplicity);
  treeCorrected->Branch("pidType", &pidType);

  Double_t corrFactor = 1.0;

  //Explicitly calculating the mult correction w.o. use of AliTPCPIDResponse
  //Old Methode
//   TF1 corrFuncMult("corrFuncMult", "[0] + [1]*TMath::Max([4], TMath::Min(x, [3])) + [2] * TMath::Power(TMath::Max([4], TMath::Min(x, [3])), 2)", 0., 0.2);
  TF1 corrFuncMultTanTheta("corrFuncMultTanTheta", "[0] * (x -[2]) + [1] * (x * x - [2] * [2])", -1.5, 1.5);
  TF1 corrFuncSigmaMult("corrFuncSigmaMult", "TMath::Max(0, [0] + [1]*TMath::Min(x, [3]) + [2] * TMath::Power(TMath::Min(x, [3]), 2))", 0., 0.2);
  //New method
  TF1 corrFuncMult("corrFuncMult", "[0] + [1]*x", 0., 0.2);
  TF1 corrFuncMultCurv("corrFuncMultCurv", "[0] + [1]*x", 0., 0.2);

  AliTPCPIDResponse* tpcResponse = 0x0;

  if (correctMult) {
    //Use this normally
/*    tpcResponse = new AliTPCPIDResponse();
    tpcResponse->SetMultiplicityCorrectionFromString(multCorrFactor);  */

    //If it does not work with AliTPCPIDResponse, use this explicit method
    //Old Method
//     SetMultiplicityCorrectionFromString(multCorrFactor, corrFuncMult, corrFuncMultTanTheta, corrFuncSigmaMult);
    //New Method
    SetMultiplicityCorrectionFromString(multCorrFactor, &corrFuncMult, &corrFuncMultCurv, &corrFuncSigmaMult);

  }

  progressbar(0.);
  Int_t oldmultiplicity = 0;
  AliESDtrack* track = new AliESDtrack();
  Double_t dEdx_temp = 0.0;
  Int_t nPoints = 0;
  AliTPCPIDResponse::ETPCgainScenario gain = AliTPCPIDResponse::kDefault;
  Double_t covar[15] = {0.0};
  Double_t param[5] = {0.0};

  for (Long64_t i = 0; i < nTreeEntries; i++) {
    tree->GetEntry(i);

    if (recalculateExpecteddEdx) {
      //Double_t old = dEdxExpected;

      if (resp && oldmultiplicity != multiplicity) {
        resp->SetCurrentEventMultiplicity(multiplicity);
//         resp->ChooseSplineForMultiplicity();   //For Different mult splines, not used currently
        resp->ResponseFunctiondEdxN(track, AliPID::kProton, AliTPCPIDResponse::kdEdxDefault, dEdx_temp, nPoints, gain, &splPr);
        oldmultiplicity = multiplicity;
      }

      dEdxExpected = 50. * splPr->Eval(pTPC / massProton); //WARNING: What, if MIP is different from 50.? Seems not to be used (tested for pp, MC_pp, PbPb and MC_PbPb), but can in principle happen
      /*if (TMath::Abs(dEdxExpected - old) > 1e-4) {
       p *rintedSomething = kTRUE;
       printf("%.2f - ", dEdxExpected);
       printf("%.2f\n", old);
      }*/
    }

    if (correctData) {
      corrFactor = getEtaCorrection(hMap, tanTheta, dEdxExpected);

      if (corrFactor <= 0)  {
        printf("Error: Bad correction factor (%f)\n", corrFactor);
        printedSomething = kTRUE;
        continue;
      }

      // Correct the track dEdx and leave the expected dEdx as it is, when creating the sigma map!
      // NOTE: This is due to the method the maps are created. The track dEdx (not the expected one!)
      // is corrected to uniquely relate a momemtum bin with an expected dEdx, where the expected dEdx
      // equals the track dEdx for all eta (thanks to the correction and the re-fit of the splines).
      // Consequently, looking up the uncorrected expected dEdx at a given tanTheta yields the correct
      // sigma parameter!

      // In principle, during map creation, one could also calculate for each tanTheta-p bin pair the expected dEdx
      // and then fill the map at the corresponding dEdxExpected-tanTheta bin. Then, one would also need to correct
      // the dEdxExpected to create the sigma maps.

      // In summary, both correction methods are valid in this case, but one has to be consistent! For the different methods,
      // the maps will be slightly distorted, but overall give the same results.



      //Scale eta dependence -> Doesn't seem to work!!!
      //corrFactor = (corrFactor - 1.)*(2e-6*30*multiplicity + 1.0) + 1.0;

      dEdx /= corrFactor;
    }

    if (correctMult) {
      Double_t corrFactorMult = 1.0;
      if (tpcResponse) {
        param[3] = TMath::Tan(0.5*TMath::Pi() - TMath::ATan(tanTheta));
        AliExternalTrackParam* trackForTheta = new AliExternalTrackParam(0.0,0.0,param,covar);
        corrFactorMult = tpcResponse->GetMultiplicityCorrectionFast(trackForTheta, dEdxExpected, multiplicity);
        delete trackForTheta;
        trackForTheta = 0x0;
      }
      else {
        Double_t dEdxExpectedInv = 1. / (dEdxExpected * corrFactor);
        //Old multiplicity method
        // Multiplicity depends on pure dEdx. Therefore, correction factor depends indirectly on eta
        // => Use eta correction factor to scale dEdxExpected accordingly
//         Double_t relSlope = corrFuncMult.Eval(dEdxExpectedInv);
//
//         //Correct eta dependence of slope
//         relSlope += corrFuncMultTanTheta(tanTheta);
//
//         Double_t corrFactorMult += relSlope * multiplicity;

        //New multiplicity method
        Double_t relSlope = corrFuncMult.Eval(dEdxExpectedInv);
        Double_t relCurv = corrFuncMultCurv.Eval(dEdxExpectedInv);

        if (multiplicity <= -relSlope/(2*relCurv))
          corrFactorMult += relSlope * multiplicity + relCurv * multiplicity * multiplicity;
        else
          corrFactorMult -= 0.25 *relSlope * relSlope/relCurv;
      }
      dEdx /= corrFactorMult;
    }

    treeCorrected->Fill();

    if (i % 10000 == 0)
      progressbar(100. * (((Double_t)i) / nTreeEntries));
  }

  progressbar(100.);

  fSave->cd();
  treeCorrected->Write(0, TObject::kOverwrite);
  fSave->Close();

  return 0;
}

TSpline3* getSplineFromFile(TString fileName, TString splineName) {
  TSpline3* spline = 0x0;

  TFile* fSpl = TFile::Open(fileName.Data());
  if (!fSpl) {
    fSpl = TFile::Open((fileName + TString(".root")).Data());
    if (!fSpl) {
      std::cout << "Failed to open spline file \"" << fileName.Data() << "\"!" << std::endl;
      return 0x0;
    }
  }

  TObjArray* TPCPIDResponse = (TObjArray*)fSpl->Get("TPCPIDResponse");
  if (!TPCPIDResponse) {
    spline = (TSpline3*)fSpl->Get(splineName.Data());

    // If splines are in file directly, without TPCPIDResponse object, try to load them
    if (!spline) {
      std::cout << "Failed to load object array from spline file \"" << fileName.Data() << "\"!" << std::endl;
      return 0x0;
    }
  }
  else {
    spline = (TSpline3*)TPCPIDResponse->FindObject(splineName.Data());

    if (!spline) {
      std::cout << "Failed to load splines from file \"" << fileName.Data() << "\"!" << std::endl;
      return 0x0;
    }
  }

  return spline;
}

Bool_t SetMultiplicityCorrectionFromString(const TString& multiplicityData, TF1* corrFuncMult, TF1* seccorrFuncMult, TF1* corrFuncSigmaMult)
{
  const TObjArray *arrParameters=0x0;
  if (!(arrParameters=GetMultiplicityCorrectionArrayFromString(multiplicityData))) {
    return kFALSE;
  }

  const TObjArray *arrPar1 = static_cast<TObjArray*>(arrParameters->At(0));
  if (corrFuncMult) {
    for (Int_t ipar=0; ipar<arrPar1->GetEntriesFast(); ++ipar) {
      corrFuncMult->SetParameter(ipar, static_cast<TObjString*>(arrPar1->At(ipar))->String().Atof());
    }
  }

  const TObjArray *arrPar2 = static_cast<TObjArray*>(arrParameters->At(1));
  if (seccorrFuncMult) {
    for (Int_t ipar=0; ipar<arrPar2->GetEntriesFast(); ++ipar) {
        seccorrFuncMult->SetParameter(ipar, static_cast<TObjString*>(arrPar2->At(ipar))->String().Atof());
    }
  }

  const TObjArray *arrPar3 = static_cast<TObjArray*>(arrParameters->At(2));
  if (corrFuncSigmaMult) {
    for (Int_t ipar=0; ipar<arrPar3->GetEntriesFast(); ++ipar) {
      corrFuncSigmaMult->SetParameter(ipar, static_cast<TObjString*>(arrPar3->At(ipar))->String().Atof());
    }
  }

  delete arrParameters;
  return kTRUE;
}

//______________________________________________________________________________
TObjArray* GetMultiplicityCorrectionArrayFromString(const TString& corrections)
{
  // the corrections string is supposed to be in the format
  // parMC_0,parMC_1,parMC_2,parMC_3, parMC_4; parMCTT_0,parMCTT_1,parMCTT_2; parMSC_0,parMSC_1,parMSC_2, parMSC_3
  // where parMC are the parameters for AliTPCPIDResponse::SetParameterMultiplicityCorrection
  // parMCTT are the parameters for AliTPCPIDResponse::SetParameterMultiplicityCorrectionTanTheta
  // parMSC are the parameters for AliTPCPIDResponse::SetParameterMultiplicitySigmaCorrection

  //For the new PbPb parametrization it should be in the following format: parMC_0,parMC1;parMCC_0,parMCC_1;parMSC_0,parMSC_1,parMSC_2, parMSC_3 where parMC_i are the parameters for the Slope parametrization and parMCC_i the parameters for the curvature parametrization

  const Int_t nSets=3;
  const Int_t nPars_0[nSets]={5,3,4};
  const Int_t nPars_1[nSets]={2,2,4};

  AliTPCPIDResponse temp;
  TObjArray *arrCorrectionSets = corrections.Tokenize(";");
  if (arrCorrectionSets->GetEntriesFast() != nSets){
    temp.Error("AliTPCPIDResponse::CheckMultiplicityCorrectionString","Number of parameter sets not equal to %d. Please read documentation of this function", nSets);
    delete arrCorrectionSets;
    return 0x0;
  }

  for (Int_t iset=0 ;iset<nSets; ++iset) {
    TObjString *string=(TObjString*)arrCorrectionSets->RemoveAt(iset);
    TObjArray *arrTmp=string->String().Tokenize(",");
    delete string;
    string=0x0;

    if (arrTmp->GetEntriesFast() != nPars_0[iset] && arrTmp->GetEntriesFast() != nPars_1[iset]){
      temp.Error("AliTPCPIDResponse::CheckMultiplicityCorrectionString","Number of parameters in set %d not equal to %d or %d. Please read documentation of this function", iset, nPars_0[iset], nPars_1[iset]);
      delete arrCorrectionSets;
      delete arrTmp;
      return 0x0;
    }

    arrCorrectionSets->AddAt(arrTmp, iset);
  }

  Bool_t matchesParSet = kTRUE;
  for (Int_t iset=0;iset<nSets;++iset)
    matchesParSet = matchesParSet && ((TObjArray*)arrCorrectionSets->At(iset))->GetEntriesFast() == nPars_0[iset];

  if (matchesParSet)
    arrCorrectionSets->AddLast(new TObjString("0"));
  else {
    matchesParSet = kTRUE;
    for (Int_t iset=0;iset<nSets;++iset) {
      matchesParSet = matchesParSet && ((TObjArray*)arrCorrectionSets->At(iset))->GetEntriesFast() == nPars_1[iset];
    }
    if (matchesParSet)
      arrCorrectionSets->AddLast(new TObjString("1"));
  }

  return arrCorrectionSets;
}
