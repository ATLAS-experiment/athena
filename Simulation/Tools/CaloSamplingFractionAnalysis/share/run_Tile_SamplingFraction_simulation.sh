#!/bin/bash
if test -z "$ATHENA_CORE_NUMBER"
then
  echo "setting up multicores"
  export ATHENA_CORE_NUMBER=8
fi


nevt=${1:-500}
if [ $# -gt 0 ]; then
  shift
fi

if [ $# -gt 0 ]; then
  args="$*"
else
  args="-5 +5 -10 +10 -15 +15 -20 +20 -25 +25 -30 +30 -35 +35 -40 +40 -45 +45 -50 +50 -55 +55 -60 +60 -65 +65"
fi

if [ ! -f tile_sf.C ]
then get_files tile_sf.C
fi

for v in `echo $args`
do if [[ "$v" -lt 1 ]] && [[ "$v" -gt -1 ]] && [ "${v/./}" != "${v}" ]
   then flags="flags.TestBeam.Eta=$v;"
   elif [[ "$v" -lt 90 ]] && [[ "$v" -gt -90 ]]
   then flags="flags.TestBeam.Z=0; flags.TestBeam.Theta=$v;"
   elif [[ "$v" -eq 90 ]] || [[ "$v" -eq -90 ]]
   then flags="flags.TestBeam.Z=2550; flags.TestBeam.Theta=$v;"
   elif [[ "$v" -gt  2280 ]] && [[ "$v" -lt  4250 ]]
   then flags="flags.TestBeam.Theta=90; flags.TestBeam.Z=${v/+/};"
   elif [[ "$v" -lt -2280 ]] && [[ "$v" -gt -4250 ]]
   then flags="flags.TestBeam.Theta=-90; flags.TestBeam.Z=${v/-/};"
   else echo "Ignoring invalid value $v"
        continue
   fi
   echo "========== Running TestBeam_tf.py for ${flags}: $nevt events =========="
   echo

   preExec="flags.TestBeam.BeamPID=11; flags.TestBeam.BeamEnergy=100000; flags.TestBeam.Zbeam=[-20,20]; flags.Tile.Sim.Ushape=1; ${flags}"
   postExec="cfg.getService(\"AvalancheSchedulerSvc\").DataLoaderAlg=\"ParticleGun\";"

   echo  TestBeam_tf.py --CA --DataRunNumber 1 --outputHITSFile tiletb${v}.HITS.pool.root --maxEvents $nevt --conditionsTag OFLCOND-MC12-SDR-27 --preExec \'$preExec\' --postExec \'$postExec\' --multithreaded
   echo; TestBeam_tf.py --CA --DataRunNumber 1 --outputHITSFile tiletb${v}.HITS.pool.root --maxEvents $nevt --conditionsTag OFLCOND-MC12-SDR-27 --preExec="${preExec}" --postExec="${postExec}" --multithreaded
   echo  "art-result: $? Tile Test Beam Simulation: theta = ${v}"

   echo
   echo "========== Running Tile Test Beam digitization: $nevt events =========="
   echo
   postExec="cfg.getCondAlgo(\"TileSamplingFractionCondAlg\").G4Version=-1;"
   echo  athena --CA TileSimEx/TileDigiRec.py --testbeam --evtMax -1 --filesInput tiletb${v}.HITS.pool.root --d3pd --hits-ntuple --file-prefix tiletb${v} --postExec \'${postExec}\'
   echo; athena --CA TileSimEx/TileDigiRec.py --testbeam --evtMax -1 --filesInput tiletb${v}.HITS.pool.root --d3pd --hits-ntuple --file-prefix tiletb${v} --postExec="${postExec}"
   echo  "art-result: $? Tile Test Beam Digitization: theta = ${v}"

   echo; root -b -q "tile_sf.C(\"$v\");" > sf$v.log
   echo  "art-result: $? Tile Sampling Fraction fit: theta = ${v}"
done

#grep SF sf*.log
get_files TileSamplingFraction_analysis.C
root -b -q "TileSamplingFraction_analysis.C(\"${args}\")" > TileSamplingFraction_analysis.log
echo  "art-result: $? Tile Sampling Fraction analysis"
