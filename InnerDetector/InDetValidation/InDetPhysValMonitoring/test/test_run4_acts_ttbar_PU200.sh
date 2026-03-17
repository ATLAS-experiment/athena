#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: acts-expert-monitoring*.root
# art-output: *idpvm*.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml
dcubeXmlTechEffLRT=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff_lrt.xml
n_events=-1
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")


# search in $DATAPATH for matching file
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
dcubeXmlTechEffLRTAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEffLRT -print -quit 2>/dev/null)

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
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-athena-acts" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=4

# Run with Athena legacy Fast Tracking
run "Reconstruction-athena" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingRecoPreInclude" \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.doITkFastTracking=True;" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.athena.root \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.athena

if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

run "IDPVM-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root \
    --HSFlag All \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --OnlyTrackingPreInclude

reco_rc=$?
if [ $reco_rc != 0 -a $reco_rc != 68 ]; then
    exit $reco_rc
fi

# Run with Acts
run "Reconstruction-acts" \
    Reco_tf.py \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingRecoPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec "flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Acts.doLargeRadius=True;" \
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

run "dcube-athena-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_shifter_last \
    -c ${dcubeXmlTechEffAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root


run "dcube-acts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlTechEffLRTAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

# Compare performance WRT legacy Athena
run "dcube-athena-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_acts \
    -c ${dcubeXmlTechEffAbsPath} \
    -r idpvm.athena.root \
    -M "acts" \
    -R "athena" \
    idpvm.acts.root

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
