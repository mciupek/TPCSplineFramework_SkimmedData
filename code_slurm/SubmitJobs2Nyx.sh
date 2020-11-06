#!/bin/bash
# Task name
#SBATCH -J TPCPID_test2
# Run time limit
#SBATCH --mem-per-cpu=16000
#SBATCH --time=08:00:00
# Working directory on shared storage
# Standard and error output in different files
#SBATCH -o %j_%N.out.log
#SBATCH -e %j_%N.err.log
#SBATCH --array=1-100:2
# Execute application code

#jobId=$1
#alisource=1
#if [ $alisource -eq 1 ];
#then source /lustre/nyx/alice/users/eperezl/alicesw/alice-env.sh -n 1
#else alikronos -n 1
#fi

#singularity exec /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500//alidockSingularity.sif aliroot -l -b <<EOF

echo "running on $(hostname)" 

echo "job_id: " ${SLURM_JOB_ID}
echo "array_job_id: " ${SLURM_ARRAY_JOB_ID}
echo "task_id: " ${SLURM_ARRAY_TASK_ID}

run=${1}
nChunks=${2}


output_dir=${2}
input_file_path=${3};

mkdir -p chunks_${SLURM_ARRAY_TASK_ID}



cd chunks_${SLURM_ARRAY_TASK_ID}




if [ ${SLURM_ARRAY_TASK_ID} -lt 10 ]
then
    ls $input_file_path/00${SLURM_ARRAY_TASK_ID}/FilterEvents_Trees.root > filtered.list
elif [ ${SLURM_ARRAY_TASK_ID} -lt 100 ] && [ ${SLURM_ARRAY_TASK_ID} -gt 10 ]
then
    ls $input_file_path/0${SLURM_ARRAY_TASK_ID}/FilterEvents_Trees.root > filtered.list
else
    ls $input_file_path/${SLURM_ARRAY_TASK_ID}/FilterEvents_Trees.root > filtered.list
fi


echo ${SLURM_ARRAY_TASK_ID} >> temp

echo "Today's date: "$today


echo $run
echo $nChunks
singularity shell /lustre/alice/users/miranov/NOTESData/alice-tpc-notes/JIRA/ATO-500/alidockSingularityNoInit.sif <<EOF
alienv -w /alicesw/sw enter AliPhysics/latest
aliroot
TStopwatch timer;
timer.Start();
.L /lustre/nyx/alice/users/mciupek/TPCSpline/SkimmedData_Framework/code_FlatTreeCreation/fitdEdxCorrectionFiltered_C.so
SetUpNewSpline($run);
InitTree(1,1,$run);
enablePileUpCorrection()
cacheCleanV0();
cacheCleanTrack();
cacheEventFlat();
timer.Print();
.qqqqq
EOF






