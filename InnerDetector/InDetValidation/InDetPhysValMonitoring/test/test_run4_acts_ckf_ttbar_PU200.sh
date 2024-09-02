#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, PU 200
# art-type: grid
# art-include: main/Athena
# art-output: idpvm*.root
# art-output: acts-*.root
# art-output: last_results/idpvm*.root
# art-output: last_results/art_download_AtlasBuildStamp
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_last
# art-athena-mt: 4

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
### uncomment this and other lines to enable technical efficiency
# dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml
n_events=-1
rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
ref_idpvm_athena=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_run4_ttbar200_reco_r25.root

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
# dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
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
    # Only report hard failures for comparison Acts-Trk since we know
    # they are different. We do not expect these tests to succeed
    [ "${name}" = "dcube-ckf-ambi" -o "${name}" = "dcube-ckf-athena" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

ignore_pattern="ActsValidateTracksTrackFindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,ActsValidateTracksTrackFindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:3.+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,ActsValidateTracksTrackFindingAlg.+ERROR.+Step.+size.+adjustment.+exceeds.+maximum.+trials,ActsValidateTracksTrackFindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,ActsValidateTracksTrackFindingAlg.Acts.+ERROR.+SurfaceError:1"

export ATHENA_CORE_NUMBER=4

# Run with Athena ambi. resolution
run "Reconstruction-ckf" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --preExec 'flags.Acts.doMonitoring=True;' \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${n_events} \
    --multithreaded
    # --preExec 'flags.Acts.doMonitoring=True; flags.Tracking.writeExtendedSi_PRDInfo=True; flags.Tracking.doStoreSiSPSeededTracks=True; flags.Tracking.ITkActsValidateTracksPass.storeSiSPSeededTracks=True;' \

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.CKF
mv acts-expert-monitoring.root acts-expert-monitoring.ckf.root

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-ckf" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.ckf.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --HSFlag All \
    --doExpertPlots
    # --validateExtraTrackCollections "SiSPSeededTracksActsValidateTracksTrackParticles"
    # --doTechnicalEfficiency \

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run with ACTS ambi. resolution
run "Reconstruction-ambi" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateResolvedTracksFlags" \
    --preExec 'flags.Acts.doMonitoring=True;' \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${rdo} \
    --outputAODFile AOD.ambi.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.AMBI
mv acts-expert-monitoring.root acts-expert-monitoring.ambi.root

if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-ambi" \
    runIDPVM.py \
    --filesInput AOD.ambi.root \
    --outputFile idpvm.ambi.root \
    --doTightPrimary \
    --doHitLevelPlots \
    --HSFlag All \
    --doExpertPlots

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
    # -c ${dcubeXmlTechEffAbsPath} \

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
    -M "ckf" \
    -R "ambi" \
    idpvm.ambi.root

# Compare performance WRT legacy Athena
run "dcube-ckf-athena" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_ckf_athena \
    -c ${dcubeXmlAbsPath} \
    -r ${ref_idpvm_athena} \
    -M "acts" \
    -R "athena" \
    idpvm.ckf.root
    # -c ${dcubeXmlTechEffAbsPath} \
