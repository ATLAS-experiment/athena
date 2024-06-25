#!/bin/bash
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Steering script for FlavourTaggingTests

echo "Input Parameters"
number_of_events=$1

#Post-processing for ID/ITk and FTag
ftag_merge_DQA="${Athena_DIR}/src/PhysicsAnalysis/JetTagging/JetTagValidation/JetTagDQA/scripts/"
ftag_merge_script="mergePhysValFiles.py"

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

export ATHENA_CORE_NUMBER=1

run "AODtoDAOD_PHYSVAL" Derivation_tf.py \
  --athenaMPMergeTargetSize "DAOD_*:0" \
  --formats "PHYSVAL" \
  --multiprocess "True" \
  --sharedWriter "True" \
  --inputAODFile ${ArtInFile} \
  --outputDAODFile "OUT.root" \
  --maxEvents ${number_of_events}

checkstep "AODtoDAOD_PHYSVAL"

run "NTUP_BTAG_PHYSVAL" Derivation_tf.py \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_BTAG_PHYSVAL.root" \
  --validationFlags doBtag \
  --format NTUP_PHYSVAL \
  --maxEvents ${number_of_events}

checkstep "NTUP_BTAG_PHYSVAL"
 
if [ -d art_core_* ]
then
  echo "Merging histograms"  
  python $ftag_merge_DQA/$ftag_merge_script --input art_core_*/* --pattern "*BTAG_PHYSVAL*" --output NTUP_MERGE_PHYSVAL.root -d BTag
else
  python $ftag_merge_DQA/$ftag_merge_script --pattern "*BTAG_PHYSVAL*"  --output NTUP_MERGE_PHYSVAL.root -d BTag
fi

checkstep "Merging and post processing"
