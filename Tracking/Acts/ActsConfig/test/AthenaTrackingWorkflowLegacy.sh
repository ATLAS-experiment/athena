#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# This test schedules both Athena and Acts workflows in parallel
# For Acts we schedule both primary and secondary passes without the caching mechanism

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5

ignore_pattern=""

export ATHENA_CORE_NUMBER=1

Reco_tf.py \
  --multithreaded True \
  --preExec "flags.Exec.FPE=-1; \
	     flags.Scheduler.CheckDependencies=True; \
	     flags.Scheduler.ShowDataDeps=True; \
	     flags.Scheduler.ShowDataFlow=True; \
	     flags.Scheduler.ShowControlFlow = True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingRecoPreInclude,ActsConfig.ActsCIFlags.athenaLegacyTrackingFlags" \
  --ignorePatterns "${ignore_pattern}" \
  --conditionsTag ${conditions_tag} \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}
