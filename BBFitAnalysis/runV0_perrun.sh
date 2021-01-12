#!/bin/bash
runlist=/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/inputRunList/LHC18q_pass3_full.txt
runFolder=/lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/BBFitAnalysis/output_SkimmedData_fixed

while read run; do
  mkdir -p $runFolder/data/2018/LHC18q/$run
  cd $runFolder/data/2018/LHC18q/$run
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output2/data/2018/LHC18q/$run/pass3/chunks_*/V0tree.root > V0_LHC18q_$run.list
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output2/data/2018/LHC18q/$run/pass3/chunks_*/CleanTrack.root > track_LHC18q_$run.list

  V0list="$runFolder/data/2018/LHC18q/$run/V0_LHC18q_$run.list"
  tracklist="$runFolder/data/2018/LHC18q/$run/track_LHC18q_$run.list"
  
singularity shell /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500/alidockSingularity.sif <<EOF
alienv -w /alicesw/sw enter AliPhysics/latest
aliroot
  .L /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/BBFitAnalysis/runV0.C
  runV0("$V0list","$tracklist",kFALSE);
  .qqqqqq
EOF

done < $runlist

cd /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/BBFitAnalysis
singAliroot
hadd -f TPCTree_BBFitAnalysis.root $runFolder/data/2018/LHC18q/*/TPCTree_BBFitAnalysis.root
