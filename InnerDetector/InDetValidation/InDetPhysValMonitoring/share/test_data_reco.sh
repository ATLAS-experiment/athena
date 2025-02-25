#!/bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART jobs with Data Reco config

inputBS=$1
dcubeRef=$2
conditions=$3
geotag=$4

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeShifterXml=${artdata}/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_baseline.xml
dcubeExpertXml=${artdata}/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_expert.xml
lastref_dir=last_results

run() { (set -x; exec "$@") }

run  Reco_tf.py \
  --inputBSFile "$inputBS" \
  --maxEvents 1000 \
  --autoConfiguration everything \
  --conditionsTag   "$conditions" \
  --geometryVersion "$geotag" \
  --outputAODFile   physval.AOD.root \
  --steering        doRAWtoALL \
  --checkEventCount False \
  --ignoreErrors    True 
rec_tf_exit_code=$?
echo "art-result: $rec_tf_exit_code reco"

run runIDPVM.py \
  --filesInput physval.AOD.root \
  --outputFile physval.ntuple.root \
  --doHitLevelPlots \
  --doExpertPlots
idpvm_tf_exit_code=$?
echo "art-result: $idpvm_tf_exit_code idpvm"

if [ $rec_tf_exit_code -eq 0 ]  ;then
  echo "download latest result"
  run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
  run ls -la "$lastref_dir"

  echo "compare with fixed reference"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter \
    -c ${dcubeShifterXml} \
    -r ${dcubeRef} \
    physval.ntuple.root
  echo "art-result: $? shifter_plots"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    -c ${dcubeShifterXml} \
    -r ${lastref_dir}/physval.ntuple.root \
    physval.ntuple.root
  echo "art-result: $? shifter_plots_last"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_expert \
    -c ${dcubeExpertXml} \
    -r ${dcubeRef} \
    physval.ntuple.root
  
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_expert_last \
    -c ${dcubeExpertXml} \
    -r ${lastref_dir}/physval.ntuple.root \
    physval.ntuple.root

fi
