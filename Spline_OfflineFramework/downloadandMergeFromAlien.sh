#!/bin/bash

alienusername=scmartin
# folder=/lustre/nyx/alice/users/maschmid/Splines/LHC15o_pass4_lowIR_pidfix_cookdedx
folder=$(pwd)/LHC18r_pass1 #LHC15o_pass4_lowIR_pidfix_cookdedx #LHC17k_pass1
year=2018
period=LHC18r
reconstructionpass=LHC18r_pass1 #pass4_lowIR_pidfix_cookdedx
trainname=856_20190121-0120/merge
macropath=/lustre/nyx/alice/users/maschmid/macros
# macropath=~/Documents/Splines/macros_for_trainfiles
runfolderprefix=

# source /lustre/nyx/alice/users/maschmid/software/envMaster
##source ~/usr/bin/LoadAliRootEnv.sh
downloadfilesfromstage() 
{
  for ((i=1;i<=$4;i+=1))
  do
    order=$(printf '%03d' $i)
    mkdir $3/$order
    alien_cp alien:$1/$2/$order/root_archive.zip file:$3/$order
    unzip  -o $3/$order/root_archive.zip -d $3/$order
  done
  for FILE in $3/001/*.root
  do
    aliroot "$macropath/recursiveMerging.C+(\"$3/*/$(basename $FILE)\", \"$3/$(basename $FILE .root)\", 4)" -b -q
  done
}

##if [ "$1" = "newtoken" ];then
##  alien-token-init $alienusername
##fi
for runfolder in $folder/$runfolderprefix*
do
echo $runfolder
  #Download root_archive from alien run folder to this folder
  alienfolder=/alice/data/$year/$period/000$(basename $runfolder)/$reconstructionpass/PWGPP/AdHocCalibration/$trainname
  alien_cp alien:$alienfolder/root_archive.zip file:$runfolder
  echo $alienfolder
  if [ -e $runfolder/root_archive.zip ];then
    echo "Download complete. Begin unzipping"
    unzip  -o $runfolder/root_archive.zip -d $runfolder
    echo "Unzipping completed"    
  else
    echo "No Root Archive found. Search next stage"
    for stage in Stage_5 Stage_4 Stage_3 Stage_2 Stage_1
    do
      output=$(eval "alien_ls $alienfolder/$stage")
      lines=$(eval "alien_ls $alienfolder/$stage | wc -l")
      if [ "$output" == "${output/"no such file"/}" ];then
        downloadfilesfromstage $alienfolder $stage $runfolder $lines
        break;
      fi  
    done
  fi
done

for FILE in $runfolder/*.root
do
  aliroot "$macropath/recursiveMerging.C+(\"$folder/$runfolderprefix*/$(basename $FILE)\", \"$folder/$(basename $FILE .root)\", 4)" -b -q    
done
echo "Merging complete. Delete downloaded files? (Y/N)"
read ANSWER
if [ "$ANSWER" = "Y" -o "$ANSWER" = "y" ];then
  for runfolder in $folder/$runfolderprefix*
  do
    rm -r $runfolder/*
  done
fi  
