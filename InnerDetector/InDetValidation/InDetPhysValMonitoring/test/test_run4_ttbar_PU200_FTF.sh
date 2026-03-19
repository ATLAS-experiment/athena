#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, TrigFastTrackFinder as an offline algorithm
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_last

lastref_dir=last_results
dcubeXml=IDPVMPlots_ITk_FastTrackFinder.xml
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

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
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.root \
    --conditionsTag "default:${conditionsTag}" \
    --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude \
    --preExec "flags.Tracking.useITkFTF=True;flags.Tracking.doITkFastTracking=True;flags.Tracking.ITkFTFPass.useTracklets=True;flags.Tracking.ITkFTFPass.useTrigRoadPredictor=True;flags.Tracking.ITkFTFPass.useTrigTrackFollowing=True"


run "IDPVM" \
    runIDPVM.py \
    --filesInput AOD.root \
    --outputFile idpvm.root \
    --doHitLevelPlots \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --truthMinPt=1000 \
    --doMuonMatchedTracks

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

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
