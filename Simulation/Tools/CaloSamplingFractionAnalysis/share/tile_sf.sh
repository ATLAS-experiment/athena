#!/bin/bash

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

   preExec="flags.TestBeam.BeamPID=11; flags.TestBeam.BeamEnergy=100000; flags.TestBeam.Zbeam=[-20,20]; ${flags}"
   postExec="cfg.getService(\"GeoModelSvc\").DetectorTools[ \"TileDetectorTool\" ].Ushape = 1;from TileSimEx.TileSimOutputConfig import TileSimOutputCfg; cfg.merge(TileSimOutputCfg(flags, ntupleOutput=\"tiletb${v}.ntup.root\", d3pdOutput=\"tiletb${v}.d3pd.root\"))"

   echo  TestBeam_tf.py --CA --DataRunNumber 1 --outputHITSFile tiletb${v}.HITS.pool.root --maxEvents $nevt --preExec \'$preExec\' --postExec \'$postExec\'
   echo; TestBeam_tf.py --CA --DataRunNumber 1 --outputHITSFile tiletb${v}.HITS.pool.root --maxEvents $nevt --preExec="${preExec}" --postExec="${postExec}"

   echo; root -b -q "tile_sf.C(\"$v\");" > sf$v.log
done

#grep SF sf*.log
get_files TileSamplingFraction_analysis.C
root -b -q "TileSamplingFraction_analysis.C" > TileSamplingFraction_analysis.log
