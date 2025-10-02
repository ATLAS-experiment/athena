#!/bin/bash

if test -z "$ATHENA_CORE_NUMBER"
then
  if test -n "$CMAKE_BUILD_PARALLEL_LEVEL"
  then
    export ATHENA_CORE_NUMBER=$CMAKE_BUILD_PARALLEL_LEVEL
  else
    export ATHENA_CORE_NUMBER=16
  fi
fi

if test -z "$inputdir"
then
  #Input files originally copied from inputdir=/eos/atlas/atlascerngroupdisk/proj-simul/G4Run3/SamplingFractions to CVMFS
  export inputdir=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CaloSamplingFractionAnalysis
fi

if test -z "$G4version"
then
  if test -n "$G4VERS"
  then
    export G4version=$G4VERS
  else
    export G4version=11.3
  fi
fi

if test -z "$PhysList"
then
  export PhysList=FTFP_BERT_ATL
fi

neventsbase=$1
if test -z "$neventsbase"
then
  neventsbase=5000
fi
neventsLArEM=$(( 8*neventsbase ))

echo "Number of cores to run on: $ATHENA_CORE_NUMBER"
echo "Location single particle EVNT input files: $inputdir"
echo "Geant4 version: $G4version"
echo "Geant4 physics list: $PhysList"
echo "Number of events to simulate: 2*$neventsLArEM for LAr EM, 6*$neventsbase for HEC, and 3*$neventsbase for FCal"

export resultdir=$PWD/"$G4version"-$PhysList
mkdir -p $resultdir
ln -s $resultdir mcoutput

##############################################
############## LAr EM ########################
##############################################
if [ 1 -eq 1 ]
then
  ### Run LAr EM simulation
  for file in $inputdir/LArEM/mc.PG_pid11_Mom50000_*.EVNT.pool.root
  do 
    outfile=$resultdir/$(basename $file)
    run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList $neventsLArEM
    status=$?
    echo  "art-result: $status LAr EM Simulation"
  done

  ### Run LAr EM ntuples
  athena.py --filesInput="$resultdir/mc.PG_pid11_Mom50000_Radius1500000*.HITS.pool.root" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py
  echo  "art-result: $? LAr EM ntuples"
  mv LArEM_SF.root $resultdir/LArEM_SF_barrel.root

  athena.py --filesInput="$resultdir/mc.PG_pid11_Mom50000_Z37*.HITS.pool.root" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py
  echo  "art-result: $? LAr EM ntuples"
  mv LArEM_SF.root $resultdir/LArEM_SF_endcap.root
fi

### Run LAr EM analysis
get_files LarEMSamplingFraction_analysis.C
root -b -q 'LarEMSamplingFraction_analysis.C("'$resultdir/LArEM_SF_barrel.root'","'$resultdir/LArEM_SF_endcap.root'")'
echo  "art-result: $? LAr EM sampling fractions"
cp SF_LAr_barrel.* SF_LAr_endcap.* $resultdir/
cp ELAr_hit_barrel.* ELAr_hit_endcap.* $resultdir/

##############################################
################# HEC ########################
##############################################
if [ 1 -eq 1 ]
then
  ### Run HEC simulation
  for file in $inputdir/HEC/mc.PG_pid*_Mom100000_*EVNT.pool.root
  do
    outfile=$resultdir/$(basename $file)
    run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList $neventsbase
    status=$?
    echo  "art-result: $status LAr HEC Simulation"
  done

  ### Run HEC ntuples
  for file in $resultdir/mc.PG_pid*Mom100000_Z[45]*HITS.pool.root
  do 
    echo $file
    athena.py --filesInput="$file" CaloSamplingFractionAnalysis/LArEMSamplingFractionConfig.py
    echo  "art-result: $? LAr HEC ntuples"
    a=$file;b=${a/Z4319500_bec_eta_150_330.HITS/HECfwh.NTUP};c=${b/Z5175000_bec_eta_160_330.HITS/HECrwh.NTUP}
    mv LArEM_SF.root $c
  done
fi

### Run HEC analysis
get_files HEC_SF_analysis
root -b -q HEC_SF_analysis/init.C 'HEC_SF_analysis/store_eta.C("'$G4version'","'$PhysList'","'$PWD'")' 'HEC_SF_analysis/get_SF.C("'$G4version'","'$PhysList'")'
echo  "art-result: $? LAr HEC sampling fractions"
cp $resultdir/SF_HEC*.pdf ./


##############################################
################# FCal #######################
##############################################
if [ 1 -eq 1 ]
then
  ### Run FCal simulation
  for file in $inputdir/FCal/mc.PG_pid11_Mom40000_*EVNT.pool.root
  do
    outfile=$resultdir/$(basename $file)
    run_LAr_SamplingFraction_simulation.sh $file ${outfile/EVNT.pool.root/HITS.pool.root} $PhysList $neventsbase
    status=$?
    echo  "art-result: $status LAr FCal Simulation"
  done
fi

### Run FCal ntuples and analysis
get_files LarFCalSamplingFraction_analysis.py

for file in $resultdir/mc.PG_pid*Mom40000_Z*_bec_eta_350_380.HITS.pool.root
do
  echo $file
  athena.py --filesInput="$file" CaloSamplingFractionAnalysis/LArFCalSamplingFractionConfig.py
  echo  "art-result: $? LAr FCal ntuples"
  a=$file;b1=${a/Z4713500_bec_eta_350_380.HITS.pool/fcal1.aan};b2=${b1/Z5173300_bec_eta_350_380.HITS.pool/fcal2.aan};b3=${b2/Z5647800_bec_eta_350_380.HITS.pool/fcal3.aan}
  mv LArFCal_SF.root $b3
  python LarFCalSamplingFraction_analysis.py $b3 -o ${b3/aan.root/txt}
  echo  "art-result: $? LAr FCal sampling fractions"
done

##############################################
################# Summary ####################
##############################################
echo "LAr barrel" > $resultdir/summary.txt
cat $resultdir/SF_LAr_barrel.txt >> $resultdir/summary.txt
echo "LAr endcap" >> $resultdir/summary.txt
cat $resultdir/SF_LAr_endcap.txt >> $resultdir/summary.txt
echo "HEC" >> $resultdir/summary.txt
cat $resultdir/HEC_SF.txt >> $resultdir/summary.txt
echo "FCal" >> $resultdir/summary.txt
cat $resultdir/mc.PG_pid11_Mom40000_fcal1.txt >> $resultdir/summary.txt
cat $resultdir/mc.PG_pid11_Mom40000_fcal2.txt >> $resultdir/summary.txt
cat $resultdir/mc.PG_pid11_Mom40000_fcal3.txt >> $resultdir/summary.txt

cat $resultdir/summary.txt

cp $resultdir/summary.txt ./

get_files PlotSamplingFractions.C

root -b -q 'PlotSamplingFractions.C("G4 10.6","'$inputdir/'SF_LAr_G4_10_6_ref.root","G4 11.3","SF_LAr.root")'
echo  "art-result: $? Comparison"

