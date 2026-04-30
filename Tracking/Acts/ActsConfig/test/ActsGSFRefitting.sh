#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=5

# Ignore specific error messages from Acts GSF
ignore_pattern=""

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
   --preExec "flags.Exec.FPE=-1;" \
   --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsValidateGSFFlags" \
   --postInclude "ActsConfig.ActsTrackFittingConfig.ActsReFitterAlgCfg" \
   --inputRDOFile ${input_rdo} \
   --outputESDFile ESD.pool.root \
   --outputAODFile AOD.pool.root \
   --maxEvents ${n_events} \
   --multithreaded \
   --ignorePatterns "${ignore_pattern}"
