uploadfiletoalien() {
  FILE=$1
  Folder=$2
  targetfolder=/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$Folder
  echo "Uploading file $FILE to $targetfolder"
  echo " ----------- "
  alien_cp $FILE alien://$targetfolder
  if [ "$(alien_ls $targetfolder/$(basename $FILE))" != "$(basename $FILE)" ];then
    "Upload of $FILE failed, retry."
    alien_cp $FILE alien://$targetfolder
    if [ "$(alien_ls $targetfolder/$(basename $FILE))" != "$(basename $FILE)" ];then
      "Upload of $FILE failed again. No automatic retry."
      uploadfailed=1
    fi
  fi
}

uploadfiletolustre() {
  FILE=$1
  NewFileName=$2
  echo "Uploading file $FILE"
  scp $FILE maschmid@$lustrehostName:/lustre/nyx/alice/users/$gsiName/software/PIDResponse/COMMON/PID/data/$NewFileName
}

downloadandmergefromstage() {
  mergefolders=$(eval "alien_ls /alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/Stage_1")
  for MERGEFOLDER in $mergefolders
  do
    mkdir $folder/$MERGEFOLDER
    alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/Stage_1/$MERGEFOLDER/root_archive.zip file:$folder/$MERGEFOLDER
    unzip  -o $folder/$MERGEFOLDER/root_archive.zip -d $folder/$MERGEFOLDER
  done
  for FILE in TPCresidualPID.root AnalysisResults.root TPCPIDEtaTree.root TPCPIDEtaQA.root
  do
    aliroot "$software/recursiveMerging.C+(\"$folder/*/$(basename $FILE)\", \"$folder/$(basename $FILE .root)\", 4)" -b -q
  done
  rm -rf $mergefolders
  rm TPCresidualPID.*.root AnalysisResults.*.root TPCPIDEtaTree.*.root TPCPIDEtaQA.*.root
}

#### All Variables are to be declared in the script in the appropriate folder ####
if [ "$ALICE_PHYSICS" = "" ];then
  source $splinepath/env.sh
fi
########## Download Files from alien ################
if [ "$1" = "download" ];then
if [[ "$trainName" = "V0"* ]];then
    echo "This will download and extract files from lustre to $folder. Extracting will overwrite existing files"
    scp maschmid@$lustrehostName:/lustre/nyx/alice/users/$gsiName/train/$trainName/mergedPeriods/*/TPC* $folder
    scp maschmid@$lustrehostName:/lustre/nyx/alice/users/$gsiName/train/$trainName/mergedPeriods/*/AnalysisResults.root $folder
  else
    echo "This will download and extract files from alien to $folder. Extracting will overwrite existing files"
    if [ "$2" = "newtoken" ];then
      alien-token-init $alienusername
    fi
    alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/root_archive.zip file:$folder
    if [ -e $folder/root_archive.zip ];then
      echo "Download complete. Begin unzipping"
      unzip  -o $folder/root_archive.zip -d $folder
      echo "Unzipping completed"
    else
      echo "No root_archive.zip found. Download TPCresidualPID.root, TPCPIDEtaTree.root, AnalysisResults.root and TPCPIDEtaQA.root from alien:/alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/"
      alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/AnalysisResults.root file:$folder
      if [ ! -e $folder/AnalysisResults.root ];then
        echo "No AnalysisResults.root file found. Try to download and merge from stage"
        downloadandmergefromstage
      else
        alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/TPCresidualPID.root file:$folder
        alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/TPCPIDEtaTree.root file:$folder
        alien_cp alien:///alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/TPCPIDEtaQA.root file:$folder
      fi
    fi
  fi
