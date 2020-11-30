void extractParameters(AliTPCcalibResidualPID::FitType fitType = AliTPCcalibResidualPID::kSaturatedLund, Double_t* parametersBB, THnSparseF* hsp2) {
  
//   parametersBB[0] = 59;
//   parametersBB[1] = 19;
//   parametersBB[2] = 15;
//   parametersBB[3] = 0.8;
//   parametersBB[4] = 73;
//   parametersBB[5] = -3;
//   parametersBB[0] = 30;
//   parametersBB[1] = 10;
//   parametersBB[2] = 20;
//   parametersBB[3] = 0.6;
//   parametersBB[4] = 10;
//   parametersBB[5] = -90;  
  parametersBB[0] = 65; //Mainly responsible for the rise of the low x (Higher rise for lower values)
  parametersBB[1] = 18; //Lower minimum for lower values
  parametersBB[2] = 1; //more kink for higher values
  parametersBB[3] = 1.0; //Lower values: Higher rise at low x
  parametersBB[4] = 130; //Rise at high x: Higher values for higher rise
  parametersBB[5] = -60; //Influences the total height of the curve. Will be cancelled out for high x values (but depending on its value they have to be very high)  
  
  //For Aleph
  if (fitType != AliTPCcalibResidualPID::kSaturatedLund && fitType != AliTPCcalibResidualPID::kLund) {
    parametersBB[0] = 1.6; 
    parametersBB[1] = 20.3;
    parametersBB[2] = -10; 
    parametersBB[3] = 2.6; 
    parametersBB[4] = 2.3;
    parametersBB[5] = 0.02;
  }
}
