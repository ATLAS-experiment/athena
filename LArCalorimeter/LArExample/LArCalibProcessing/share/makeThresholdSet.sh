#!/bin/zsh

if [[ $# > 0  ]]
then
 Run=$1
else 
 Run=500000
fi 
if [[ $# > 1 ]]
then
 Sqlite=$2
else 
 Sqlite=""
fi 
if [[ $# > 2 ]]
then
 pfold=$2
else 
 pfold=""
fi 
fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2"  /LAR/NoiseOfl/CellNoise | tail -1`
foreach i in `seq 0 7`
    if [[ $Sqlite != ""  &&  $pfold != "" ]]
    then
       python -m LArOnlDbPrep.LArDSPThresholdTopOptions -r $Run -s $Run -t $i -n  $fulltag --sqlite $Sqlite -a -p
    elif [[ $Sqlite != ""  && $pfold == "" ]]
    then
       python -m LArOnlDbPrep.LArDSPThresholdTopOptions -r $Run -s $Run -t $i -n  $fulltag --sqlite $Sqlite -a 
    else   
    python -m LArOnlDbPrep.LArDSPThresholdTopOptions -r $Run -s $Run -t $i -n  $fulltag
    fi
end