########## STEP 0 ####################################
elif [ "$1" = "0" ]; then
echo "****** Step 0: Extract Bethe-Bloch Curve ****** "
  if [ ! -e $basefolder/$etaTreeName ];then
    if [ -e $basefolder/TPCPIDEtaTree.root ];then
      mv $basefolder/TPCPIDEtaTree.root $folder/$etaTreeName
    else
      echo "TPCPIDEtaTree.root not found, etaMaps cannot be produced"
    fi
  fi
  if [ "$collsys" = "PPB" -o "$collsys" = "PBPB" ];then
    if [ ! -e $basefolder/$etaTreeNameMultSplitted ];then
      echo "Split Tree in multiplicity bins"
      aliroot "$software/splitPbPbTreeMultiplicity.C+(\"$basefolder\",kFALSE,$stepSizeMult,$stepSizeMult)" -b -q -l
      multCorrFile=$(ls -rt1 $basefolder/*multiplicityDependence* | tail -1)
      aliroot "$trainmacros/GetMultiplicityFactors.C(\"$multCorrFile\", $useSigmaMultCorrection, $multtanThetaBin, $multpBin, $useNewMultParametrization)" -b -q -l
      multParameters=$(cat $basefolder/multParameters.txt)
    fi
  fi
  if [ ! -e $folder/extractParameters.C ];then
    echo "Copy extractParameters.C"
    cp $software/extractParameters.C $folder/
  fi
  if [ "$2" != "testParameters" ];then
    if [ "$MultSeparationString" != "" ];then
      mkdir $folder/SplineQA
      bash $software/HelpFunctions.sh "LoopThroughString" $MultSeparationString 'root.exe "'$software'/extract.C(\"'$folder/TPCresidualPID.root'\", \"'$spline2'\", \"DATA\", \"'$period'\", \"'$pass'\", \"'$collsys'\", kFALSE, '$BBFunctionNumber', \"'$software'\", $MultLow, $MultHigh)" -l -b -q;for FILE in '$folder'/*.pdf '$folder'/splines_QA*.root '$folder'/V0_dEdx_purity*.root; do fileext=${FILE##*.};mv $FILE '$folder'/SplineQA/$(basename $FILE .$fileext)"_"$MultLow"_"$MultHigh.$fileext; done; mv '$spline2' '$folder/$(basename $spline2 .root)'"_"$MultLow"_"$MultHigh.root'
      cp $folder/SplineQA/bethebloch* $folder
    else
      root.exe "$software/extract.C(\"$folder/TPCresidualPID.root\", \"$folder/$spline2\", \"DATA\", \"$period\", \"$pass\", \"$collsys\" , kFALSE, $BBFunctionNumber, \"$software\")" -l -b -q
    fi
  else
    root.exe "$software/extract.C(\"$folder/TPCresidualPID.root\", \"$folder/$spline2\", \"DATA\", \"$period\", \"$pass\", \"$collsys\" , kTRUE, $BBFunctionNumber, \"$software\")" -l
  fi
echo "****** Step 0 finished *****"
########## STEP 1 ##########
elif [ "$1" = "1" ]; then
echo "****** Step 1 ****** "
  if [[ "$MultSeparationString" != "" ]];then
    sourcepath=$folder/TPCPIDResponseOADB_temp.root
  else
    sourcepath=$folder/$spline2
  fi
   aliroot  "$software/correctShapeEtaTree.C+(kFALSE, \"\", \"\", \"\", kTRUE, \"$sourcepath\", \"$spline\", \"$basefolder\", $hasMult, kFALSE, \"$etaTreeNameMultSelected\", \"fTree\", \"$multParameters\")" -b -q -l
echo "****** Step 1 finished ******"
########## STEP 2 ##########
elif [ "$1" = "2" ];then
echo "****** Step 2 ****** "
   if [ "$2" != "nosearch" ];then
      file1=$(basename $(ls -rt1 $folder/*PIDetaTree*NewSplines* | grep -v outputCheckShapeEta | tail -1))
      echo "Processing file $file1"
   fi
   aliroot "$software/checkShapeEtaTree.C+(0x0, $binning, $binning, $TOFcut, $mergeBinThresh, $V0cut, $V0TOFcut, $maxDeviation1, kFALSE, 0, 60, \"$folder\", \"$file1\", kFALSE, \"TPCPIDResponse.root\", \"$spline\", 1, \"fTree\", \"$(basename $basefolder/$etaTreeNameComplete .root)\")" -b -q -l
echo "****** Step 2 finished ******"
########## STEP 3 ##########
elif [ "$1" = "3" ];then
  echo "****** Step 3 ****** "
   if [ "$2" != "nosearch" ];then
      file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))
      echo "Processing file $file2"
   fi
  if [[ $MultSeparationString ]];then
    sourcepath=$folder/TPCPIDResponseOADB_temp.root
  else
    sourcepath=$folder/$spline2
  fi
   aliroot  "$software/correctShapeEtaTree.C+(kTRUE, \"$folder\", \"$file2\", \"NoNormalisation\", kTRUE, \"$sourcepath\", \"$spline\", \"$basefolder\", $hasMult, kFALSE, \"$etaTreeNameMultSelected\", \"fTree\", \"$multParameters\")" -b -q -l
echo "****** Step 3 finished ******"
########## STEP 4 ##########
elif [ "$1" = "4" ];then
  echo "****** Step 4 ****** "
   if [ "$2" != "nosearch" ];then
      file3=$(basename $(ls -rt1 $folder/*PIDetaTree*outputCheckShapeEtaTree* | tail -1))
      echo "Processing file $file3"
   fi
   aliroot  "$software/checkShapeEtaTree.C+(0x0, $binning, $binning, $TOFcut, $mergeBinThresh, $V0cut, $V0TOFcut, $maxDeviation2, kTRUE, 0, 60, \"$folder\", \"$file3\", kFALSE, \"TPCPIDResponse.root\", \"$spline\", 1, \"fTree\", \"$(basename $basefolder/$etaTreeNameComplete .root)\")" -b -q -l
echo "****** Step 4 finished ******"
########## STEP 5 ##########
elif [ "$1" = "5" ];then
echo "****** Step 5: Check Pull Tree ****** "
   if [ "$2" != "nosearch" ];then
      file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))
      echo "Processing file $file2"
      file4=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1))
      echo "Processing file $file4"
   fi
   if [[ $MultSeparationString ]];then
     sourcepath=$folder/TPCPIDResponseOADB_temp.root
     multParameters=""
   else
     sourcepath=$folder/$spline2
   fi
#    bash $software/HelpFunctions.sh "LoopThroughString" $MultSeparationString 'aliroot "'$software'/checkPullTree.C+(\"'$folder'\", \"'$folder/$file2'\",\"'$folder/$file4'\", \"NoNormalisation\", \"'$collsys'\", kTRUE, 2, \"'$(basename $spline2 .root)\"_\"\$MultLow\"_\"\$MultHigh'\", \"'$spline'\", \"'$etaTreeNameComplete'\", \"fTree\", \"\", $MultLow, $MultHigh)" -b -q -l' "" 'pullfile=$(ls '$folder/'*checkPullSigma* | xargs -n1 basename | grep -v Mult | tail -n1);mv '$folder/'$pullfile '$folder'/$(basename $pullfile .root)"_"Mult"_"$MultLow"_"$MultHigh.root'
   aliroot  "$software/checkPullTree.C+(\"$folder\", \"$folder/$file2\",\"$folder/$file4\", \"NoNormalisation\", \"$collsys\", kTRUE, 2, \"$sourcepath\", \"$spline\", \"$etaTreeNameComplete\", \"fTree\", \"$multParameters\")" -b -q -l
echo "****** Step 5 finished ******"
########## QA STEP ##########
# Tasks 5 and 6 are only for the final QA
elif [ "$1" = "QA1" ];then
   if [ "$collsys" = "PP" ]; then
      aliroot  "$software/checkShapeEta.C+(\"$folder\", -1,-1, 0, \"TPCPIDEtaQA.root\", \"TPCPIDEtaQA\")" -b -q -l
   elif [ $collsys = "PPB" ]||[ $collsys = "PBPB" ]; then
      aliroot  "$software/checkShapeEta.C+(\"$folder\", $stepSizeMult,$maxMult, 0, \"TPCPIDEtaQA.root\", \"TPCPIDEtaQA\")" -b -q -l
   fi
elif [ "$1" = "QA2" ];then
   aliroot  "$software/MakePIDqaReport.C+(\"$folder/AnalysisResults.root\",\"$folder/PIDqaReport.pdf\", \"PIDqa\")" -b -q -l
elif [ "$1" = "GetQAEtaAdv" ];then
  aliroot "$software/GetSplineQAPlots.C(\"$(ls -rt1 $folder/outputCheckShapeEtaAdv* | tail -1)\",kFALSE, $stepSizeMult, $maxMult)" -l

########## MULTIPLICITY DEPENDENCE ##########

# EXTRACT MULTIPLICITY JUST ONCE AND DO NOT USE IT IN correctShapeEtaTree.C DURING SPLINE CREATION. CHECK ITS PARAMETERS WITH checkPullTree.C
elif [ "$1" = "mult" ];then
echo "Extract multiplicity dependence"
#pPb: 4,150, tanThetaBin2 (-0.6->-0.4) and params from slopes2, tanTheta abh from pBin9 (0.8-0.8x GeV/c)
#PbPb:3,800, tanThetaBin2, slopes2 and sigmaSlopes, pBin9
  if [ $collsys = "PPB" ];then
    widthfactor=4
    multstepsize=150
  elif [ $collsys = "PBPB" ];then
    widthfactor=3
    multstepsize=800
  fi
  aliroot "$software/extractMultiplicityDependence.C+(\"$folder\",$widthfactor,$multstepsize,kFALSE,kFALSE,\"bhess_PIDetaTree.root\",\"fTree\",\"$spline2\",\"$spline\")" -b -q -l
   # second argument stands for the width factor. A value of 4 seems to be reasonable for pPb. third argument for the step size in multiplicity -> 150 in pPb
   # 4 parameters from tanThetaBin2/slopes2_tanTheta2
echo "****** Step mult finished ******"
###########     COMP SPLINES     ############
elif [ "$1" = "comp" ];then
  if [[ "$Iteration" = "1" ]];then
    echo "****** First Iteration, nothing to compare *******"
  else
    echo "****** Step comp: Compare with old Bethe-Bloch ****** "
    aliroot  "$software/compareSplines2.C(\"$prevfolder/$spline2\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE,  \"it$(expr $Iteration - 1)\", \"it$Iteration\", \"$folder\",\"pdf\",0.90,1.10,\"$MultSeparationString\")" -b -q -l
#     bash $software/HelpFunctions.sh "LoopThroughString" $MultSeparationString 'aliroot  "$software/compareSplines2.C(\"$prevfolder/$(basename $spline2 .root)_$MultLow"_"$MultHigh.root\", \"$folder/$(basename $spline2 .root)_$MultLow"_"$MultHigh.root\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"it$(expr $Iteration - 1)\", \"it$Iteration\", \"$folder\", \"pdf\",0.98,1.02)" -b -q -l; mv $folder/CompareSplinesLHC13C_PASS4it$(expr $Iteration - 1)it$Iteration.pdf $folder/CompareSplinesLHC13C_PASS4it$(expr $Iteration - 1)it$Iteration"_"$MultLow"_"$MultHigh.pdf'  '"aliroot  \"$software/compareSplines2.C(\"$prevfolder/$spline2\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"it$(expr $Iteration - 1)\", \"it$Iteration\", \"$folder\", \"pdf\",0.98,1.02)\" -b -q -l"'
     echo "****** Step comp finished ******"
  fi
elif [ "$1" = "compMultSplines" ];then
  aliroot  "$software/compareSplines2.C(\"$folder/$spline2\", \"\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"MultBin$j\", \"MultBin1\", \"$folder\",\"root\",0.98,1.02,\"$MultSeparationString\")" -b -q -l
#   FirstMult=$(cut -d: -f1 <<< "$MultSeparationString")
#   SecondMult=$(cut -d: -f2 <<< "$MultSeparationString")
#   MultLow=$(cut -d: -f3 <<< "$MultSeparationString")
#   i=3
#   j=2
#   while [ "$MultLow" != "" ]
#   do
#     MultHigh=$(cut -d: -f$(expr $i + 1) <<< "$MultSeparationString")
#     aliroot  "$software/compareSplines2.C(\"$folder/$(basename $spline2 .root)"_"$MultLow"_"$MultHigh.root\", \"$folder/$(basename $spline2 .root)"_"$FirstMult"_"$SecondMult.root\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"MultBin$j\", \"MultBin1\", \"$folder\",\"root\")" -b -q -l
#     i=$(expr $i + 2)
#     j=$(expr $j + 1)
#     MultLow=$(cut -d: -f$i <<< "$MultSeparationString")
#   done
###########     Make OADB Object  ############
elif [ "$1" = "OADB" ];then
echo "****** Step OADB: Produce OADB File ****** "
  if [ "$firstRun" = "" ];then
    firstRun=0
    lastRun=0
  fi
  aliroot "$trainmacros/MakeTPCPIDResponseOADB.C+(\"$folder/$OADBFileName\", \"""\", \"$folder/$spline2\", $firstRun, $lastRun, $passNumber, \"$multParameters\", \"$MultSeparationString\", \"$pileupDefinition\")" -b -q -l
  echo "****** Step OADB finished ******"
###########     Make eta Maps - need to update addMapToFile.C before!! ############
elif [ "$1" = "etaMaps" ];then
  echo "****** Step etaMaps: Produce etaMaps ****** "
   mkdir $folder/etaMaps
   if [ "$2" != "nosearch" ];then
      file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))
      echo "Processing file $file2"
      file4=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1))
      echo "Processing file $file4"
   fi
   aliroot "$trainmacros/addMapToFile.C+(\"$folder/$file2\", \"$normalisation\", \"$period\", $passNumber, kFALSE, kFALSE,\"etaMaps\",\"$etaMapName\", $firstRun, $lastRun)" -b -q -l
   aliroot "$trainmacros/addMapToFile.C+(\"$folder/$file4\", \"$normalisation\", \"$period\", $passNumber, kFALSE, kTRUE,\"etaMaps\",\"$etaMapName\", $firstRun, $lastRun)" -b -q -l
   mv $folder/etaMaps/UsedFilesForMap.txt $folder/etaMaps/$UsedFilesForMapName
   echo "****** Step etaMaps finished ******"
###########     Upload the necessary files to alien ############
elif [ "$1" = "upload" ];then
echo "****** Step upload ****** "
  if [ "$toLustre" = "1" ];then
#     for FILE in "$folder/$OADBFileName TPCPIDResponseOADB.root" "$folder/etaMaps/$etaMapName TPCetaMaps.root"
    for FILE in "$folder/$OADBFileName" "$folder/etaMaps/$etaMapName"
    do
      uploadfiletolustre $FILE
    done
  else
    if [ "$2" = "newtoken" ];then
      alien-token-init $alienusername
    fi
    if [ "$alienfolder" != "" ];then
    alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder
    fi
    for FILE in $folder/$OADBFileName $folder/etaMaps/$etaMapName
    do
      uploadfiletoalien $FILE
    done
    echo "Use the following string as 6th argument for PIDResponse_Devel"
    echo "TPC-OADB:alien:///alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$OADBFileName ;TPC-Maps:alien:///alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$etaMapName"
    if [ "$uploadfailed" = "1" ];then
    echo "Attention: The upload failed!!"
    uploadfailed=0
    fi
  fi
echo "****** Step upload finished ******"
elif [ "$1" = "bbsystematics" ];then
  echo "******** Produce Systematic plots for the BB fit *********"
  cd $folder
  aliroot "$software/GetSplineQAPlots.C(\"$(ls -rt1 $folder/outputCheckShapeEtaAdv* | tail -1)\")" -l -b -q
  for BBFunctionLoop in ALEPH Lund ModifiedALEPH
  do
    case "$BBFunctionLoop" in
      ALEPH) BBFunctionNumberLoop=0;;
      Lund) BBFunctionNumberLoop=1;;
      ModifiedALEPH) BBFunctionNumberLoop=3;;
    esac
    bash $software/HelpFunctions.sh "LoopThroughString" "$MultSeparationString" 'root.exe "$software/extract.C(\"$folder/TPCresidualPID.root\", \"$folder/splines_'$BBFunctionLoop'""_$MultLow""_$MultHigh.root\", \"DATA\", \"$period\", \"$pass\", \"$collsys\", kFALSE, '$BBFunctionNumberLoop', \"$software\", $MultLow, $MultHigh)" -l -b -q;mv $folder/splines_QA_BetheBlochFit_'$BBFunctionLoop'.root $folder/splines_QA_BetheBlochFit_'$BBFunctionLoop'""_$MultLow""_$MultHigh.root' 'root.exe "$software/extract.C(\"$folder/TPCresidualPID.root\", \"$folder/splines_'$BBFunctionLoop'.root\", \"DATA\", \"$period\", \"$pass\", \"$collsys\", kFALSE, '$BBFunctionNumberLoop', \"$software\")" -l -b -q'
    rm bethebloch*_$BBFunctionLoop.pdf
  done
  mv $folder/SplineQA/splines_QA_BetheBlochFit_* $folder/
  rm $folder/V0_dEdx_purity*.root
  rm $folder/splines_QA_Residual*
  rm $folder/splines_QA_arrGraphs_check_jy.root
  rm $folder/polynomials*.pdf
  bash $software/HelpFunctions.sh "LoopThroughString" "$MultSeparationString" 'root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_\", \"\", $MultLow, $MultHigh)" -l -b -q' 'root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_\")" -l -b -q'
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_ALEPH.root\")" -l -b -q
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_Lund.root\")" -l -b -q
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_ModifiedALEPH.root\")" -l -b -q
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/splines_QA_BetheBlochFit_SaturatedLund.root\")" -l -b -q
  aliroot  "$software/compareSplines2.C(\"$folder/splines_Lund.root\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"Lund\", \"SaturatedLund\", \"$folder\", \"root\")" -b -q -l
  aliroot  "$software/compareSplines2.C(\"$folder/splines_ALEPH.root\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"ALEPH\", \"SaturatedLund\", \"$folder\", \"root\")" -b -q -l
  aliroot  "$software/compareSplines2.C(\"$folder/splines_ModifiedALEPH.root\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"ModifiedALEPH\", \"SaturatedLund\", \"$folder\", \"root\")" -b -q -l
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/CompareSplines$perpass""ALEPHSaturatedLund.root\")" -l -b -q
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/CompareSplines$perpass""LundSaturatedLund.root\")" -l -b -q
  root.exe "$software/ProduceSystematicPlots.C(\"$folder/CompareSplines$perpass""ModifiedALEPHSaturatedLund.root\")" -l -b -q
  rm $folder/CompareSplines*SaturatedLund.root
#
  cd -
  echo "******** Finished producing systematics for the BB fit **************"
elif [ "$1" = "etasystematics" ];then
  echo "*********** Produce systematic plots for the eta corection ***********"
  if [ -e $folder/TPCPIDEtaQA.root ];then
    root.exe "$software/ProduceSystematicPlots.C(\"$folder/TPCPIDEtaQA.root\", \"$software\")" -l -b -q
  elif [ -e $folder/Iteration$QAIteration/TPCPIDEtaQA.root ];then
    root.exe "$software/ProduceSystematicPlots.C(\"$folder/Iteration$QAIteration/TPCPIDEtaQA.root\", \"$software\")" -l -b -q
  else
    echo "TPCPIDEtaQA.root not found"
  fi
  echo "******** Finished producing systematics for the eta correction **************"
elif [ "$1" = "ressystematics" ];then
  echo "*********** Produce systematic plots for the resolution ***********"
  if [ -e $folder/TPCresidualPID.root ];then
    root.exe "$software/ProduceSystematicPlots.C(\"$folder/TPCresidualPID.root\")" -l -b -q
  else
    root.exe "$software/ProduceSystematicPlots.C(\"$folder/Iteration$QAIteration/TPCresidualPID.root\")" -l -b -q
  fi
  echo "******** Finished producing systematics for the resolution **************"
elif [ "$1" = "uploadsystematics" ];then
  echo "****** Step upload systematics ****** "
  uploadfiletoalien $folder/$SplineSystematics.root $splinefolder
  echo "****** Step upload systematics finished ******"
elif [ "$1" = "uploadsplines" ];then
  echo "****** Step uploadsplines ****** "
  if [ "$2" = "newtoken" ];then
    alien-token-init $alienusername
  fi
  lastfolder=$basefolder/Iteration$(expr $Iteration - 1) ## selected spline folder
  finalfolder=$basefolder/Iteration$(expr $Iteration) ## fianlQA folder

  EtaTreeFile1=$(ls -rt1 $lastfolder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1)
  EtaTreeFile2=$(ls -rt1 $lastfolder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1)
  pullSigma_last=$(ls -rt1 $lastfolder/*checkPullSigma* | tail -1)
  QAfile1=$(ls $lastfolder/*.pdf)
  QAfile2=$(ls $lastfolder/V0_dEdx_purity_*.root)
  QAfile3=$(ls $lastfolder/splines*.root)

  pullSigma_final=$(ls -rt1 $finalfolder/*checkPullSigma* | tail -1) ## can be not existed
  etaAdv=$(ls -rt1 $finalfolder/outputCheckShapeEtaAdv* | tail -1)
  QAfile11=$(ls $finalfolder/*.pdf)
  QAfile22=$(ls $finalfolder/V0_dEdx_purity_*.root)
  QAfile33=$(ls $finalfolder/splines*.root)

  echo "Creating alien directory: /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$splinefolder"
  alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$splinefolder
  alien_ls /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder
  alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$splinefolder/Iteration$(expr $Iteration)
  alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$splinefolder/Iteration$(expr $Iteration - 1)

  ## new upload format: with iteration folders
  for FILE in $EtaTreeFile1 $EtaTreeFile2 $pullSigma_last $QAfile1 $QAfile2 $QAfile3
    do
      uploadfiletoalien $FILE $splinefolder/Iteration$(expr $Iteration - 1)
    done
  for FILE in $pullSigma_final $etaAdv $QAfile11 $QAfile22 $QAfile33
    do
      uploadfiletoalien $FILE $splinefolder/Iteration$(expr $Iteration)
    done

  if [ "$uploadfailed" = "1" ];then
    echo "Upload of at least one spline file failed."
    uploadfailed=0
  fi
  echo "****** Step uploadsplines finished ******"
########### Extracting Lambda Parameter #####################
elif [ "$1" = "lambda" -o "$1" = "Lambda" ];then
echo "***************** Extracting Lambda Parameter for MTF *********"
aliroot "$software/FitAsymmetricShape.C+(2.0,0.5,10,\"$folder\",\"TREE\",\"$etaTreeName\",kTRUE)" -l -q
########### Create own subdirectory for this iteration and move all files belonging to a specific iteration in this directory #####################
elif [ "$1" = "cu" -o "$1" = "cleanup" ];then
   mkdir $folder/Iteration$Iteration
   cp $folder/extractParameters.C $folder/Iteration$Iteration
   mv $folder/SplineSystematics.root $folder/Iteration$Iteration
   mv $folder/*.pdf $folder/Iteration$Iteration
   mv $folder/splines* $folder/Iteration$Iteration
   mv $folder/*Splines* $folder/Iteration$Iteration
   mv $folder/$OADBFileName $folder/Iteration$Iteration
   mv $folder/TPCPIDResponseOADB_temp.root $folder/Iteration$Iteration
   mv $folder/etaMaps $folder/Iteration$Iteration
   mv $folder/AnalysisResults.root $folder/Iteration$Iteration
   mv $folder/*checkPullSigma* $folder/Iteration$Iteration
   mv $folder/TPCresidualPID.root $folder/Iteration$Iteration
   mv $folder/V0* $folder/Iteration$Iteration
   mv $folder/EventStat_temp.root $folder/Iteration$Iteration
   mv $folder/outputCheckShapeEtaAdv*.root $folder/Iteration$Iteration
   mv $folder/TPCPIDEtaQA.root $folder/Iteration$Iteration
   mv $folder/settingsForMapCreation.txt $folder/Iteration$Iteration/etaMaps
   mv $folder/root_archive.zip $folder/Iteration$Iteration
   mv $folder/SplineQA $folder/Iteration$Iteration
   echo "The following files have been moved to Iteration$Iteration"
   ls $folder/Iteration$Iteration -all -sh --color=auto
fi
