#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh

# Write here the name and path of your program and database
DIR_P2=$HOME/PAV/P2
DB=$DIR_P2/db.v4
CMD="$DIR_P2/bin/vad"

if [ $# -eq 1 ]
  then
    CMD="$DIR_P2/bin/vad --alfa1=$1"
fi
if [ $# -eq 2 ]
  then
    CMD="$DIR_P2/bin/vad --alfa1=$1 --alfa2=$2"
fi
if [ $# -eq 3 ]
  then
    CMD="$DIR_P2/bin/vad --alfa1=$1 --alfa2=$2 --N_tramas_ini=$3"
fi

for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}

    $CMD -i $filewav -o $filevad || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD $filewav $filevad $filewavOut || exit 1

done

scripts/vad_evaluation.pl $DB/*/*lab

exit 0
