#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, PU 200
# art-input: mc21_14TeV:mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
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
    [ "${name}" = "dcube-athena-acts" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=4

# Run with legacy Athena
run "Reconstruction-athena" \
    Reco_tf.py --CA \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Tracking.ITkMainPass.doAmbiguityProcessorTrackFit=False;flags.Reco.EnableHGTDExtension=False;" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.athena.root \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.ATHENA

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-athena" \
    runIDPVM.py \
    --OnlyTrackingPreInclude \
    --filesInput AOD.athena.root \
    --outputFile idpvm.athena.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.+ERROR.+Step.+size.+adjustment.+exceeds.+maximum.+trials,Acts.+FindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+SurfaceError:1,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"

# Run with full ACTS chain, including ACTS ambi. resolution
run "Reconstruction-acts" \
    Reco_tf.py --CA \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateAmbiguityResolutionFlags" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.acts.root \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.ACTS

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --OnlyTrackingPreInclude \
    --filesInput AOD.acts.root \
    --outputFile idpvm.acts.root

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
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root

run "dcube-acts-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_acts_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

# Compare performance ACTS vs Athena
run "dcube-athena-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_acts \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.athena.root \
    -M "acts" \
    -R "athena" \
    idpvm.acts.root
