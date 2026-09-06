#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS LRT with Athena LEGACY, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: acts-expert-monitoring*.root
# art-output: *idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXmlTechEffLRT=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff_lrt.xml
n_events=-1
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

# search in $DATAPATH for matching file
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEffLRT -print -quit 2>/dev/null)

# Don't run if dcube config not found
if [ -z "$dcubeXmlTechEffAbsPath" ]; then
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

export ATHENA_CORE_NUMBER=4

# Run with Acts
run "Reconstruction-acts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingRecoPreInclude" \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True;" \
    --conditionsTag "default:${conditionsTag}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.acts.root \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.acts

if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --filesInput AOD.acts.root \
    --outputFile idpvm.acts.root \
    --HSFlag All \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude \
    --doLargeD0Tracks \
    --largeD0TrackCollection InDetActsLargeRadiusTrackParticles

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-acts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
