#!/bin/bash
#title           :submit_one_run.sh
#description     :This script will submit the job for each run
#author          :Xiaozhi Bai xiaozhi.bai@cern.ch  x.bai@gsi.de
#date            :Fri May  8 12:18:32 EDT 2020
#version         :1.0



base_path=${1}
data=${2}
year=${3}
run_pariod=${4}
run=${5}
pass=${6}
prod_id=${7}
file_name=${8}
output_base_dir=${9}
current_dir=${10}

max_chunks=${11}

if [ "${data}"x = "data"x ]
then
    runPrefix="000"
elif [ "${data}"x = "sim"x ]
then
    runPrefix=""
fi

#echo "input files path: " $base_path/$data/$year/$run_pariod/
#echo "run number "$runPrefix$run
#echo "input pass "$pass
#echo "input production id" $prod_id
#echo "input file name"=$file_name
#echo "output dir path" $output_base_dir

output_dir=${output_base_dir}/$data/$year/$run_pariod/$run/$pass

mkdir -p $output_dir
[[ ! -d $output_dir ]] && echo "output dir not exist" && exit 0; 
ls ${base_path}/${data}/${year}/${run_pariod}/$runPrefix${run}/${pass}/${prod_id}/*/${file_name} > $output_dir/filtered.list

input_file_path=${base_path}/${data}/${year}/${run_pariod}/$runPrefix${run}/${pass}/${prod_id}/

echo $input_file_path

nChunks=`cat $output_dir/filtered.list | wc -l`



[[ ${nChunks} -gt $max_chunks ]] && echo "number of chunks" echo $nChunks

echo $split_jobs

if [ ${nChunks} -gt $max_chunks ]
then
#    echo "need to split "
    number_of_split_jobs=`expr $((nChunks/$max_chunks))`;
 #   echo "number_of_split_jobs "$number_of_split_jobs
    additinal_one_job=`expr $nChunks % $max_chunks`;
  #  echo " addtinal "$additinal_one_job
    [[ $additinal_one_job -ne 0 ]] && ((number_of_split_jobs=number_of_split_jobs+1)) 
   # echo "number_of_split_jobs final" $number_of_split_jobs
else
    number_of_split_jobs=1
fi


    mkdir -p $output_dir/

    cp $current_dir/submit_one_run.sh $output_dir/
    cp $current_dir/../code_FlatTreeCreation/fitdEdxCorrectionFiltered.C $output_dir/
    cd $output_dir

    sbatch --partition=main --array=0-${nChunks} $current_dir/SubmitJobs2Nyx.sh $run  ${output_dir} ${input_file_path} 3 9 

#--exclude=lxbk[0600-0723] 
echo "XXXX" $output_dir  
echo "YYY" $input_file_path

echo "CCCC" $nChunks_each
exit;


c=1;
ZERO=0
start_line=$max_chunks;
end_line=$max_chunks
while [ $c -le $number_of_split_jobs ]
do

   

    if [[ $number_of_split_jobs -eq $c  && $additinal_one_job -ne $ZERO ]] 
    then
       head -n  $start_line $output_dir/filtered.list | tail -$additinal_one_job >  $output_dir/"chunks_"$c/filtered.list
    else
	head -n  $start_line $output_dir/filtered.list | tail -$max_chunks >  $output_dir/"chunks_"$c/filtered.list
    fi
    nChunks_each=`cat $output_dir/"chunks_"$c/filtered.list | wc -l`
    cd $output_dir/"chunks_"$c
#    sbatch  $current_dir/SubmitJobs2Nyx.sh $run $nChunks_each > $output_dir/"chunks_"$c/worker$i.out    
#    sbatch --array=0-3    -N1  $current_dir/SubmitJobs2Nyx.sh $run $nChunks_each > $output_dir/"chunks_"$c/worker$i.out    

    ((c=c+1))
    ((start_line=start_line+max_chunks))

done












#echo $run $nChunks


echo "submit is done for run :" $run

