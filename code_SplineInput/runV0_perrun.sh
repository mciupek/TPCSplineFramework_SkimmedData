#!/bin/bash
runlist=/lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/inputRunList/LHC15o_pass2_full.txt
runFolder=/lustre/nyx/alice/users/mciupek/output_test

while read run; do
  mkdir -p $runFolder/data/2015/LHC15o/$run
  cd $runFolder/data/2015/LHC15o/$run
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2015/LHC15o/$run/pass2/chunks_*/V0tree.root > V0_LHC15o_$run.list
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2015/LHC15o/$run/pass2/chunks_*/CleanTrack.root > track_LHC15o_$run.list

  V0list="$runFolder/data/2015/LHC15o/$run/V0_LHC15o_$run.list"
  tracklist="$runFolder/data/2015/LHC15o/$run/track_LHC15o_$run.list"
  
  singularity exec /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500/alidockSingularity5.sif aliroot -l -b <<EOF
  .L /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/code_SplineInput/runV0.C
  runV0("$V0list","$tracklist",kFALSE,kTRUE);
  .qqqqqq
EOF

done < $runlist

#cd /lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/code_SplineInput_changed/
#singAliroot
#hadd -f TPCresidualPID.root $runFolder/data/2018/LHC18q/*/TPCresidualPID.root 
#hadd -f TPCPIDEtaTree.root $runFolder/data/2018/LHC18q/*/TPCPIDEtaTree.root 