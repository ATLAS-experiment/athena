#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
n_events=1

ignore_pattern=""

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
  	     flags.Acts.doITkConversion=True; \
	     flags.Acts.doLargeRadius=True; \
	     flags.Acts.doLowPt=True; \
	     flags.Detector.GeometryHGTD=True; \
             flags.HGTD.doActs=True; \
	     flags.Reco.EnableHGTDExtension=True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
