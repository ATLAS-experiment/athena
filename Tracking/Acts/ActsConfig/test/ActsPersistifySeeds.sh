#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=1


# Run Acts
Reco_tf.py \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.pool.root \
    --outputESDFile ESD.acts.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --preExec "flags.Exec.FPE=-1; \
               flags.Tracking.doStoreTrackSeeds=True; \
               flags.Tracking.doStoreSiSPSeededTracks=True; \
               flags.Tracking.ITkActsPass.storeTrackSeeds=True; \
               flags.Tracking.ITkActsPass.storeSiSPSeededTracks=True; \
               flags.Scheduler.CheckDependencies=True; \
               flags.Scheduler.ShowDataDeps=True; \
               flags.Scheduler.ShowDataFlow=True; \
               flags.Scheduler.ShowControlFlow = True; \
               flags.Tracking.writeExtendedSi_PRDInfo=True;" \
    --postExec "from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; \
                toAOD = ['xAOD::TrackParticleContainer#SiSPSeedSegments*', 'xAOD::TrackParticleAuxContainer#SiSPSeedSegments*']; \
                cfg.merge(addToAOD(flags, toAOD));" \
    --conditionsTag ${conditions_tag} \
    --maxEvents ${n_events} \
    --multithreaded

reco_rc=$?
if [ $reco_rc != 0 ]; then
    exit $reco_rc
fi

echo "Dumping Acts ESD content"
checkxAOD.py ESD.acts.pool.root
