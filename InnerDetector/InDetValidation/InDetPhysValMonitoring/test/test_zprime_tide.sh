#!/bin/bash
# art-description: Standard test for MC23a zprime for IDTIDE
# art-input: mc23_13p6TeV:mc23_13p6TeV.801271.Py8EG_A14NNPDF23LO_flatpT_Zprime.merge.HITS.e8514_e8528_s4159_s4114
# art-input-nfiles: 1
# art-cores: 4
# art-memory: 4096
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-output: physval*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_idtide_last

# Fix ordering of output in logfile
exec 2>&1
run() { (set -x; exec "$@") }

relname="r24.0.65"

lastref_dir=last_results

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
dcubeXml_idtide="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/dcube/config/IDPVMPlots_idtide.xml"
dcubeRef_idtide=$artdata/InDetPhysValMonitoring/ReferenceHistograms/${relname}/physval_zprime_tide.root

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml_idtide -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

export ATHENA_PROC_NUMBER=1
export ATHENA_CORE_NUMBER=1
# Reco step based on test InDetPhysValMonitoring ART setup from Josh Moss.
Reco_tf.py \
    --inputHITSFile=${ArtInFile} \
    --maxEvents 100 \
    --postInclude "default:PyJobTransforms.UseFrontier"  \
    --autoConfiguration="everything" \
    --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-07" \
    --digiSeedOffset1="8" \
    --digiSeedOffset2="8" \
    --CA "default:True" \
    --steering "doRAWtoALL" \
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

  echo "compare with 24.0.1"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide \
    -c ${dcubeXmlAbsPath} \
    -r ${dcubeRef_idtide} \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide"
  
  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_idtide_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/physval_idtide.ntuple.root \
    physval_idtide.ntuple.root
  echo "art-result: $? shifter_plots_idtide_last"

fi

