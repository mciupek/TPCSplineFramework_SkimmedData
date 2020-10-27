#!/bin/bash 
#title           :submit_All_skimedData_ana.sh
#description     :This script will submit the job to GSI batch farm.
#author          :Xiaozhi Bai xiaozhi.bai@cern.ch  x.bai@gsi.de
#date            :Fri May  8 12:18:32 EDT 2020
#version         :1.0

#pass=pass2_CENT_woSDD   # pass1, pass2..
pass=pass3

MAX_runs=150 #99 for LHC18r, 136 for 18q, user can give a small number for testing 
MAX_chunks=1 # how many chunks per job, split the large runs with into several jobs,  

#run_pariod=LHC16q #LHC18q, LHC18r.
#run_list=pPb/LHC16q_pass2_wSSD.txt #LHC18r_pass3_full.txt

run_pariod=LHC18q
run_list=LHC18q_pass3_full.txt

#user can modify the below path according to the input 
input_base_path=/lustre/alice/DETdata/triggeredESD/alice 
data=data # data or sim
year=2018 # 2018. 2015

prod_id=AODFilterTrees01/AOD  #for PbPb 2018
#prod_id=AOD/
file_name=FilterEvents_Trees.root


output_base_dir=/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/output_test
current_dir=/lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/code_slurm
input_path=$input_base_path/$data/$year/$run_pariod

[ ! -d ${input_path} ] && echo "Input files dir is not exist!!! please check"  && return
[ ! -s ${output_base_dir} ] && echo "output can not wirite to this dir. !!! please check"  && return

run_counting=1;
exec 6>&1
exec 6<../inputRunList/$run_list

while read -u 6 runNumber
do
    echo "run index" $run_counting
    [[ run_counting -gt MAX_runs ]] && exit 0;
  ./submit_one_run.sh $input_base_path $data $year $run_pariod $runNumber $pass $prod_id $file_name $output_base_dir $current_dir $MAX_chunks
    echo 'submit_one_run.sh' $input_base_path $data $year $run_pariod $runNumber $pass $prod_id $file_name $output_base_dir $current_dir $MAX_chunks   
    ((run_counting=run_counting+1))
done

exit 0
