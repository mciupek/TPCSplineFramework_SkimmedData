#!/bin/bash
runlist=/lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/inputRunList/LHC18r_pass3_full.txt
runFolder=/lustre/nyx/alice/users/mciupek/TPCSpline/Tree_sigmaPara/LHC18r_pass3

while read run; do
  mkdir -p $runFolder/data/2018/LHC18r/$run/pass3/
  cd $runFolder/data/2018/LHC18r/$run/pass3/
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2018/LHC18r/$run/pass3/chunks_*/V0tree.root > V0_LHC18r_$run.list
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2018/LHC18r/$run/pass3/chunks_*/CleanTrack.root > track_LHC18r_$run.list

  V0list="$runFolder/data/2018/LHC18r/$run/pass3/V0_LHC18r_$run.list"
  tracklist="$runFolder/data/2018/LHC18r/$run/pass3/track_LHC18r_$run.list"
  
singularity shell /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500/alidockSingularity.sif <<EOF
alienv -w /alicesw/sw enter AliPhysics/latest
aliroot
  .L runV0.C
  runV0("$V0list","$tracklist",kFALSE);
  .qqqqqq
EOF

done < $runlist

#cd /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/BBFitAnalysis
#singAliroot
#hadd -f TPCTree_BBFitAnalysis.root $runFolder/data/2018/LHC18q/*/TPCTree_BBFitAnalysis.root
