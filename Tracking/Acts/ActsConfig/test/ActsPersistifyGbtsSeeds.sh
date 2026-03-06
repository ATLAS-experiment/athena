# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=50

# Run Athena
export ATHENA_CORE_NUMBER=8
Reco_tf.py \
    --CA \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.athena.pool.root \
    --outputESDFile ESD.athena.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Tracking.doStoreTrackSeeds=True;flags.Tracking.doStoreSiSPSeededTracks=True;" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD;toAOD=['xAOD::TrackParticleContainer#SiSPSeedSegments*','xAOD::TrackParticleAuxContainer#SiSPSeedSegments*'];cfg.merge(addToAOD(flags,toAOD))" \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run Acts
Reco_tf.py \
    --CA \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.pool.root \
    --outputESDFile ESD.acts.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec 'from ActsConfig.ActsConfigFlags import SeedingStrategy;flags.Acts.SeedingStrategy=SeedingStrategy.Gbts;flags.Acts.doMonitoring=True;flags.Acts.doAnalysis=True;flags.Acts.doAnalysisNtuples=False;flags.DQ.useTrigger=False;flags.Output.HISTFileName="ActsMonitoringOutput.root";flags.Tracking.doStoreTrackSeeds=True;flags.Tracking.doStoreSiSPSeededTracks=True;flags.Tracking.ITkActsValidateSeedsPass.storeTrackSeeds=True;flags.Tracking.ITkActsValidateSeedsPass.storeSiSPSeededTracks=False;' \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD;toAOD=['xAOD::TrackParticleContainer#SiSPSeedSegments*','xAOD::TrackParticleAuxContainer#SiSPSeedSegments*'];cfg.merge(addToAOD(flags,toAOD))" \
    --maxEvents ${n_events} \
    --multithreaded
    
reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "Dumping Athena ESD content"
checkxAOD.py ESD.athena.pool.root

echo "Dumping Acts ESD content"
checkxAOD.py ESD.acts.pool.root
