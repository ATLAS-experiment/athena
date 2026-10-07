# Steering script for IDPVM ART jobs with Data Reco config
maxEvents=1000

artdata=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art
lastref_dir=last_results
dcubeXml=dcube_ART_IDPVMPlots_vertex.xml
# search in $DATAPATH for matching file
dcubeShifterXml=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeShifterXml" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi


run() { (set -x; exec "$@") }

run  Derivation_tf.py \
     --inputAODFile ${ArtInFile} \
     --outputDAODFile sumpt2.root \
     --maxEvents $maxEvents \
     --formats PHYSVAL \
     --preExec "from TrkConfig.VertexFindingFlags import VertexSortingSetup; flags.Tracking.PriVertex.sortingSetup=VertexSortingSetup.SumPt2Sorting"

mv log.Derivation log.Derivation.sumpt2

der_tf_exit_code=$?
echo "art-result: $der_tf_exit_code deriv sumpt2"

run runIDPVM.py \
  --filesInput DAOD_PHYSVAL.sumpt2.root \
  --outputFile idpvm.sumpt2.root 
idpvm_tf_exit_code=$?
echo "art-result: $idpvm_tf_exit_code idpvm sumpt2"

run  Derivation_tf.py \
     --inputAODFile ${ArtInFile} \
     --outputDAODFile hsgnn.root \
     --maxEvents $maxEvents \
     --formats PHYSVAL \
     --preExec "from TrkConfig.VertexFindingFlags import VertexSortingSetup; flags.Tracking.PriVertex.sortingSetup=VertexSortingSetup.GNNSorting"

mv log.Derivation log.Derivation.hsgnn

der_tf_exit_code=$?
echo "art-result: $der_tf_exit_code deriv hsgnn"

run runIDPVM.py \
  --filesInput DAOD_PHYSVAL.hsgnn.root \
  --outputFile idpvm.hsgnn.root 
idpvm_tf_exit_code=$?
echo "art-result: $idpvm_tf_exit_code idpvm hsgnn"


if [ $idpvm_tf_exit_code -eq 0 ]  ;then
  echo "download latest result"
  run art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
  run ls -la "$lastref_dir"

  echo "compare with sumpt2 reference"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_sumpt2 \
    -c ${dcubeShifterXml} \
    -r idpvm.sumpt2.root \
    idpvm.hsgnn.root
  echo "art-result: $? sumpt2_plots"

  echo "compare with last build"
  $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    -c ${dcubeShifterXml} \
    -r ${lastref_dir}/idpvm.hsgnn.root \
    idpvm.hsgnn.root
  echo "art-result: $? shifter_plots_last"

fi
