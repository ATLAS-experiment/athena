#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=10

# Run Acts
Reco_tf.py \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.pool.root \
    --outputESDFile ESD.acts.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
    --preExec "flags.Exec.FPE=-1;" "flags.Tracking.doStoreTrackSeeds=True;flags.Tracking.doStoreSiSPSeededTracks=True;flags.Tracking.ITkActsLegacyPass.storeTrackSeeds=True;flags.Tracking.ITkActsLegacyPass.storeSiSPSeededTracks=True;flags.Tracking.writeExtendedSi_PRDInfo=False;flags.Tracking.ITkActsLegacyPass.doAthenaSeed=False;flags.Tracking.ITkActsLegacyPass.doActsSeed=True" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; toAOD = ['xAOD::TrackParticleContainer#SiSPSeedSegments*', 'xAOD::TrackParticleAuxContainer#SiSPSeedSegments*']; cfg.merge(addToAOD(flags, toAOD))" \
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

