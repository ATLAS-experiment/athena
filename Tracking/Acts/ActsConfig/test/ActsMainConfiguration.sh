#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

extraArgs=$1
ignore_pattern=$2

n_events=1
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")

echo "*** Running ACTS reconstruction with extra args: "${extraArgs}

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
    --preExec "flags.Exec.FPE=-1; \
    	       flags.Acts.doAnalysis=True; \
	       flags.Acts.doMonitoring=True; \
    	       flags.DQ.useTrigger=False; \
	       flags.Output.HISTFileName=\"ActsMonitoringOutput.root\"; \
	       ${extraArgs}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude" \
    --ignorePatterns "${ignore_pattern}" \
    --conditionsTag ${conditions_tag} \
    --inputRDOFile ${input_rdo} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${n_events} \
    --multithreaded
