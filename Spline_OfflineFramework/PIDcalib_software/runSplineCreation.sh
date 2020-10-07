#! /bin/bash
scriptname=$software/splineCreationScript.sh
echo "Run runSplinecreation.sh"

runthrough=1
scriptcommand=0
download=1
final=0
noChecks=0
doSomething=1
checkPlots=1
exSingleCommand=0
while [ -n "$1" ]
do
  case "$1" in
    -q | no[cC]hecks) checkPlots=0; noChecks=1;;
    -QA | showQAPlots) doSomething=0; download=0;;
    no[dD]ownload) download=0;;
    -no[dD]ownload) download=0;;
    [cC]ontinue[cR]un) runthrough=0; scriptcommand=$2; download=0;shift;;
    [fF]inal) final=1;;
    [sS]ys | [sS]ystematics) systematics=1;;
    *) exSingleCommand=1;runthrough=0;scriptcommand=$@;;
  esac
  shift
done 

if [[ $runthrough = 0 && "$scriptcommand" = "" ]];then
  echo "Give starting point as second parameter after ContinueRun"
  exit 0
fi

if [ "$basefolder" = "" ];then
  if [[ "$folder" = *"Iteration"* ]];then
    basefolder=$(dirname $folder)
  else
    basefolder=$folder
  fi
fi


if [[ $basefolder != $folder ]];then
  echo "Warning: folder is not equal to basefolder. Intended? (Y/N)"
  read ANSWER
  if [[ ($ANSWER = N || $ANSWER = n) || $noChecks = 1 ]];then
    echo "Exit"
    exit 
  fi    
fi

if [ "$ALICE_PHYSICS" = "" ];then
  if [ ! -e $splinepath/env.sh ];then
    "Please create $splinepath/env.sh and source in there the Aliroot environment"
    exit
  fi
  source $splinepath/env.sh
fi

