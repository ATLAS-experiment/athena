#!/bin/bash

export ATHENA_CORE_NUMBER=16

inputEVNT=$1
outputHITS=$2
physlist=$3
nevents=$4
if test -z "$nevents"
then
  nevents=5000
fi  
skip=0

echo inputEVNT=$inputEVNT
echo simulate $nevents events

outfile_job=$outputHITS
rundir="rundir_"$(basename $inputEVNT)
echo outputHITS=$outfile_job, rundir=$rundir
mkdir -p $rundir
cd $rundir

Sim_tf.py \
--CA \
--multithreaded \
--conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-04' \
--physicsList "$physlist" \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoVCalibrationHits,SimulationConfig.disablePhotonRussianRoulette,SimulationConfig.disableNeutronRussianRoulette,SimulationConfig.disableFrozenShowersFCalOnly' \
--geometryVersion 'default:ATLAS-R3S-2021-03-02-00' \
--inputEVNTFile "$inputEVNT" \
--outputHITSFile "$outfile_job" \
--maxEvents $nevents \
--skipEvent $skip \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

cd ..


