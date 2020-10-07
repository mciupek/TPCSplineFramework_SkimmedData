# TPCSplineFramework_SkimmedData
Software to run the TPCspline framework on skimmed data stored at GSI

Here in general steps the process should be described in how the splines can be created. In the different folders one can find the code which is needed to run the different steps of the spline framework.

The creation of the splines consist of the following steps:
1) Creating flattrees for primary and V0 tracks which contains all the needed informations to create the input files for the offline spline framework. (code_FlatTreeCreation)
2) Slurm code to run code for flattree creation over full statistics on the batch farm (code_slurm)
3) Code to create the ThNsparse and TTree, which is needed as input for the offline spline framework from the flattrees. (code_SplineInput)
4) Offline framework to extract splines for given ThNsparse and TTree, which contains itself on following steps.
    4.1) Bethe-Bloch fit using clean samples of electron, pion, kaon, proton (and deuteron)
    4.2) Extraction of low momentum correction for electron, pion, kaon, proton (and deuteron)
    4.3) Extraction of the multiplicty correction (if pPb or PbPb)
    4.4) Extraction of the eta correction
    4.5) Extraction of the resolution correction
    4.6) Check on protons if spline creation was successful
    4.7) Check if TPCnsigma is beheaving well for electron,pion,kaon, proton (and deuteron)
    
These steps need to be repeated until the splines converge to a reasonable result.
