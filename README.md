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
