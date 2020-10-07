OpenFileWithPDFReader() {
  if [ -f $1 ];then
    evince $1
  else
    echo "File $1 not found"
  fi
}

if [ "$ALICE_PHYSICS" = "" ];then
  if [ ! -e $splinepath/env.sh ];then
    "Please create $splinepath/env.sh and source in there the Aliroot environment"
    exit
  fi
  source $splinepath/env.sh
fi

if [[ $1 = OPEN ]];then
  OpenFileWithPDFReader $2
elif [[ $1 = LoopThroughString ]];then
  MultSeparationString=$2
  if [[ -z $MultSeparationString ]];then
    com=$4
    eval $com
  else
    com=$3
    MultLow=$(cut -d':' -f1 <<< "$MultSeparationString")
    i=1
    while [ "$MultLow" != "" ]
    do
      MultHigh=$(cut -d':' -f$(expr $i + 1) <<< "$MultSeparationString")
      eval $com
      i=$(expr $i + 2)
      MultLow=$(cut -d':' -f$i <<< "$MultSeparationString")
    done
  fi
else
  echo "Argument for HelpFunctions.sh not known"
fi