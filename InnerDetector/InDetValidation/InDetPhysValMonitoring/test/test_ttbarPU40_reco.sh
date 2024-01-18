#!/bin/bash
# art-description: Standard test for MC23a ttbar
# art-input: user.keli:user.keli.mc23a_13TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4111_s4114_r14622_tid33359244_00
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_idtide_last

#RDO is made at rel 22.0.73
#reference plots are made at rel 22.0.73

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }


lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_lrt.xml"
dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r24.root"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r23.root"
fi
dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_idtide_r24.root"
dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r24.root"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_idtide_r23.root"
  dcubeRef_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_ttbarPU40_lrt_r23.root"
fi

# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
run Reco_tf.py \
  --CA \
  --runNumber="801271" \
  --AMITag="r14519" \
  --autoConfiguration="everything" \
  --conditionsTag   'default:OFLCOND-MC23-SDR-RUN3-01' \
  --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
  --inputRDOFile     ${ArtInFile} \
  --outputAODFile   physval.AOD.root \
  --steering        doRAWtoALL \
  --checkEventCount False \
  --ignoreErrors    True \
  --maxEvents       100 
rec_tf_exit_code=$?
echo "art-result: $rec_tf_exit_code reco"

if [ $rec_tf_exit_code -eq 0 ]  ;then
  #run IDPVM for IDTIDE derivation
  #for LRT
  run runIDPVM.py --doLargeD0Tracks --filesInput physval.AOD.root --outputFile physval_lrt.ntuple.root

  echo "download latest result"
  run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
  run ls -la "$lastref_dir"

  echo "compare with R23.0.23 or 24.0.1"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt \
    -c ${dcubeXml_lrt} \
    -r ${dcubeRef_lrt} \
    physval_lrt.ntuple.root
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt_last \
    -c ${dcubeXml_lrt} \
    -r ${lastref_dir}/physval_lrt.ntuple.root \
    physval_lrt.ntuple.root
  echo "art-result: $? shifter_plots_lrt_last"
fi

