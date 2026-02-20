#!/bin/bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Steering script for CampaignsARTTests with mu=0 configs

echo "Input Parameters"
number_of_events=$1

#Option for sim/digi/reco
default_geometry=$(python -c "from AthenaConfiguration.TestDefaults import defaultGeometryTags; print(defaultGeometryTags.RUN4)")
default_condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

run () {
  name="${1}"
  cmd="${@:2}"
  echo "Running transform for ${name}\n"
  time ${cmd}
  rc=$?
  echo "art-result: $rc ${name}"
  return $rc
}

checkstep () {
  if [ $? != 0 ]
  then
    exit $?
  else
    echo "${1} Succeeded"
  fi
}

run "Simulation" Sim_tf.py \
  --conditionsTag "default:${default_condition}" \
  --geometryVersion "default:${default_geometry}" \
  --multithreaded "True" \
  --postInclude "default:PyJobTransforms.UseFrontier" \
  --preInclude "EVNTtoHITS:Campaigns.MC23PhaseIISimulation" \
  --simulator "FullG4MT" \
  --inputEVNTFile ${ArtInFile} \
  --outputHITSFile "HITS.pool.root" \
  --imf False \
  --maxEvents ${number_of_events}

checkstep "Simulation"

export ATHENA_CORE_NUMBER=1

run "RAWtoALL" Reco_tf.py \
  --athenaMPEventsBeforeFork "1" \
  --autoConfiguration "everything" \
  --conditionsTag "all:${default_condition}" \
  --digiSteeringConf "StandardSignalOnlyTruth" \
  --geometryVersion "all:${default_geometry}" \
  --multithreaded "True" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --preInclude "all:Campaigns.MC23PhaseIINoPileUp" \
  --inputHitsFile "HITS.pool.root" \
  --outputAODFile "AOD.pool.root" \
  --maxEvents ${number_of_events}

#a missing Acts material map can be overriden by a statement like 
#--preExec "all:ConfigFlags.Acts.TrackingGeometry.MaterialSource='material-maps-ATLAS-P2-RUN4-01-01-00-ITk-HGTD.json'" \

checkstep "RAWtoALL"

run "AODtoDAOD_PHYSVAL" Derivation_tf.py \
  --athenaMPMergeTargetSize "DAOD_*:0" \
  --formats "PHYSVAL" \
  --multiprocess "True" \
  --sharedWriter "True" \
  --inputAODFile "AOD.pool.root" \
  --outputDAODFile "OUT.root" \
  --maxEvents ${number_of_events}

checkstep "AODtoDAOD_PHYSVAL"

run "NTUP_PHYSVAL" Derivation_tf.py \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_PHYSVAL.root" \
  --validationFlags doInDet, doMET, doEgamma, doTau, doJet, doTopoCluster, doPFlow, doMuon, doLLPSecVtx, doBtag \
  --format NTUP_PHYSVAL \
  --maxEvents ${number_of_events}

mv runargs.PhysicsValidation.py runargs.PhysicsValidation.Main.py
mv log.PhysicsValidation log.PhysicsValidation.Main

checkstep "NTUP_PHYSVAL"
 
if [ -d art_core_* ]
then
    run "NTUPMerge" NTUPMerge_tf.py \
	--inputNTUP_PHYSVALFile art_core_*/NTUP_PHYSVAL.root \
	--outputNTUP_PHYSVAL_MRGFile NTUP_MERGE_PHYSVAL.root
else
    run "NTUPMerge" NTUPMerge_tf.py \
	--inputNTUP_PHYSVALFile NTUP_PHYSVAL.root \
	--outputNTUP_PHYSVAL_MRGFile NTUP_MERGE_PHYSVAL.root
fi

checkstep "NTUPMerge"
