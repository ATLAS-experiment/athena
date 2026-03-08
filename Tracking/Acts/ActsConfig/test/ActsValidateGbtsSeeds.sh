#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=10

Reco_tf.py --CA \
  --preExec "flags.Exec.FPE=-2;from ActsConfig.ActsConfigFlags import SeedingStrategy;flags.Acts.SeedingStrategy=SeedingStrategy.Gbts;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile test.AOD.pool.root  \
  --maxEvents ${n_events}
