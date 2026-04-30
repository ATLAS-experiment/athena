#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=5

# Note: To fit from PrepRawData instead of RIO_OnTrack:
#  1) use the --preExec option: flags.Acts.fitFromPRD=True (in addition to all the other ones needed)
#  2) in addition to only use the --postInclude option:  ActsConfig.ActsTrackFittingConfig.forceITkActsReFitterAlgCfg

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
   --preExec "flags.Exec.FPE=-1;" \
   --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
   --postInclude "ActsConfig.ActsTrackFittingConfig.ActsReFitterAlgCfg" \
   --inputRDOFile ${input_rdo} \
   --outputESDFile ESD.pool.root \
   --outputAODFile AOD.pool.root \
   --maxEvents ${n_events} \
   --multithreaded