hasMult=kFALSE
useMultCorrection=kFALSE
etaTreeNameComplete=$etaTreeName
etaTreeNameMultSelected=$etaTreeName
multParameters=
if [ "$collsys" = "PBPB" -o "$collsys" = "PPB" ];then
  etaTreeNameMultSelected=$etaTreeNameMultSplitted
  hasMult=kTRUE
  if [ "$MultSeparationString" = "" ];then
    useMultCorrection=kTRUE
    if [ ! -e $basefolder/*multiplicityDependence* ];then
      source $scriptname "mult"
    fi
    multCorrFile=$(ls -rt1 $basefolder/*multiplicityDependence* 2>/dev/null | tail -1)
    if [ "$multCorrFile" != "" ];then
      aliroot "$trainmacros/GetMultiplicityFactors.C(\"$multCorrFile\", $useSigmaMultCorrection, $multtanThetaBin, $multpBin, $useNewMultParametrization)" -b -q -l     
    fi
    multParameters=$(cat $basefolder/multParameters.txt)  
  fi
fi

if [ "$BBFunction" = "SaturatedLund" -o "$BBFunction" = "" ];then
  BBFunctionNumber=2;
elif [ "$BBFunction" = "ALEPH" ];then
  BBFunctionNumber=0; 
elif [ "$BBFunction" = "ModifiedALEPH" ];then
  BBFunctionNumber=3; 
elif [ "$BBFunction" = "Lund" ];then
  BBFunctionNumber=1;   
fi

if [[ $exSingleCommand = 1 ]];then
  source $scriptname $scriptcommand
  exit
fi

if [[ $download = 1 ]];then
  echo "Download files (Y/any other)?"
  read ANSWER
  if [[ ("$ANSWER" = "Y" || "$ANSWER" = "y") || $noChecks = 1 ]];then
    source $scriptname download newtoken
  fi  
fi  
if [[ $final = 1 ]];then
  if [ "$QAIteration" = "" ];then
    echo "Doing final QA, creating spline comparison, outputCheckShapeEtaAdv and PIDqaReport. Using Iteration $Iteration. Continue?"
    read ANSWER
    if [[ ("$ANSWER" = "Y" || "$ANSWER" = "y") || $noChecks = 1 ]];then  
      QAIteration=$Iteration
    else
      exit 0
    fi
  else
    echo "Doing final QA, creating spline comparison, outputCheckShapeEtaAdv and PIDqaReport. Using Iteration $QAIteration"
  fi
  mv $folder/AnalysisResults.root $folder/Iteration$QAIteration
  mv $folder/TPCPIDEtaQA.root $folder/Iteration$QAIteration
  folder=$folder/Iteration$QAIteration
#   source $scriptname 0 
#   source $scriptname comp
  source $scriptname QA2
  source $scriptname QA1
  echo "Check $(basename $(ls -rt1 $folder/outputCheckShapeEtaAdv* | tail -1)) and $(basename $(ls -rt1 $folder/PIDqaReport.pdf | tail -1)). Upload splines, outputCheckShapeEtaTree* files and QA plots in $folder to alien (Y/N)?"
  bash $software/HelpFunctions.sh "OPEN" $folder/PIDqaReport.pdf
  source $scriptname GetQAEtaAdv
  echo "Now type your answer!"
  read ANSWER
  if [[ ("$ANSWER" = "Y" || "$ANSWER" = "y") || $noChecks = 1 ]];then
    source $scriptname lambda > $basefolder/lambdaParameter.txt
    cp $basefolder/multParameters.txt $folder
    echo "You can use the extracted Lambda-Parameter in $folder/lambdaParameter.txt for the asymmetric Multi Template Fit."
    echo "Upload splines"
    source $scriptname uploadsplines
  fi
  echo "Producing Systematics (Y/N)?"
  read ANSWER
  if [[ ("$ANSWER" = "Y" || "$ANSWER" = "y") || $noChecks = 1 ]];then
    echo "Producing systematics"
    cd $folder
    source $scriptname bbsystematics
    source $scriptname etasystematics
    source $scriptname ressystematics
#     source $scriptname uploadsystematics
    cd ..
  fi
  exit 0
fi

if [[ $systematics = 1 ]];then
  echo "Producing systematics"
  cd $folder
  source $scriptname bbsystematics
  source $scriptname etasystematics
  source $scriptname ressystematics
#   source $scriptname uploadsystematics
  cd ..
fi

if [ -d $folder/Iteration$Iteration ];then
  echo "Folder Iteration$Iteration found, processing Iteration $Iteration anyway (Y/N)"
  read ANSWER
  if [[ ! (("$ANSWER" = "Y" || "$ANSWER" = "y") || $noChecks = 1) ]];then
    echo "Break"
    exit 0
  fi
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = 0) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 0 
    if [[ "$MultSeparationString" != "" ]];then
      aliroot "$trainmacros/MakeTPCPIDResponseOADB.C+(\"$folder/TPCPIDResponseOADB_temp.root\", \"""\", \"$folder/$spline2\", 1, 1, 0, \"\", \"$MultSeparationString\", \"$pileupDefinition\")" -b -q -l
    fi
  fi
  if [[ $checkPlots = 1 ]];then
    cnt=`ls $folder/bethebloch"_"$BBFunction* 2>/dev/null | wc -l`
    if [[ $cnt = 0 ]];then
      echo "File $folder/bethebloch_"$BBFunction".pdf not found, please check your code or files!"
      exit 0
    fi
    echo "Check if bethebloch QA plots look good. (Y/N)"
    if [ "$collsys" = "PBPB" -o "$collsys" = "PPB" ];then 
      bash $software/HelpFunctions.sh "LoopThroughString" "$MultSeparationString" "OpenFileWithPDFReader $folder/bethebloch_$BBFunction\"_\"\$MultLow\"_\"\$MultHigh.pdf; OpenFileWithPDFReader $folder/betheblochresidual_$BBFunction\"_\"\$MultLow\"_\"\$MultHigh.pdf" "OpenFileWithPDFReader $folder/bethebloch_$BBFunction.pdf; OpenFileWithPDFReader $folder/betheblochresidual_$BBFunction.pdf"
    else
      bash $software/HelpFunctions.sh "OPEN" $folder/bethebloch_$BBFunction.pdf
      bash $software/HelpFunctions.sh "OPEN" $folder/betheblochresidual_$BBFunction.pdf
    fi
    echo "Give answer (Y/N)"
    read ANSWER
    if [ "$ANSWER" = "N" -o "$ANSWER" = "n" ];then
      echo "Break"
      exit 0
    fi
    else    
    echo "Continue"
  fi
  if [ "$MultSeparationString" = "" -a "$collsys" = "PPB" -o "$collsys" = "PBPB" ];then
    if [ ! -e $folder/$(basename $etaTreeName .root)""_multiplicityDependence* ];then
      source $scriptname mult     
    fi
  fi 
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = comp) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname comp 
  fi
  echo "Comparing new Splines with old splines."
  if [[ $Iteration != 1 && $checkPlots = 1 ]];then
#     if [ "$collsys" = "PBPB" -o "$collsys" = "PPB" ];then 
#       bash $software/HelpFunctions.sh "LoopThroughString" $MultSeparationString "OpenFileWithPDFReader $folder/CompareSplines$perpass""it$(expr $Iteration - 1)it$Iteration"_""\"$MultLow\"""_""\"$MultHigh\"".pdf"
#     else
      bash $software/HelpFunctions.sh "OPEN" $folder/CompareSplines$perpass""it$(expr $Iteration - 1)it$Iteration.pdf
#     fi 
    echo "Check If splines are sufficiently consistent with old splines. If so, stop and do final QA. Continue with production (Y/N)?"
    read ANSWER
    if [ "$ANSWER" = "N" -o "$ANSWER" = "n" ];then
      echo "Break"
      exit 0
    fi
  fi
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = 1) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 1 
  fi
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = 2) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 2 
  fi 
  if [[ $checkPlots = 1 ]];then
    echo "Check the plot of hPureResults shown in the canvas. There should be no kinks or strong asymmetries visible and the map should look rather smooth. Otherwise reduce the resolution of the map (if you are limited by statistics) (-q for leaving aliroot followed by Y/N)" 
    aliroot "$software/GetSplineQAPlots.C(\"$(ls -rt1 $folder/outputCheckShapeEtaTree* | grep -v CorrectedWithMap | tail -1)\")" -l
    echo "Now type decision"
    read ANSWER
    if [ "$ANSWER" = "N" -o "$ANSWER" = "n" ];then
      echo "Break"
      exit 0
    fi
    echo "Continue"
  fi
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = 3) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 3 
  fi
fi
if [[ $runthrough = 1 || ($runthrough = 0 && $scriptcommand = 4) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 4 
  fi
  if [[ $checkPlots = 1 ]];then
    echo "Check the plot of hPureResults shown in the canvas. There should be no kinks or strong asymmetries visible and the map should look rather smooth. Otherwise reduce the resolution of the map (if you are limited by statistics) (-q for leaving aliroot followed by Y/N)" 
    aliroot "$software/GetSplineQAPlots.C(\"$(ls -rt1 $folder/outputCheckShapeEtaTree* | grep CorrectedWithMap | tail -1)\")" -l 
    echo "Now type decision"
    read ANSWER
    if [ "$ANSWER" = "N" -o "$ANSWER" = "n" ];then
      echo "Break"
      exit 0
    fi
    echo "Continue"
  fi
fi 
ANSWER=n
if [[ $runthrough = 1 || ($runthrough = 0 && ($scriptcommand = pull || $scriptcommand = 5)) ]];then
  runthrough=1
  if [[ $doSomething = 1 ]];then
    source $scriptname 5 
  fi
  if [[ $checkPlots = 1 ]];then
    echo "Check Pull Plots. Creating OADB files and etaMaps (Y/N)?"
    aliroot "$software/GetSplineQAPlots.C(\"$(ls -rt1 $folder/*checkPullSigma* | tail -1)\")" -l
    echo "Now type decision"
    read ANSWER
  fi
fi  
if [[ $runthrough = 0 && ($scriptcommand = OADB || $scriptcommand = etaMaps) ]];then
  ANSWER=y 
fi  
if [[ ($ANSWER = y || $ANSWER = Y) || $checkPlots = 1 ]];then
  echo "Creating OADB and eta Maps."
  source $scriptname OADB
  source $scriptname etaMaps 
  source $scriptname upload newtoken
  echo "Cleanup files"
  source $scriptname cu
fi
