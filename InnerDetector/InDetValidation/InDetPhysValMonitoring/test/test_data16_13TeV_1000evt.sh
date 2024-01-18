#!/bin/bash
# art-description: Standard test for 2016 data
# art-type: grid
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }


lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
inputBS=${artdata}/RecJobTransformTests/data16_13TeV.00310809.physics_Main.daq.RAW._lb1219._SFO-2._0001.data 
dcubeShifterXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_baseline.xml"
dcubeExpertXml="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_data_expert.xml"
dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_data16_1000evt_reco_r24.root"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  dcubeRef="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_data16_1000evt_reco_r23.root"
fi

# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.

run  Reco_tf.py \
  --CA \
  --inputBSFile "$inputBS" \
  --maxEvents 1000 \
  --autoConfiguration everything \
  --conditionsTag   'CONDBR2-BLKPA-RUN2-11' \
  --geometryVersion="ATLAS-R2-2016-01-00-01" \
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

#  echo "compare with R22 with nightly build at 23.0.23 or 24.0.1"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube \
    -c ${dcubeShifterXml} \
    -r ${dcubeRef} \
    physval.ntuple.root
  echo "art-result: $? shifter plots"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube \
    -c ${dcubeExpertXml} \
    -r ${dcubeRef} \
    physval.ntuple.root
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    -c ${dcubeShifterXml} \
    -r ${lastref_dir}/physval.ntuple.root \
    physval.ntuple.root
  echo "art-result: $? shifter_plots_last"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_expert_last \
    -c ${dcubeExpertXml} \
    -r ${lastref_dir}/physval.ntuple.root \
    physval.ntuple.root
fi

