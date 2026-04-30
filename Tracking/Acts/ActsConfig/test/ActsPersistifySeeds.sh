#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=1

# Run Athena
export ATHENA_CORE_NUMBER=1
Reco_tf.py \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.athena.pool.root \
    --outputESDFile ESD.athena.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Exec.FPE=-1; \
    	       flags.Tracking.doStoreTrackSeeds=True; \
    	       flags.Tracking.doStoreSiSPSeededTracks=True; \
    	       flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.doStoreTrackSeeds=True;" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; \
    	        toAOD = ['xAOD::TrackParticleContainer#SiSPSeedSegments*', 'xAOD::TrackParticleAuxContainer#SiSPSeedSegments*']; \
    	        cfg.merge(addToAOD(flags, toAOD));" \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run Acts - Seeding config
Reco_tf.py \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.pool.root \
    --outputESDFile ESD.acts.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --preExec "flags.Exec.FPE=-1; \
    	       flags.Tracking.doStoreTrackSeeds=True; \
    	       flags.Tracking.doStoreSiSPSeededTracks=True; \
    	       flags.Tracking.ITkActsPass.storeTrackSeeds=True; \
    	       flags.Tracking.ITkActsPass.storeSiSPSeededTracks=True; \
    	       flags.Tracking.writeExtendedSi_PRDInfo=True;" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; \
    	        toAOD = ['xAOD::TrackParticleContainer#SiSPSeedSegments*', 'xAOD::TrackParticleAuxContainer#SiSPSeedSegments*']; \
    	        cfg.merge(addToAOD(flags, toAOD));" \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

# Run Acts - Ckf conversion config
Reco_tf.py \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.ckf.pool.root \
    --outputESDFile ESD.acts.ckf.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateTracksFlags" \
    --preExec 'flags.Exec.FPE=-2; \
    	       flags.Tracking.writeExtendedSi_PRDInfo=True; \
    	       flags.Tracking.doStoreSiSPSeededTracks=True; \
    	       flags.Tracking.ITkActsValidateTracksPass.storeSiSPSeededTracks=True;' \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; \
    	        toAOD = ['xAOD::TrackParticleContainer#SiSPSeedSegments*', 'xAOD::TrackParticleAuxContainer#SiSPSeedSegments*']; \
    	        cfg.merge(addToAOD(flags, toAOD));" \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "Dumping Athena ESD content"
checkxAOD.py ESD.athena.pool.root

echo "Dumping Acts (seeding) ESD content"
checkxAOD.py ESD.acts.pool.root

echo "Dumping Acts (ckf) ESD content"
checkxAOD.py ESD.acts.ckf.pool.root
