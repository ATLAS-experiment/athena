#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# This test schedules both Acts workflows for ScoreBasedAmbiguity
# For Acts we schedule both primary and secondary passes without the caching mechanism

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=5

Reco_tf.py \
  --preExec "flags.Exec.FPE=-1;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsScoreBasedAmbiguityWorkflowFlags" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}
