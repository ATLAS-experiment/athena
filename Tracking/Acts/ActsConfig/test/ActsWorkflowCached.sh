#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# This test schedules both Athena and Acts workflows in parallel
# For Acts we schedule both primary and secondary passes with the caching mechanism

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5

ignore_pattern=""

Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
	     flags.Acts.useCache=True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
  --ignorePatterns "${ignore_pattern}" \
  --conditionsTag ${conditions_tag} \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}
