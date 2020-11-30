uploadfiletoalien() {
  FILE=$1
  Folder=$2
  echo "Uploading file $folder/$FILE"
  echo " ----------- "
  alien_cp $folder/$FILE alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$Folder/
  if [ "$(alien_ls /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$Folder/$(basename $FILE))" != "$(basename $FILE)" ];then
    "Upload of $FILE failed, retry."
    alien_cp $folder/$FILE alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$Folder/
    if [ "$(alien_ls /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$Folder/$(basename $FILE))" != "$(basename $FILE)" ];then
      "Upload of $FILE failed again. No automatic retry."
      uploadfailed=1
    fi
  fi  
}

#### All Variables are to be declared in the script in the appropriate folder ####
if [ "$ALICE_PHYSICS" = "" ];then
  source ~/usr/bin/LoadAliRootEnv.sh
fi
########## Download Files from alien ################
if [ "$1" = "download" ];then
   echo "This will download and extract files from alien to $folder. Extracting will overwrite existing files"
   if [ "$2" = "newtoken" ];then  
      alien-token-init $alienusername
   fi
   alien_cp alien:/alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/root_archive.zip file:$folder
   if [ -e $folder/root_archive.zip ];then
      echo "Download complete. Begin unzipping"
      unzip  -o $folder/root_archive.zip -d $folder
      echo "Unzipping completed"   
   else
      echo "No root_archive.zip found. Download all *.root files from alien:/alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/"
      alien_cp alien:/alice/cern.ch/user/a/alitrain/PWGPP/AdHocCalibration/$trainName/*.root file:$folder   
   fi 
########## STEP 0 ####################################
elif [ "$1" = "0" ]; then
echo "****** Step 0: Extract Bethe-Bloch Curve ****** "
  if [  "$collsys" = "PP" -o "$collsys" = "PBPB" ]; then
    if [ ! -e $folder/extract.sh ]; then
      echo "Copy extract.sh and extract.C"
      cp $software/extract.sh $folder/
      cp $software/extract.C $folder/
    fi
  elif [ "$collsys" = "PPB" ]; then
    if [ ! -e $folder/extractpPb.sh ]; then
      echo "Copy extractpPb.sh and extractpPb.C"
      cp $software/extractpPb.sh $folder/
      cp $software/extractpPb.C $folder/
    fi  
  fi  
  if [ "$collsys" = "PP" ]; then
    bash $folder/extract.sh $folder/TPCresidualPID.root $spline2 DATA $period $pass $collsys $folder
  elif [ "$collsys" = "PPB" ]; then
    bash $folder/extractpPb.sh $folder/TPCresidualPID.root $spline2 DATA $period $pass $collsys $folder
  elif [ "$collsys" = "PBPB" ]; then
    bash $folder/extract.sh $folder/TPCresidualPID.root $spline2 DATA $period $pass $collsys $folder
  fi
  if [ ! -e $folder/$etaTreeName ];then
    mv $folder/TPCPIDEtaTree.root $folder/$etaTreeName
  fi
  if [ "$collsys" = "PPB" -o "$collsys" = "PBPB" ];then
    if [ ! -e $folder/$(basename $etaTreeName .root)""_mult0* ];then
      echo "Split Tree in multiplicity bins"
      aliroot  "$software/splitPbPbTreeMultiplicity.C+(\"$folder\",kFALSE,$stepSizeMult,$maxMult)" -b -q -l
    fi
  fi    
echo "****** Step 0 finished *****"  
########## STEP 1 ##########
elif [ $1 = "1" ]; then
echo "****** Step 1 ****** "
   if [ $collsys = "PPB" ]; then
      aliroot  "$software/correctShapeEtaTree.C+(kFALSE, \"\", \"\", \"\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kTRUE, kFALSE, \"$etaTreeNameMultSplitted\")" -b -q -l
   elif [ $collsys = "PP" ]; then
      aliroot "$software/correctShapeEtaTree.C+(kFALSE, \"\", \"\", \"\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kFALSE, kFALSE, \"$etaTreeName\")" -b -q -l
   elif [ $collsys = "PBPB" ]; then
      aliroot  "$software/correctShapeEtaTree.C+(kFALSE, \"\", \"\", \"\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kTRUE, kFALSE, \"$etaTreeNameMultSplitted\")" -b -q -l
   fi
echo "****** Step 1 finished ******"   
########## STEP 2 ##########
elif [ "$1" = "2" ];then
echo "****** Step 2 ****** "
   if [ "$2" != "nosearch" ];then 
      file1=$(basename $(ls -rt1 $folder/*PIDetaTree*NewSplines* | grep -v outputCheckShapeEta | tail -1))
      echo "Processing file $file1"
   fi   
   aliroot "$software/checkShapeEtaTree.C+(0x0, $binning, $binning, $TOFcut, $mergeBinThresh, $V0cut, $V0TOFcut, $maxDeviation1, kFALSE, 0, 60, \"$folder\", \"$file1\", kFALSE, \"TPCPIDResponse.root\", \"$spline\", 1, \"fTree\", \"$(basename $folder/$etaTreeName .root)\")" -b -q -l
echo "****** Step 2 finished ******"  
########## STEP 3 ##########
elif [ $1 = "3" ];then
  echo "****** Step 3 ****** "
   if [ "$2" != "nosearch" ];then 
      file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))
      echo "Processing file $file2"
   fi  
   if [ $collsys = "PPB" ]; then
      aliroot  "$software/correctShapeEtaTree.C+(kTRUE, \"$folder\", \"$file2\", \"NoNormalisation\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kTRUE, kFALSE, \"$etaTreeNameMultSplitted\")" -b -q -l
   # last boolean arguments set to kTRUE for pPb, they are "hasMultiplicity" which reads the multiplicity from the tree and "correctMult" which uses the determined multiplicity corrections
      ls $folder/bhess_PIDetaTree_mult0_499_multiplicityCorrected_CorrectedWithMap_*
   elif [ $collsys = "PP" ]; then
      aliroot  "$software/correctShapeEtaTree.C+(kTRUE, \"$folder\", \"$file2\", \"NoNormalisation\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kFALSE, kFALSE, \"$etaTreeName\")" -b -q -l
      ls $folder/bhess_PIDetaTree_CorrectedWithMap_*
        elif [ $collsys = "PBPB" ]; then
      aliroot  "$software/correctShapeEtaTree.C+(kTRUE, \"$folder\", \"$file2\", \"NoNormalisation\", kTRUE, \"$folder/$spline2\", \"$spline\", \"$folder\", kTRUE, kFALSE, \"$etaTreeNameMultSplitted\")" -b -q -l
      ls $folder/bhess_PIDetaTree_mult0_499_CorrectedWithMap_*
   fi
echo "****** Step 3 finished ******"  
########## STEP 4 ##########
elif [ $1 = "4" ];then
  echo "****** Step 4 ****** "
   if [ "$2" != "nosearch" ];then 
      file3=$(basename $(ls -rt1 $folder/*PIDetaTree*outputCheckShapeEtaTree* | tail -1))
      echo "Processing file $file3"
   fi  
   aliroot  "$software/checkShapeEtaTree.C+(0x0, $binning, $binning, $TOFcut, $mergeBinThresh, $V0cut, $V0TOFcut, $maxDeviation2, kTRUE, 0, 60, \"$folder\", \"$file3\", kFALSE, \"TPCPIDResponse.root\", \"$spline\", 1, \"fTree\", \"$(basename $etaTreeName .root)\")" -b -q -l
echo "****** Step 4 finished ******"  
########## STEP 5 ##########
elif [ $1 = "5" ];then
echo "****** Step 5: Check Pull Tree ****** "
   if [ "$2" != "nosearch" ];then
      file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))
      echo "Processing file $file2"   
      file4=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1))
      echo "Processing file $file4"
   fi  
   if [ $collsys = "PP" ]; then
   aliroot  "$software/checkPullTree.C+(\"$folder\", \"$folder/$file2\",\"$folder/$file4\", \"NoNormalisation\", 0, kTRUE, 2, \"$folder/$spline2\", \"$spline\")" -b -q -l
   fi
   if [ $collsys = "PPB" ]; then
   aliroot  "$software/checkPullTree.C+(\"$folder\", \"$folder/$file2\",\"$folder/$file4\", \"NoNormalisation\", 1, kTRUE, 2, \"$folder/$spline2\", \"$spline\")" -b -q -l
   fi
        if [ $collsys = "PBPB" ]; then
   aliroot  "$software/checkPullTree.C+(\"$folder\", \"$folder/$file2\",\"$folder/$file4\", \"NoNormalisation\", 0, kTRUE, 2, \"$folder/$spline2\", \"$spline\")" -b -q -l
   fi
echo "****** Step 5 finished ******"  
########## QA STEP ##########
# Tasks 5 and 6 are only for the final QA
elif [ $1 = "QA1" ];then
   if [ $collsys = "PP" ]; then
      aliroot  "$software/checkShapeEta.C+(\"$folder\", -1,-1, 0, \"TPCPIDEtaQA.root\", \"TPCPIDEtaQA\")" -b -q -l
   elif [ $collsys = "PPB" ]||[ $collsys = "PBPB" ]; then
      aliroot  "$software/checkShapeEta2.C+(\"$folder\", $stepSizeMult,$maxMult, 0, \"TPCPIDEtaQA.root\", \"TPCPIDEtaQA\")" -b -q -l
   fi  
elif [ $1 = "QA2" ];then
   aliroot  "$software/MakePIDqaReport.C+(\"$folder/AnalysisResults.root\",\"$folder/PIDqaReport.pdf\", \"PIDqa\")" -b -q -l

########## MULTIPLICITY DEPENDENCE ##########

# EXTRACT MULTIPLICITY JUST ONCE AND DO NOT USE IT IN correctShapeEtaTree.C DURING SPLINE CREATION. CHECK ITS PARAMETERS WITH checkPullTree.C
elif [ $1 = "mult" ];then
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
   # par1 and par2 as par0 and par1 from tanThetaFits/tanThetaSlopes_pBin9, set par2 as the center of the pT bin of pBin9 -> 0.5
   # If something changes in getting the multiplicity factors, MakeTPCPIDResponseOADB.C must be changed!! 
echo "****** Step mult finished ******"     
###########     COMP SPLINES     ############
elif [ "$1" = "comp" ];then
  if [[ "$Iteration" = "1" ]];then
    echo "****** First Iteration, nothing to compare *******"
  else
    echo "****** Step comp: Compare with old Bethe-Bloch ****** "
    aliroot  "$software/compareSplines2.C(\"$prevfolder/$spline2\", \"$folder/$spline2\", \"$perpass\", \"DATA\", \"$collsys\", \"\", \"\", \"\", kFALSE, kFALSE, \"it$(expr $Iteration - 1)\", \"it$Iteration\", \"$folder\")" -b -q -l
    echo "****** Step comp finished ******"  
  fi  
###########     Make OADB Object - need to update MakeTPCPIDResponseOADB.C and addMapToFile.C before!! ############
elif [ $1 = "OADB" ];then
echo "****** Step OADB: Produce OADB File ****** "
  if [ "$firstRun" = "" ];then
    firstRun=0
    lastRun=0
  fi
  multCorrFile=$(ls -rt1 $folder/*multiplicityDependence* | tail -1)
  aliroot "$trainmacros/MakeTPCPIDResponseOADB.C+(\"$folder/$OADBFileName\", \"""\", \"$folder/$spline2\", $firstRun, $lastRun, $passNumber, \"$multCorrFile\")" -b -q -l 
  echo "****** Step OADB finished ******"  
###########     Make eta Maps - need to update addMapToFile.C before!! ############   
elif [ $1 = "etaMaps" ];then
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
   if [ "$2" = "newtoken" ];then  
      alien-token-init $alienusername
   fi
   if [ "$alienfolder" != "" ];then
    alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder
   fi
   for FILE in $OADBFileName etaMaps/$etaMapName 
   do
    uploadfiletoalien $FILE
   done        
   echo "Use the following string as 6th argument for PIDResponse_Devel"
   echo "TPC-OADB:alien:///alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$OADBFileName ;TPC-Maps:alien:///alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$alienfolder/$etaMapName"
   if [ "$uploadfailed" = "1" ];then
    echo "Attention: The upload failed!!"
    uploadfailed=0
   fi 
#    alien_cp $folder/Iteration$Iteration/$OADBFileName alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/
#    alien_cp $folder/Iteration$Iteration/etaMaps/$etaMapName alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/
echo "****** Step upload finished ******"  
elif [ "$1" = "uploadsplines" ];then
  echo "****** Step uploadsplines ****** "
   if [ "$2" = "newtoken" ];then  
      alien-token-init $alienusername
   fi
   file2=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1))    
   file4=$(basename $(ls -rt1 $folder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1)) 
   echo "Creating alien directory: /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$splinefolder"
   alien_mkdir /alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$splinefolder
   for FILE in $file2 $file4 $spline2 
   do
    uploadfiletoalien $FILE $splinefolder
   done    
#    echo "Uploading file $folder/$file4"  
#    echo " ----------- "
#    alien_cp $folder/$file4 alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$splinefolder/
#    echo "Uploading file $folder/$spline2" 
#    echo " ----------- "
#    alien_cp $folder/$spline2 alien:/alice/cern.ch/user/$(echo $alienusername | head -c 1)/$alienusername/$splinefolder/  
   if [ "$uploadfailed" = "1" ];then
     echo "Upload of at least one spline file failed."
     uploadfailed=0
   fi
   echo "****** Step uploadsplines finished ******"  
########### Extracting Lambda Parameter #####################   
elif [ "$1" = "lambda" -o "$1" = "Lambda" ];then
echo "***************** Extracting Lambda Parameter for MTF *********"
aliroot "$software/FitAsymmetricShape.C+(2.0,0.5,10,\"$folder\",\"TREE\",\"$etaTreeName\",kTRUE)" -l
########### Create own subdirectory for this iteration and move all files belonging to a specific iteration in this directory #####################   
elif [ "$1" = "cu" -o "$1" = "cleanup" ];then
   newfoldername=Iteration$Iteration
   mkdir $folder/$newfoldername
   mv $folder/*.pdf $folder/$newfoldername
   mv $folder/splines* $folder/$newfoldername
   mv $folder/*Splines* $folder/$newfoldername
   mv $folder/$OADBFileName $folder/$newfoldername
   mv $folder/etaMaps $folder/$newfoldername
   mv $folder/AnalysisResults.root $folder/$newfoldername
   mv $folder/*checkPullSigma* $folder/$newfoldername
   mv $folder/TPCresidualPID.root $folder/$newfoldername
   mv $folder/V0* $folder/$newfoldername
   mv $folder/EventStat_temp.root $folder/$newfoldername
   mv $folder/outputCheckShapeEtaAdv*.root $folder/$newfoldername
   mv $folder/TPCPIDEtaQA.root $folder/$newfoldername
   mv $folder/settingsForMapCreation.txt $folder/$newfoldername/etaMaps
   mv $folder/root_archive.zip $folder/$newfoldername
   echo "The following files have been moved to $newfoldername"
   ls $folder/$newfoldername -all -sh --color=auto
fi
