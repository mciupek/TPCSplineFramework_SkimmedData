void extract(const char* input, const char* splines, const char* DataOrMc, const char* period, const char* pass, const char* collsys, Bool_t ownParameters = kFALSE, AliTPCcalibResidualPID::FitType fitType = AliTPCcalibResidualPID::kSaturatedLund, TString softwarepath = "", Int_t lowMult = -1, Int_t highMult = -1){
  //Comment in the following two lines if you want to use your own version of AliTPCcalibResidualPID
  gROOT->ProcessLine(".include $ALICE_ROOT/include");
  gROOT->ProcessLine(".include $ALICE_PHYSICS/include");
  gROOT->LoadMacro((softwarepath + TString("/AliTPCcalibResidualPID2.cxx+g")).Data());

  TFile TPCresPID(input);
  TFile f(input);
  hsp2=(THnSparseF*)TPCresPID.Get("fHistPidQA");
  //hsp2=(THnSparseF*)TPCresPID->FindObject("fHistPidQA");
  if (lowMult != -1) {
    hsp2->GetAxis(6)->SetRangeUser(lowMult+0.1,highMult-0.1);
    cout << "all" << endl;
  }
  else {
    if (collsys != "PP") {
      hsp2->GetAxis(6)->SetRangeUser(0.0, 4999.9); // limits the multiplicity to events with a maximum of 500 tracks
      //hsp2->GetAxis(7)->SetRangeUser(0,0.9); // do avoid detector effects.

    }
  }

  gROOT->cd();
  Double_t parametersBB[6] = { 0. };
  //Give standard parameters
  parametersBB[0] = 65; //Mainly responsible for the rise of the low x (Higher rise for lower values)
  parametersBB[1] = 18; //Lower minimum for lower values
  parametersBB[2] = 1; //more kink for higher values
  parametersBB[3] = 1.0; //Lower values: Higher rise at low x
  parametersBB[4] = 130; //Rise at high x: Higher values for higher rise
  parametersBB[5] = -60; //Influences the total height of the curve. Will be cancelled out for high x values (but depending on its value they have to be very high)

  TString parametersMacroPath = (TString(gSystem->GetFromPipe("echo $(pwd)")) + TString("/extractParameters.C")).Data();

  Int_t success = gROOT->LoadMacro(parametersMacroPath.Data());
  //Load specific parameters for this period
  if (success == 0)
    extractParameters(fitType, parametersBB, hsp2);
  else
    printf("No %s/extractParameters.C found, using standard extraction parameters\n", parametersMacroPath.Data());

  AliTPCcalibResidualPID2* calib = new AliTPCcalibResidualPID2();  //Comment out if you dont want to use the own class
  //AliTPCcalibResidualPID* calib = new AliTPCcalibResidualPID(); //Comment out if you do want to use the own class

  if (ownParameters) {
    TCanvas* c = new TCanvas("FitFunction");
    c->SetLogx(1);
    c->RangeAxis(0.9,-10,10e4,120);
//     calib->SetUpFitFunction(parametersBB,fitType,0.9,10e3,kFALSE,kFALSE,0x0)->Draw();
    TF1* bbFunc = calib->SetUpFitFunction(parametersBB,fitType,0.9,10e3,kFALSE,kFALSE,0x0);
    bbFunc->Draw();
    calib->CreatePlotWithOwnParameters(hsp2,kTRUE,DataOrMc,collsys,parametersBB,fitType,0.9,10e4);
  }
  else {
    if (fitType != AliTPCcalibResidualPID::kSaturatedLund && fitType != AliTPCcalibResidualPID::kLund)
      calib->ExtractResidualPID(hsp2,kTRUE,splines,DataOrMc,period,pass,collsys,0x0,"", fitType);
    else
      calib->ExtractResidualPID(hsp2,kTRUE,splines,DataOrMc,period,pass,collsys,parametersBB,"", fitType);
  }

}
