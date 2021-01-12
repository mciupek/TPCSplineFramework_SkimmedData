#!/bin/bash
runlist=/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/inputRunList/LHC18r_pass3_full.txt
runFolder=/lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework/code_Splineinput/output_Sparse_Tree/

while read run; do
  mkdir -p $runFolder/data/2018/LHC18q/$run
  cd $runFolder/data/2018/LHC18q/$run
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2018/LHC18r/$run/pass3/chunks_*/V0tree.root > V0_LHC18q_$run.list
  ls -1v /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/output/data/2018/LHC18r/$run/pass3/chunks_*/CleanTrack.root > track_LHC18q_$run.list

  V0list="$runFolder/data/2018/LHC18q/$run/V0_LHC18q_$run.list"
  tracklist="$runFolder/data/2018/LHC18q/$run/track_LHC18q_$run.list"
  
  singularity exec /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500//alidockSingularity.sif aliroot -l -b <<EOF
  .L /lustre/nyx/alice/users/mciupek/TPCSpline/SplineCreationFramework/TPCSplineFramework_SkimmedData/code_SplineInput/runV0.C
  runV0("$V0list","$tracklist",kFALSE);
  .qqqqqq
EOF

done < $runlist

#cd /lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/code_SplineInput_changed/
#singAliroot
#hadd -f $runFolder/data/2018/LHC18q/*/TPCresidualPID.root TPCresidualPID.root
#hadd -f $runFolder/data/2018/LHC18q/*/TPCPIDEtaTree.root TPCPIDEtaTree.root
