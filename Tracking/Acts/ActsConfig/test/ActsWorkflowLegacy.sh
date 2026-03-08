#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# This test schedules both Athena and Acts workflows in parallel
# For Acts we schedule both primary and secondary passes without the caching mechanism

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=5

ignore_pattern=""

Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
	     flags.Acts.doLargeRadius=True; \
	     flags.Acts.doLowPt=True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
  --outputDAOD_IDTRKVALIDFile DAOD.IDTRKVALID.pool.root \
  --outputDAOD_IDTIDEFile DAOD.CTIDE.pool.root \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}
