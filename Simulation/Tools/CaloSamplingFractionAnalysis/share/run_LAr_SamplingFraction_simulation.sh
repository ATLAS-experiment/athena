#!/bin/bash
inputEVNT=$1
outputHITS=$2
physlist=$3
nsubsample=$4
maxjobs=$5
if test -z "$nsubsample"
then
  nsubsample=1
fi  
if test -z "$maxjobs"
then
  maxjobs=1
fi  
nevents=5000
skip=0
waittime=120

echo inputEVNT=$inputEVNT
echo simulate $nsubsample'*'$nevents events

for((ijob=0;ijob<$nsubsample;ijob+=1))
do
  outfile_job=${outputHITS/.pool.root/.$ijob.pool.root}
  rundir="rundir_"$(basename $inputEVNT)"_job$ijob"
  echo job=$ijob, skip=$skip, outputHITS=$outfile_job, rundir=$rundir
  mkdir -p $rundir
  cd $rundir
  nohup Sim_tf.py --simulator 'FullG4MT' \
  --CA \
  --conditionsTag 'default:OFLCOND-MC16-SDR-14' \
  --physicsList "$physlist" \
  --truthStrategy 'MC15aPlus' \
  --postInclude 'default:PyJobTransforms.UseFrontier' \
  --preExec 'from SimulationConfig.SimEnums import VertexSource;flags.Sim.VertexSource=VertexSource.AsGenerated;from SimuJobTransforms.SimulationHelpers import enableBeamPipeKill;enableBeamPipeKill(flags);from SimuJobTransforms.SimulationHelpers import enableCalHits;enableCalHits(flags);from SimuJobTransforms.SimulationHelpers import enableParticleID;enableParticleID(flags);from SimuJobTransforms.SimulationHelpers import enableTightMuonStepping;enableTightMuonStepping;enableTightMuonStepping(flags)' \
  --DataRunNumber '284500' \
  --geometryVersion 'default:ATLAS-R2-2016-01-00-01' \
  --inputEVNTFile "$inputEVNT" \
  --outputHITSFile "$outfile_job" \
  --maxEvents $nevents \
  --skipEvent $skip \
  --imf False &
  cd ..
  let "skip=$skip + $nevents"

  nrun=$(jobs | grep Sim_tf.py | wc -l)
  while [ "$nrun" -ge "$maxjobs" ]
  do
    echo $nrun jobs running, maximum is $maxjobs, waiting $waittime seconds
    sleep $waittime
    nrun=$(jobs | grep Sim_tf.py | wc -l)
  done
  echo $nrun jobs running, maximum is $maxjobs, continuing with next job
done
