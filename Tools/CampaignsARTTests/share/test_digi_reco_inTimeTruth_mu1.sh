#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for CampaignsARTTests with mu=1 inTimeTruth configs

echo "Input Parameters"
number_of_events=$1

#Option for sim/digi/reco
default_geometry="ATLAS-P2-RUN4-03-00-00"
default_condition=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

#Post-processing for ID/ITk and FTag
ftag_merge_DQA="${Athena_DIR}/src/PhysicsAnalysis/JetTagging/JetTagValidation/JetTagDQA/scripts/"
ftag_merge_script="mergePhysValFiles.py"
ftag_roc_script="Draw_PhysVal_btagROC.c"
idpvm_merge_script="postProcessIDPVMHistos.py"

#HITS inputs
if [ -z ${ATLAS_REFERENCE_DATA+x} ]; then
  ATLAS_REFERENCE_DATA="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art"
fi

HSHitsFile="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"
HighPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4151/*"
LowPtMinbiasHitsFiles="${ATLAS_REFERENCE_DATA}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4151/*"


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

run "RAWtoALL" Reco_tf.py \
  --CA "all:True" \
  --athenaMPEventsBeforeFork "1" \
  --autoConfiguration "everything" \
  --conditionsTag "all:${default_condition}" \
  --digiSteeringConf "StandardInTimeOnlyTruth" \
  --geometryVersion "all:${default_geometry}" \
  --multithreaded "True" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --preInclude "all:Campaigns.PhaseIIPileUp1" \
  --inputHITSFile ${HSHitsFile} \
  --inputHighPtMinbiasHitsFile ${HighPtMinbiasHitsFiles} \
  --inputLowPtMinbiasHitsFile ${LowPtMinbiasHitsFiles} \
  --outputAODFile "AOD.pool.root" \
  --maxEvents ${number_of_events}

checkstep "RAWtoALL"

run "AODtoDAOD_PHYSVAL" Derivation_tf.py \
  --CA "all:True" \
  --athenaMPMergeTargetSize "DAOD_*:0" \
  --formats "PHYSVAL" \
  --multiprocess "True" \
  --sharedWriter "True" \
  --inputAODFile "AOD.pool.root" \
  --outputDAODFile "OUT.root" \
  --maxEvents ${number_of_events}

checkstep "AODtoDAOD_PHYSVAL"

run "NTUP_PHYSVAL" Derivation_tf.py \
  --CA \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_PHYSVAL.root" \
  --validationFlags doInDet, doMET, doEgamma, doTau, doJet, doTopoCluster, doPFlow, doMuon, doLLPSecVtx \
  --format NTUP_PHYSVAL \
  --preExec "flags.PhysVal.IDPVM.setTruthStrategy='All'" \
  --maxEvents ${number_of_events}

mv runargs.PhysicsValidation.py runargs.PhysicsValidation.Main.py
mv log.PhysicsValidation log.PhysicsValidation.Main

checkstep "NTUP_PHYSVAL"

#Run btag separately because they are ..doing things differently >:V
run "NTUP_BTAG_PHYSVAL" Derivation_tf.py \
  --CA \
  --inputDAOD_PHYSVALFile "DAOD_PHYSVAL.OUT.root" \
  --outputNTUP_PHYSVALFile "NTUP_BTAG_PHYSVAL.root" \
  --validationFlags doBtag \
  --format NTUP_PHYSVAL \
  --maxEvents ${number_of_events}

mv runargs.PhysicsValidation.py runargs.PhysicsValidation.BTAG.py
mv log.PhysicsValidation log.PhysicsValidation.BTAG

checkstep "NTUP_BTAG_PHYSVAL"
 
if [ -d art_core_* ]
then
  echo "Merging histograms"
  hadd NTUP_PHYSVAL.root art_core_*/NTUP_PHYSVAL.root
  $idpvm_merge_script NTUP_PHYSVAL.root
  python $ftag_merge_DQA/$ftag_merge_script --input art_core_*/* --pattern "*BTAG_PHYSVAL*" --output NTUP_BTAG_MERGE_PHYSVAL.root -d BTag
else
  python $ftag_merge_DQA/$ftag_merge_script --pattern "*BTAG_PHYSVAL*"  --output NTUP_BTAG_MERGE_PHYSVAL.root -d BTag
fi

# Disabled following ATLSWUPGR-190
#root -l -b -q $ftag_merge_DQA/$ftag_roc_script\(\"ttbar\",\"EMTopo\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"NTUP_BTAG_MERGE_PHYSVAL.root\",\"ROC_NTUP_BTAG_MERGE_PHYSVAL.root\",\{\"IP3D\",\"SV1\",\"DL1dv00\",\"GN1\"\}\)

hadd NTUP_MERGE_PHYSVAL.root NTUP_PHYSVAL.root NTUP_BTAG_MERGE_PHYSVAL.root

checkstep "Merging and post processing"
