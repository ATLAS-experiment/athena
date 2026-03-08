#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Informations:
# This script shows how to include the dumper of the tracking geometry detector elements Acts and Athena identifiers
# and their positions. The result will be stored in a csv file (default: transforms.csv)


# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=1

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
  --postInclude "ActsConfig.ActsGeometryConfig.ActsWriteTrackingGeometryTransformsAlgCfg"\
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.validateclusters.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
