#!/bin/bash
# art-description: Run 4 configuration, ITK only recontruction, all-hadronic ttbar, full pileup, acts activated
# art-type: grid
# art-include: main/Athena
# art-output: *.root
# art-output: *.xml
# art-output: dcube*
# art-html: dcube_acts_shifter_last
# art-athena-mt: 8

lastref_dir=last_results
dcubeXml=dcube_IDPVMPlots_ACTS_SEEDING_EFF_ITk.xml
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=20

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
    # they are different. We do not expect this test to succeed
    [ "${name}" = "dcube-athena-acts" ] && [ $rc -ne 255 ] && rc=0
    echo "art-result: $rc ${name}"
    return $rc
}

export ATHENA_CORE_NUMBER=8

run "Reconstruction-athena" \
    Reco_tf.py \
     --CA \
     --inputRDOFile  ${input_rdo} \
     --outputAODFile AOD.athena.pool.root \
     --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
     --preExec "flags.Tracking.doStoreTrackSeeds=True; \
		flags.Tracking.doStoreSiSPSeededTracks=True; \
		flags.Tracking.writeExtendedSi_PRDInfo=True;" \
     --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD;toAOD=['xAOD::TrackParticleContainer#SiSPSeedSegments*','xAOD::TrackParticleAuxContainer#SiSPSeedSegments*'];cfg.merge(addToAOD(flags,toAOD));" \
     --maxEvents ${n_events} \
     --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-athena" \
    runIDPVM.py \
    --filesInput AOD.athena.pool.root \
    --outputFile idpvm.athena.root \
    --doExpertPlots \
    --doTechnicalEfficiency \
    --OnlyTrackingPreInclude \
    --validateExtraTrackCollections "SiSPSeedSegmentsTrackParticles"

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "Reconstruction-acts" \
    Reco_tf.py \
     --CA \
     --inputRDOFile  ${input_rdo} \
     --outputAODFile AOD.acts.pool.root \
     --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateSeedsFlags" \
     --preExec "flags.Tracking.doStoreTrackSeeds=True; \
		flags.Tracking.doStoreSiSPSeededTracks=True; \
		flags.Tracking.ITkActsValidateSeedsPass.storeTrackSeeds=True;\
		flags.Tracking.ITkActsValidateSeedsPass.storeSiSPSeededTracks=True; \
		flags.Tracking.writeExtendedSi_PRDInfo=True; \
		flags.Tracking.ITkActsValidateSeedsPass.extension=\"\";"\
     --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD;toAOD=['xAOD::TrackParticleContainer#SiSPSeedSegments*','xAOD::TrackParticleAuxContainer#SiSPSeedSegments*'];cfg.merge(addToAOD(flags,toAOD))" \
     --maxEvents ${n_events} \
     --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

run "IDPVM-acts" \
    runIDPVM.py \
    --filesInput AOD.acts.pool.root \
    --outputFile idpvm.acts.root \
    --doExpertPlots \
    --doTechnicalEfficiency \
    --OnlyTrackingPreInclude \
    --validateExtraTrackCollections "SiSPSeedSegmentsTrackParticles"

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
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.acts.root \
    idpvm.acts.root

run "dcube-athena-last" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_shifter_last \
    -c ${dcubeXmlAbsPath} \
    -r ${lastref_dir}/idpvm.athena.root \
    idpvm.athena.root

run "dcube-athena-acts" \
    $ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py \
    -p -x dcube_athena_acts \
    -c ${dcubeXmlAbsPath} \
    -r idpvm.athena.root \
    -M "acts" \
    -R "athena" \
    idpvm.acts.root

echo "Clean up output directory (based on compiler)"
clean_up_outdir.sh ${AtlasBuildBranch} ${AtlasProject} ${AtlasBuildStamp}
