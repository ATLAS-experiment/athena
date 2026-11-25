#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=100

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
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect these tests to succeed
    [ "${name}" = "dcube-default-ambi" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=4

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")


# Run with full ACTS chain, including ACTS ambi. resolution
run "Reconstruction-ambi" \
    Reco_tf.py \
    --conditionsTag "default:${conditionsTag}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "all:flags.Scheduler.ShowDataDeps = True;flags.Scheduler.ShowDataFlow = True; flags.Acts.doAmbiguityResolution=True" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.ambi.root \
    --postExec "cfg.printConfig(withDetails=True, summariseProps=True);" \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.ambi

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-ambi" \
    runIDPVM.py \
    --OnlyTrackingPreInclude \
    --filesInput AOD.ambi.root \
    --outputFile idpvm.ambi.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi


# Run with default ACTS
run "Reconstruction" \
    Reco_tf.py \
    --conditionsTag "default:${conditionsTag}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.default.root \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.default

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-default" \
    runIDPVM.py \
    --OnlyTrackingPreInclude \
    --filesInput AOD.default.root \
    --outputFile idpvm.default.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi


echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-default-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_default_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.default.root \
    idpvm.default.root

run "dcube-ambi-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ambi_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.ambi.root \
    idpvm.ambi.root

# Compare performance without vs with ambi
run "dcube-default-ambi" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_default_ambi \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.default.root \
    -M "ambi" \
    -R "default" \
    idpvm.ambi.root

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
