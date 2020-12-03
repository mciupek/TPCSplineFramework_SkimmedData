# TPCSplineFramework_SkimmedData
Software to run the TPCspline framework on skimmed data stored at GSI.

Here in general steps the process should be described in how the splines can be created. In the different folders one can find the code which is needed to run the different steps of the spline framework. In some of the folders a more detail describtion of the code can be found, in form of a README file.

A write-up on the creating process of the splines can be found in the attached Thesis of Benjamin Andreas Heß in Chapter 3 (CERN-THESIS-2015-158_2.pdf).

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


## How to create the splines (Step-by-Step).

1) Download the github directory to your user directory at lustre GSI. (git clone https://github.com/mciupek/TPCSplineFramework_SkimmedData.git)
2) Switch to the directory code_FlatTreeCreation.
3) load aliroot using the following command (at the moment Marians singularity container is loaded, but in principle other aliroot5 installations are fine)
   - source  /lustre/alice/users/miranov/NOTES/alice-tpc-notes2/JIRA/ATO-500/setDefaultEnv.sh
   - alicehub5Sif /lustre/alice/users/miranov/NOTESData/alice-tpc-notes
   
4) In the code fitdEdxCorrectionFiltered.C go to the function SetUpNewSpline(Int_t run) and make sure the default splines are loaded without eta and multiplicity correction. In addition make sure you select the correct pass and pass number. For that comment out: 
   - fPIDResponse->SetCustomTPCpidResponseOADBFile("...");
   - fPIDResponse->SetCustomTPCetaMaps("...");
- And deactivate multiplicty and eta correction, since for the first Iteration this should be turn off.
   - fPIDResponse->SetUseTPCMultiplicityCorrection(false);
   - fPIDResponse->SetUseTPCEtaCorrection(false);
   - fPIDResponse->SetUseTPCPileupCorrection(false);
 
5) Compile your code and try it on one chunk of the filtered tree. 

Make sure some splines are loaded (usually splines from the last pass) and that the flattrees are created and filled. The path of the input file needs to be defined in the filtered.list file. To test the chain start aliroot and compile the code.
   - gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
   - .L fitdEdxCorrectionFiltered.C+
   - AliDrawStyle::SetDefaults();
   - AliDrawStyle::ApplyStyle("figTemplate");
   - gStyle->SetOptTitle(1);
   - SetUpNewSpline($runnumber);                    // here the splines are set up, the run number need to be the same as for your input file
   - InitTree(1,1,$runnumber);                      // here the V0 and primary track tree are loaded, in addition they are linked to the event tree the run number need to be the same as for your input file
   - cacheCleanV0();                            // here the flat V0 tree is filled and created (V0tree.root)
   - cacheCleanTrack();                         // here the flat primary track tree is filled and created (CleanTrack.root)
   
6) If everything worked fine we can run the code on the full statistics of filtered trees. For that we use that code in the folder code_slurm.

7) The submit script is defined in submit_All_skimedData_ana.sh, make sure the paths are pointing to the correct directory (data set, pass, output directory etc.). In addition make sure the correct runlist is specificied in the Inputrunlist folder. The runlist can be usually found on the website of the Data Preperation Group (https://twiki.cern.ch/twiki/bin/view/ALICE/AliDPGRunLists).

If everything is defined you can submit your jobs via ./submit_All_skimedData_ana.sh.

Note: make sure you are on the virgo job submit note, before trying to submit your jobs. You can get on it usually using the following command: "ssh lxbk0595.gsi.de"

8) When the flattrees for V0 and primary tracks are generated we can process the outputs further and generate the THnSparse and TTree needed for the splines creation. For that we change to the folder code_SplineInput.

9) First of all we need to create a list of the input files. For that we switch to the folder "lists" and perform the following command, where $output_dir, $year, $dataset, $pass are depending on your output folder defined in the steps before:
   - ls -1v $output_dir/data/$year/$dataset/*/$pass/chunks_*/V0tree.root > V0_$dataset.list
   - ls -1v $output_dir/data/$year/$dataset/*/$pass/chunks_*/Cleantrack.root > track_$dataset.list


10) The code used to loop over the flattrees to create the THnSparse and TTree is AliSkimmedDataAnalysisMaker.cpp, there you can add new variables, selection criteria etc. . 

The looping is performed by calling the macro runV0.C ( aliroot -l runV0.C). Make sure that in this macro the correct (AliSkimmedDataAnalysisMaker_C.so) is loaded, to create it you need to call the command "make" when inside the singularity container (singAliroot, when container is loaded [Step 3]).

In the runV0.C file make sure that you enable the creation of the THnSparse and TTree, the following lines should not be commented out.
    - ana->read(filename);           //Loop to create THnsparse for V0
    - ana->Filltreeformap_V0(filename);         //Loop to create Tree for V0
    - ana->Read_tracktree(filename_track);               //Loop to create THnsparse for Track
    - ana->Filltreeformap_track(filename_track);            //Loop to create TTree for V0
         
 11) This process takes for Pb-Pb LHC18r approximate 15-20 minutes when run locally, in principle this could be also submitted to the batch farm but it is not necassary. Grab some coffee ;)
 
 At the end of this step you should find two root files in your directory:
 - TPCresidualPID.root                  // THnSparse
 - TPCPIDEtaTree.root                   // TTree
 
 12) After this step the input files for the Bethe-Bloch fit and eta/multiplicity are created. Go to the folder Spline_OfflineFramework and create a folder for your data set (LHC16q_pass2 for example) and copy the 0PERIOD_PASS_splineCreationv3.sh file to this folder. In this file make sure the paths are defined correctly and that the informations are matching the one of your dataset.
 
 13) To run the script load your aliroot5 environment and call bash 0PERIOD_PASS_splineCreationv3.sh. Now the whole spline creation is running step by step. 
 
 First the Bethe-Bloch function is fitted for all particles as function of momentum, than the eta/multiplicty corrections are extracted. Make sure the low momentum correction fits are described well. At the end the corrections are applied on the Tree for a first check how the TPCnsigma looks like. 

 At the end you will have created two files for the parametrization. First the TPCPIDResponse...root file which contains the momentum dependent parametrization and the multiplicty correction as string and the TPCEtamaps...root containing the eta dependent correction. The whole code of creating the splines is summarized in the Spline_OfflineFramework folder.
 
 14) These splines are now used for the second iteration to imporove the performance of the splines. Go back to step 4 and add the path of your created parametrization to:
     - fPIDResponse->SetCustomTPCpidResponseOADBFile("$yournewSpline");
     - fPIDResponse->SetCustomTPCetaMaps("$yournewSplineEtamap");
- Also make sure that the multiplicty correction and eta correction is turned on:
     - fPIDResponse->SetUseTPCMultiplicityCorrection(true);
     - fPIDResponse->SetUseTPCEtaCorrection(true);
     - fPIDResponse->SetUseTPCPileupCorrection(true);           // only important when creating splines for PbPb collisions.
  
 15) Repeat step 5) to 14) until your splines converge and you are happy with the results. This should be the case after 4-7 Iterations depending on the data. In step 10) we do not need to create the TTree again. Therefore we can comment out:
     - ana->Filltreeformap_track(filename_track)
     - ana->Filltreeformap_V0(filename).
 
 16) Now we can run the QA to check the performance of the splines on different particles and as function of different varialbes (centrality, eta etc.), need to be added yet to the github.
 
 
             
