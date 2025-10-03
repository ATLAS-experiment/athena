#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction with ACTS, no pileup
# art-input: mc21_14TeV:mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14697
# art-input-nfiles: 1
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_ambi_shifter_last

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_CKF_ITk.xml
dcubeXmlTechEff=dcube_IDPVMPlots_ACTS_CKF_ITk_techeff.xml
ref_idpvm_athena=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/InDetPhysValMonitoring/ReferenceHistograms/physval_run4_ttbar0PU_reco_r25.root
n_events=1000

# search in $DATAPATH for matching file
dcubeXmlAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXml -print -quit 2>/dev/null)
dcubeXmlTechEffAbsPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 1 -name $dcubeXmlTechEff -print -quit 2>/dev/null)
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
    if [[ ("${name}" == "dcube-ckf-ambi" || "${name}" == "dcube-ckf-athena" || "${name}" == "dcube-ambi-greedy-scored") && ${rc} -ne 255 ]]; then
        rc=0
    fi
    echo "art-result: $rc ${name}"
    return $rc
}

# Run with Athena ambi. resolution
run "Reconstruction-ckf" \
    Reco_tf.py --CA \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --preExec 'flags.Acts.doMonitoring=True; flags.Tracking.writeExtendedSi_PRDInfo=True; flags.Tracking.doStoreSiSPSeededTracks=True; flags.Tracking.ITkActsValidateTracksPass.storeSiSPSeededTracks=True;' \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ckf.root \
    --maxEvents ${n_events}

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.CKF
mv acts-expert-monitoring.root acts-expert-monitoring.ckf.root

# don't stop right away on an ERROR message ($?=68)
if [[ $reco_rc != 0 && $reco_rc != 68 ]]; then
    exit $reco_rc
fi

run "IDPVM-ckf" \
    runIDPVM.py \
    --filesInput AOD.ckf.root \
    --outputFile idpvm.ckf.root \
    --OnlyTrackingPreInclude \
    --doTightPrimary \
    --doHitLevelPlots \
    --doTechnicalEfficiency \
    --doExpertPlots \
    --validateExtraTrackCollections "SiSPSeededTracksActsValidateTracksTrackParticles"

ckf_rc=$?

# Run with ACTS ambi. resolution
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

if [[ $reco_rc != 0 && $reco_rc != 68 ]]; then
    exit $reco_rc
fi

run "IDPVM-ambi" \
    runIDPVM.py \
    --filesInput AOD.ambi.root \
    --outputFile idpvm.ambi.root \
    --OnlyTrackingPreInclude \
    --doTightPrimary \
    --doHitLevelPlots \
    --doExpertPlots

ambi_rc=$?
if [ $ckf_rc != 0 ]; then
    exit_rc=$ckf_rc
else
    exit_rc=$ambi_rc
fi
if [[ $ckf_rc != 0 && $ambi_rc != 0 ]]; then
    exit $exit_rc
fi

run "Reconstruction-ambi-scored" \
    Reco_tf.py \
    --steering doRAWtoALL \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateResolvedTracksFlags" \
    --preExec "flags.Acts.doMonitoring=True; \
               from ActsConfig.ActsConfigFlags import AmbiguitySolverStrategy; \
               flags.Acts.AmbiguitySolverStrategy = AmbiguitySolverStrategy.ScoreBased;" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${ArtInFile} \
    --outputAODFile AOD.ambi.scored.root \
    --perfmon fullmonmt \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?

mv log.RAWtoALL log.RAWtoALL.AMBI.SCORED
mv acts-expert-monitoring.root acts-expert-monitoring.ambi.scored.root

if [[ $reco_rc != 0 && $reco_rc != 68 ]]; then
    exit $reco_rc
fi

run "IDPVM-ambi-scored" \
    runIDPVM.py \
    --filesInput AOD.ambi.scored.root \
    --outputFile idpvm.ambi.scored.root \
    --OnlyTrackingPreInclude \
    --doTightPrimary \
    --doHitLevelPlots \
    --doExpertPlots

ambi_scored_rc=$?
if [ $ambi_scored_rc != 0 ]; then
    exit $ambi_scored_rc
fi


echo "download latest result..."
art.py download --user=artprod --dst="$lastref_dir" "$ArtPackage" "$ArtJobName"
ls -la "$lastref_dir"

if [ $ckf_rc == 0 ]; then
    run "dcube-ckf-last" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ckf_shifter_last \
        -c ${dcubeXmlTechEffAbsPath} \
        -r ${lastref_dir}/idpvm.ckf.root \
        idpvm.ckf.root

    # Compare performance WRT legacy Athena
    run "dcube-ckf-athena" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ckf_athena \
        -c ${dcubeXmlTechEffAbsPath} \
        -r ${ref_idpvm_athena} \
        -M "acts" \
        -R "athena" \
        idpvm.ckf.root
fi

if [ $ambi_rc == 0 ]; then
    run "dcube-ambi-last" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ambi_shifter_last \
        -c ${dcubeXmlAbsPath} \
        -r ${lastref_dir}/idpvm.ambi.root \
        idpvm.ambi.root
fi

if [[ $ckf_rc == 0 && $ambi_rc == 0 ]]; then
    # Compare performance w/ and w/o ambi. resolution
    run "dcube-ckf-ambi" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ckf_ambi \
        -c ${dcubeXmlAbsPath} \
        -r idpvm.ckf.root \
        -M "ckf" \
        -R "ambi" \
        idpvm.ambi.root
fi

if [ $ambi_scored_rc == 0 ]; then
    run "dcube-ambi-scored-last" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ambi_scored_shifter_last \
        -c ${dcubeXmlAbsPath} \
        -r ${lastref_dir}/idpvm.ambi.scored.root \
        idpvm.ambi.scored.root
fi

if [[ $ambi_rc == 0 && $ambi_scored_rc == 0 ]]; then
    run "dcube-ambi-greedy-scored" \
        $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
        -p -x dcube_ambi_greedy_scored \
        -c ${dcubeXmlAbsPath} \
        -r idpvm.ambi.root \
        -M "ScoreBased" \
        -R "Greedy" \
        idpvm.ambi.scored.root
fi

exit $exit_rc

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
