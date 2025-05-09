#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=10

# Run Acts
Reco_tf.py \
    \
    --inputRDOFile  ${input_rdo} \
    --outputAODFile AOD.acts.pool.root \
    --outputESDFile ESD.acts.pool.root \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
    --preExec "flags.Exec.FPE=-1;" "flags.Tracking.doStoreTrackSeeds=True;flags.Tracking.doTruth=True;flags.Tracking.doStoreSiSPSeededTracks=True;flags.Tracking.ITkActsLegacyPass.storeTrackSeeds=True;flags.Tracking.ITkActsLegacyPass.storeSiSPSeededTracks=True;flags.Tracking.writeExtendedSi_PRDInfo=False;flags.Tracking.ITkActsLegacyPass.doAthenaSeed=False;flags.Tracking.ITkActsLegacyPass.doActsSeed=True" \
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

