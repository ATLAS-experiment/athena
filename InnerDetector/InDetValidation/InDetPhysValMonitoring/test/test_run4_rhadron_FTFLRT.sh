#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, TrigFastTrackFinder as an offline algorithm
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

lastref_dir=last_results
dcubeXml=IDPVMPlots_ITk_FastTrackFinderLRT.xml
rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.504643.MGPy8EG_A14NNPDF23LO_GG_qqn1_2000_1950_rpvLF_p1ns.recon.RDO.e8337_s4149_r14697/RDO.38107309._000003.pool.root.1

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

run () {
    name="${1}"
    cmd=("${@:2}")
    ############
    echo "Running ${name}..."
    time "${cmd[@]}"
    rc=$?
    echo "art-result: $rc ${name}"
    return $rc
}

run "Reconstruction" \
    Reco_tf.py \
    --CA \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.root \
    --steering doRAWtoALL \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude \
    --preExec "flags.Tracking.useITkFTF=True;flags.Tracking.doITkFastTracking=True;flags.Tracking.ITkFTFPass.useTracklets=True;flags.Tracking.ITkFTFPass.useTrigRoadPredictor=True;flags.Tracking.ITkFTFPass.useTrigTrackFollowing=True;flags.Tracking.doLargeD0=True;"


run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doHitLevelPlots \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --truthMinPt=1000 \
    --doMuonMatchedTracks \
    --doLargeD0Tracks

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.root \
    idpvm.root

