#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=2

# Run reconstruction and produce AOD with persistified Acts EDM
export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
  	     flags.Tracking.ITkActsLegacyPass.storeSeparateContainer=True; \
  	     flags.Acts.EDM.PersistifySpacePoints=True; \
	     flags.Acts.EDM.PersistifyTracks=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
  --postExec "cfg.printConfig(withDetails=True, summariseProps=True);" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded

rc=$?
if [ $rc != 0 ]; then
    exit $rc
fi

checkxAOD.py AOD.pool.root

# Check we can retrieve the EDM, and related quantities, with our analysis algorithms
ActsReadEDM.py \
   --filesInput AOD.pool.root -- \
   readClusters=True \
   readSpacePoints=True \
   readTracks=True \
   tracks="ActsLegacyTracks" \
   readTrackParticles=True \
   redoAmbiguity=True \
   trackParticles="InDetActsLegacyTrackParticles"

rc=$?
if [ $rc != 0 ]; then
    exit $rc
fi

# Check we can run IDPVM
runIDPVM.py \
   --filesInput AOD.pool.root \
   --outputFile idpvm.root \
   --doActs \
   --validateExtraTrackCollections "InDetActsLegacy"

