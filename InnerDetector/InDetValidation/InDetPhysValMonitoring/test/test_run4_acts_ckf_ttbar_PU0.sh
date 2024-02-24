#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, no pileup
# art-input: mc21_14TeV:mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14697
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_last

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
n_events=1000

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# Don't run if dcube config not found
if [ -z "$dcubeXmlAbsPath" ]; then
    echo "art-result: 1 dcube-xml-config"
    exit 1
fi

run () {
    name="${1}"
    cmd="${@:2}"
    ############
    echo "Running ${name}..."
    time ${cmd}
    rc=$?
    # Only report hard failures for 21.9 vs master tests since both
    # branches are unlikely to ever match perfectly
    [ "${name}" = "dcube-ckf-ambi" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

ignore_pattern="ActsTrackFindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,ActsTrackFindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:3.+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters"

# Run w/o ambi. resolution
run "Reconstruction-ckf" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --preExec 'flags.Acts.doMonitoring=True;' \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${n_events}

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.CKF
mv acts-expert-monitoring.root acts-expert-monitoring.ckf.root

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-ckf" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.ckf.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run w/ ambi. resolution
run "Reconstruction-ambi" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateResolvedTracksFlags" \
    --preExec 'flags.Acts.doMonitoring=True;' \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ambi.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events}

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.AMBI
mv acts-expert-monitoring.root acts-expert-monitoring.ambi.root

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-ambi" \
    runIDPVM.py \
    --filesInput AOD.ambi.root \
    --outputFile idpvm.ambi.root

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

run "dcube-ckf-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.ckf.root \
    idpvm.ckf.root

run "dcube-ambi-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ambi_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.ambi.root \
    idpvm.ambi.root

# Compare performance w/ and w/o ambi. resolution
run "dcube-ckf-ambi" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_ambi \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.ckf.root \
    idpvm.ambi.root
