#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

extraArgs=$1
ignore_pattern=$2

n_events=1
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/RDO_HIJING_ITk_lowstat.pool.root

echo "*** Running ACTS reconstruction with extra args: "${extraArgs}

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
    --preExec "flags.Exec.FPE=-1; \
    	       flags.Tracking.doITkFastTracking=False; \
    	       flags.Acts.doAnalysis=True; \
	       flags.Acts.doMonitoring=True; \
    	       flags.DQ.useTrigger=False; \
	       flags.Output.HISTFileName=\"ActsMonitoringOutput.root\"; \
	       ${extraArgs}" \
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${input_rdo} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${n_events} \
    --multithreaded

