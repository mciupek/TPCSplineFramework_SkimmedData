# TPCSplineFramework_SkimmedData
Software to run the TPCspline framework on skimmed data stored at GSI

Here in general steps the process should be described in how the splines can be created. In the different folders one can find the code which is needed to run the different steps of the spline framework.

The creation of the splines consist of the following steps:
- Creating flattrees for primary and V0 tracks which contains all the needed informations to create the input files for the offline spline framework. (code_FlatTreeCreation)
- Slurm code to run code for flattree creation over full statistics on the batch farm (code_slurm)
- Code to create the ThNsparse and TTree, which is needed as input for the offline spline framework from the flattrees. (code_SplineInput)
- Offline framework to extract splines for given ThNsparse and TTree, which contains itself on following steps.
    - Bethe-Bloch fit using clean samples of electron, pion, kaon, proton (and deuteron)
    - Extraction of low momentum correction for electron, pion, kaon, proton (and deuteron)
    - Extraction of the multiplicty correction (if pPb or PbPb)
    - Extraction of the eta correction
    - Extraction of the resolution correction
    - Check on protons if spline creation was successful
    - Check if TPCnsigma is beheaving well for electron,pion,kaon, proton (and deuteron)
    
These steps need to be repeated until the splines converge to a reasonable result.


How to create the splines (Step-by-Step).

1) In a first step the flattrees are created to generate the input file for the creation of the ouput files for the offline framework.
The necesarry code is found in the folder: code_CreateFlatTree.

The file fitdEdxCorrectionFiltered.C is the code needed to create the Flattrees. There serveral functions are defined which need to be called in the following order.

  gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
  .L fitdEdxCorrectionFiltered.C+
  SetUpNewSpline(295585);      // here the splines from the last iteration are loaded, in addition here we can turn on/off eta, multiplicty and pileup correction.
  InitTree(1,1,295585);        // here the filterered trees we are looping on are initialised, in addition event informations are coupled with track informations.
  cacheCleanV0();              // here the flat tree containing the V0 tracks is created, here we also define the track information which should be stored 
  cacheCleanTrack();           // here the flat tree containing the primary tracks is created, here we also define the track information which should be stored
  cacheEventFlat();            // here the event tree is created, this can be used for cross checks.
  
  As input a list of the location of the filtered trees is needed (filtered.list). At the moment only one chunk at the moment can be processed.
  
  The fitdEdxCorrectionFiltered.so_C is needed in the next step to run over the full statistics. Therefore everytime changes are done here the code needs to be compiled. Otherwise the changes are not taken into account when running on the GSI farm over full statistics.
  
  2) 
