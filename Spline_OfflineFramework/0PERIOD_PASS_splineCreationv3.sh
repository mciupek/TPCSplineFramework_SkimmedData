# Declare the software folder containing the macros correctShapeEtaTree.C, checkShapeEtaTree.C and checkPullTree.C
# Macros can be obtained from $ALIPHYSICS_BASEDIR/master/inst/PWGPP/TPC/macros/PIDCalib
export splinepath=$(dirname $(pwd))
export software=$splinepath/SplineCreation/PIDcalib_software
trainmacros=$splinepath/macros_for_trainfiles
alienusername=tpcdrop #scmartin
alienfolder="calib/Splines"
trainName=710_20180108-1405/merge #385_20160622-1438/merge
export Iteration=1
QAIteration=
BBFunction=SaturatedLund #(ALEPH,ModifiedALEPH,Lund other possibilities) - leave it at SaturatedLund
# MultSeparationString=0:200:200:400:400:600
toLustre=0         #This works only if you have specified a hostname for lustre gsi_nyx in your ~/.ssh/config - give the name below to lustrehostName

# Adapt the following variables to your period, pass and collision system (be case sensitive!)
firstRun=274523 #195517
lastRun=274671 #195678
basefolder=$(pwd)
periodNumber=17j #13C
export period=LHC$periodNumber
passNumber=1 #4
export pass=pass$passNumber""
export collsys=PP #PPB
#spline2=splines_13c.pass4.root
export spline2=splines.root
# perpass=LHC16R_PASS1
export perpass=$period\_$pass
# spline=TSPLINE3_DATA_PROTON_LHC15O_PASS3_LOWIR_PIDFIX_PBPB_MEAN
spline=TSPLINE3_DATA_PROTON_$perpass""_$collsys""_MEAN
day=$(date +%Y_%m_%d)
# day=2017_04_26

export folder=$basefolder  #Change if you want to rerun a specific iteration already in its folder
export prevfolder=$folder/Iteration$(expr $Iteration - 1)

#splinefolder="Splines/$perpass"
splinefolder="$period.$pass" ##Jens' asking: folder name like LHC17a.pass1

#inputfile for pileup Correction
export pileupDefinition="alien:///alice/cern.ch/user/t/tpcdrop/calib/Splines/PilupCorrectionObjects/TPCdEdxCorr.LHC18r.pass1.root"

#Adapt the following variables to fit your desired trainfiles
OADBFileName=TPCPIDResponseOADB_$day""_$periodNumber""_pass$passNumber""_It$Iteration.root
etaMapName=TPCetaMaps_$day""_$periodNumber""_pass$passNumber""_It$Iteration.root
UsedFilesForMapName=UsedFilesForMap_$day""$periodNumber""_pass$passNumber""_It$Iteration.txt

#Normally: Leave the following variables as they are
normalisation=NoNormalisation

# Adjust the following variables depending on your available statistics or your needs
export useSigmaMultCorrection=kTRUE
export multtanThetaBin=2
export multpBin=9
useNewMultParametrization=kFALSE
multiplictyEstimator=AliTPCPIDResponse::kNTPCTrackBeforeClean
if [[ $collsys == PBPB ]];then
  useNewMultParametrization=kTRUE
fi
export useNewMultParametrization

multiplictyEstimator=AliTPCPIDResponse::kNTPCTrackBeforeClean
binning=2.5 # was 1.1
TOFcut=0.7
mergeBinThresh=0 # was 0
V0cut=3
V0TOFcut=5
maxDeviation1=0.04 #was 0.04
maxDeviation2=0.2

stepSizeMult=500
maxMult=2500
if [ "$collsys" = "PBPB" ]; then
  maxMult=25000
fi

etaTreeName=bhess_PIDetaTree.root
etaTreeNameMultSplitted=bhess_PIDetaTree_mult0_$(expr $stepSizeMult - 1).root
# FOR pPb and PbPb ONLY!

# Change the filenames to your output files
#with 1.6 resolution
# After the first step you will get file 1, after step 2 you will get file 2, etc
 file1=_mult0_499_NewSplines___2017_02_02__14_56
 file2=outputCheckShapeEtaTree_2017_02_02__14_57__mult0_499_NewSplines___2017_02_02__14_56.root
 file3=_mult0_499_multiplicityCorrected_CorrectedWithMap_outputCheckShapeEtaTree_2017_02_02__14_57__mult0_499_NewSplines___2017_02_02__14_56___2017_02_02__14_59
 file4=outputCheckShapeEtaTree_2017_02_02__15_02__mult0_499_multiplicityCorrected_CorrectedWithMap_outputCheckShapeEtaTree_2017_02_02__14_57__mult0_499_NewSplines___2017_02_02__14_56___2017_02_02__14_59.root

gsiName=
lustrehostName=gsi_nyx   #Specified in ~/.ssh/config

source $software/runSplineCreation.sh $@    # Pass all arguments

##### Calling this script without parameters will circle through an complete iteration of the spline production, governed by the script $software/runSplineCreation.sh. Calling it with parameters will let runSplineCreation.sh hand over to a single call of $software/splineCreationScript.sh. Possible parameters are:
# download : Download Train files (from trainname) and unzip them, writing over existing files.
# upload : Upload OADB Files and etaMaps to the user directory (alienusername/alienfolder)
# uploadsplines: Upload splines and outputCheckShapeEtaTree* files (those used for the spline creation)
####### -> called with newtoken as second parameter, a new alien token will be created
# 0 : Creates BetheBlochFit. If not yet present, it will copy software/extract.C to folder and (for pA or AA) split the Tree in Multiplicities. If you give testParameters as an additional parameter, it will create a canvas with the parameters given in extract.C
# 1: Create file1
# 2: Create file2
# 3: Create file3
# 4: Create file4
# 5: Create Pull Plots
# -> Calling those with nosearch as second parameter will let the script take the above values for the files
# QA1: Produce PIdqaReport.pdf
# QA1: Produce outputCheckShapeEtaAdv*
# final: Producing both previous QA plot, uploading (if satisfied) necessary files to your alien spline folder. Specify the Iteration you have used for this QA in QAIteration (script will ask you though if you haven't)
# mult: Extract the multiplicityDependence
# comp: Compare the splines of folder to prevfolder
# OADB: Create OADB Files
# etaMaps: Create etaMaps (can also be called with nosearch, same effect)
# cu: Cleanup order: mkdir folder/Iteration$Iteration and move all files belonging to one iteration in this folder
