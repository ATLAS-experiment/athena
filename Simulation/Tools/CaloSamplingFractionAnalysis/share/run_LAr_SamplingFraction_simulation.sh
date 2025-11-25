#!/bin/bash

if test -z "$ATHENA_CORE_NUMBER"
then
  export ATHENA_CORE_NUMBER=8
fi

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

geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN3)")
conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")
echo "Run simulation with geometry: $geometry , conditions: $conditions"

Sim_tf.py \
--CA \
--multithreaded \
--conditionsTag "default:${conditions}" \
--geometryVersion "default:${geometry}" \
--physicsList "$physlist" \
--simulator 'FullG4MT_QS' \
--postInclude 'PyJobTransforms.TransformUtils.UseFrontier' \
--preInclude 'EVNTtoHITS:Campaigns.MC23SimulationSingleIoVCalibrationHits,SimulationConfig.disablePhotonRussianRoulette,SimulationConfig.disableNeutronRussianRoulette,SimulationConfig.disableFrozenShowersFCalOnly' \
--inputEVNTFile "$inputEVNT" \
--outputHITSFile "$outfile_job" \
--maxEvents $nevents \
--skipEvent $skip \
--postExec 'with open("ConfigSimCA.pkl", "wb") as f: cfg.store(f)' \
--imf False

#To run with the new EMEC geometry, use 
#--preExec "flags.dump('GeoModel');flags.GeoModel.EMECStandard=True;flags.dump('GeoModel')" \
#This should eventually move into a dedicated ART test

status=$?
#echo  "art-result: $status Simulation"

cd ..

exit $status

