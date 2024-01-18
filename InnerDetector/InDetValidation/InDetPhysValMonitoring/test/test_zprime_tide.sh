#!/bin/bash
# art-description: Standard test for MC23a zprime for IDTIDE
# art-input: user.keli:user.keli.mc23a_13TeV.801271.Py8EG_A14NNPDF23LO_flatpT_Zprime.merge.HITS.e8514_s4100_s4101_tid32652444_00
# art-input-nfiles: 1
# art-type: grid
# art-include: master/Athena
# art-include: 22.0/Athena
# art-include: 23.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

#RDO is made at rel 22.0.73
#reference plots are made at rel 22.0.73

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }


lastref_dir=last_results
artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_idtide.xml"
dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_zprime_idtide_r24.root"
if [[ "$ATLAS_RELEASE_BASE" == *"23.0"* ]]; then
  dcubeRef_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_zprime_idtide_r23.root"
fi

export ATHENA_PROC_NUMBER=1
export ATHENA_CORE_NUMBER=1
# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
Reco_tf.py \
    --inputHITSFile=${ArtInFile} \
    --maxEvents 100 \
    --postInclude "default:PyJobTransforms.UseFrontier"  \
    --autoConfiguration="everything" \
    --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-01" \
    --geometryVersion="default:ATLAS-R3S-2021-03-02-00" \
    --digiSeedOffset1="8" \
    --digiSeedOffset2="8" \
    --CA "default:True" "RDOtoRDOTrigger:False" \
    --steering "doRDO_TRIG" "doTRIGtoALL" \
    --outputDAOD_IDTIDEFile="DAOD_TIDE.pool.root"  \
    --outputRDOFile output.RDO.root \
    --multithreaded="True"
rec_tf_exit_code=$?
echo "art-result: $rec_tf_exit_code reco"

if [ $rec_tf_exit_code -eq 0 ]  ;then
  #run IDPVM for IDTIDE derivation
  run runIDPVM.py --doIDTIDE --doTracksInJets --doTracksInBJets --filesInput DAOD_TIDE.pool.root --outputFile physval_idtide.ntuple.root

  echo "download latest result"
  run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
  run ls -la "$lastref_dir"

  echo "compare with R23.0.23 or 24.0.1"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide \
    -c ${dcubeXml_idtide} \
    -r ${dcubeRef_idtide} \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide_last \
    -c ${dcubeXml_idtide} \
    -r ${lastref_dir}/physval_idtide.ntuple.root \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide_last"

fi

