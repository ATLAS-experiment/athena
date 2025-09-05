#!/bin/bash
# art-description: Standard test for MC23a ttbar
# art-input: user.keli:user.keli.mc23a_13TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4111_s4114_r14622_tid33359244_00
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_lrt_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

relname="r24.0.109"

lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml_lrt="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_lrt.xml"
dcubeRef_lrt=${artdata}/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_ttbarPU40_reco.root

conditions=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN3_MC)")

# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
run Reco_tf.py \
  --CA \
  --runNumber="801271" \
  --AMITag="r14519" \
  --autoConfiguration="everything" \
  --conditionsTag "default:${conditions}" \
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

  echo "compare with 24.0.1"

  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt \
    -c ${dcubeXml_lrt} \
    -r ${dcubeRef_lrt} \
    physval_lrt.ntuple.root
  echo "art-result: $? shifter_plots_lrt"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_lrt_last \
    -c ${dcubeXml_lrt} \
    -r ${lastref_dir}/physval_lrt.ntuple.root \
    physval_lrt.ntuple.root
  echo "art-result: $? shifter_plots_lrt_last"
fi

