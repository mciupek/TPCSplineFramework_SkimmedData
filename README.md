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

1) Download the github directory to your user directory at lustre GSI.
2) Switch to the directory code_CreateFlatTree.
3) load aliroot using the following command (at the moment Marians singularity container is loaded, but in principle other aliroot5 installations are fine)
3.1) source  /lustre/alice/users/miranov/NOTES/alice-tpc-notes2/JIRA/ATO-500/setDefaultEnv.sh
3.2) alicehub5Sif /lustre/alice/users/miranov/NOTESData/alice-tpc-notes
4) In the code fitdEdxCorrectionFiltered.C got to the function  SetUpNewSpline(Int_t run) and make sure the default splines are loaded without eta and multiplicity correction. For the comment out: 
fPIDResponse->SetCustomTPCpidResponseOADBFile("...");
fPIDResponse->SetCustomTPCetaMaps("...");
And deactivate multiplicty and eta correction, since for the first Iteration this should be turn off.
  fPIDResponse->SetUseTPCMultiplicityCorrection(false);
  fPIDResponse->SetUseTPCEtaCorrection(false);
  fPIDResponse->SetUseTPCPileupCorrection(false);
 
5) Compile your code and try it on one chunk of the filtered tree. Make sure some splines are loaded (usually splines from the last pass) and that the flattrees are created.
6) If everything worked fine we can run the code on the full statistics of filtered trees. For that we use that code in the folder code_slurm.
7) The submit script is defined in submit_All_skimedData_ana.sh, make sure the paths are pointing to the correct directory (data set, pass, output directory etc.). If everything is defined you can submit your jobs via ./submit_All_skimedData_ana.sh
8) When the ouput is generated we can process the outputs further and generate the THnSparse and TTree needed for the splines creation. For that we change to the folder code_SplineInput.
9) First of all we need to create a list of the input files. For that we switch to the folder "lists" and perform the following command:
ls -1v $output_dir/data/$year/$dataset/*/$pass/chunks_*/V0tree.root > V0_$dataset.list
ls -1v $output_dir/data/$year/$dataset/*/$pass/chunks_*/Cleantrack.root > track_$dataset.list
where $output_dir, $year, $dataset, $pass are depending on your folder and dataset processed.

10) The code used to loop over the flattrees to create the THnSparse and TTree is AliSkimmedDataAnalysisMaker.cpp. The looping is performed by calling the macro runV0.C ( aliroot -l runV0.C). There make sure that you enable the creation of the THnSparse and TTree, the following lines should not be commented out.
         ana->read(filename);           //Loop to create THnsparse for V0
		 ana->Filltreeformap_V0(filename);         //Loop to create Tree for V0
         ana->Read_tracktree(filename_track);               //Loop to create THnsparse for Track
	     ana->Filltreeformap_track(filename_track);            //Loop to create TTree for V0
         
 11) This process takes for Pb-Pb LHC18r approximate 15-20 minutes when run locally, in principle this could be also submitted to the batch farm but it is not necassary. Grab some coffee ;)
 
             
