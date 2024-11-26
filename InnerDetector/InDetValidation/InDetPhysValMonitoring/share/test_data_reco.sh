#!/bin/bash
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Steering script for IDPVM ART jobs with Data Reco config

ArtProcess=$1
ArtInFile=$2
dcubeRef=$3
lastref_dir=$4
conditions=$5
geotag=$6

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeShifterXml=${artdata}/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_baseline.xml
dcubeExpertXml=${artdata}/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_expert.xml

set -x

echo "ArtProcess: $ArtProcess"
lastref_dir=last_results
script="`basename \"$0\"`"
success_run=0

case $ArtProcess in
  "start")
    echo "Starting"
    echo "List of files = " ${ArtInFile}
    ;;
  "end")
    echo "Ending"
    if [ ${success_run} -eq 0 ]  ;then
      echo "download latest result"
      art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
      ls -la "$lastref_dir"
      echo "Merging physval.root"
      hadd  physval.root art_core_*/physval.ntuple.root
      echo "postprocess"
      postProcessIDPVMHistos physval.root

      echo "compare with fixed reference"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter \
	   -c ${dcubeShifterXml} \
	   -r ${dcubeRef} \
	   physval.root
      echo "art-result: $? shifter_plots"

      echo "compare with last build"
      $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	   -p -x dcube_shifter_last \
	   -c ${dcubeShifterXml} \
	   -r last_results/physval.root \
	   physval.root
      echo "art-result: $? shifter_plots_last"

      if $doExpert ; then
          $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
              -p -x dcube_expert \
              -c ${dcubeExpertXml} \
	      -r ${dcubeRef} \
	      physval.root

          $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
	      -p -x dcube_expert_last \
	      -c ${dcubeExpertXml} \
	      -r last_results/physval.root \
	      physval.root
      fi
    else
      echo "reco failed"
    fi
    ;;
  *)
    echo "Test $ArtProcess"
    mkdir "art_core_${ArtProcess}"
    cd "art_core_${ArtProcess}"
    IFS=',' read -r -a file <<< "${ArtInFile}"
    file=${file[${ArtProcess}]}
    x="../$file"
    echo "Unsetting ATHENA_NUM_PROC=${ATHENA_NUM_PROC} and ATHENA_PROC_NUMBER=${ATHENA_PROC_NUMBER}"
    unset  ATHENA_NUM_PROC
    unset  ATHENA_PROC_NUMBER

    Reco_tf.py \
	--inputBSFile $x \
	--maxEvents 1000 \
	--autoConfiguration everything \
	--conditionsTag   "$conditions" \
	--geometryVersion "$geotag" \
	--outputAODFile   physval.AOD.root \
	--steering        doRAWtoALL \
	--checkEventCount False \
	--ignoreErrors    True
    rec_tf_exit_code=$?

    echo "art-result: $rec_tf_exit_code reco_${file}"

    run runIDPVM.py \
	--filesInput physval.AOD.root \
	--outputFile physval.ntuple.root \
	--doHitLevelPlots \
	--doExpertPlots
    idpvm_tf_exit_code=$?
    echo "art-result: $idpvm_tf_exit_code idpvm"

    if [ $rec_tf_exit_code -ne 0 ]  ;then
      success_run=$rec_tf_exit_code
    fi
    ls -lR
    ;;
esac
