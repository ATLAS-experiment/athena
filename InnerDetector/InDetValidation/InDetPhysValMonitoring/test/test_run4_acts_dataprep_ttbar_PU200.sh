#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_R22.xml
rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
nEvents=20

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

# Run ACTS Reco
export ATHENA_CORE_NUMBER=8

conditionsTag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

run "Reconstruction-acts" \
    Reco_tf.py \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.acts.root \
    --conditionsTag "default:${conditionsTag}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Acts.EDM.PersistifyClusters=True;flags.Acts.EDM.PersistifySpacePoints=True;" \
    --maxEvents ${nEvents} \
    --perfmon fullmonmt \
    --multithreaded

reco_rc=$?

# Rename log
mv log.RAWtoALL log.RAWtoALL.acts

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --filesInput AOD.acts.root \
    --outputFile idpvm.acts.root \
    --OnlyTrackingPreInclude \
    --doActs

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_shifter_last \
    --plotopts=ratio \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
