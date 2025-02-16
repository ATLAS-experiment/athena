#!/usr/bin/bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

extraArgs=$1
ignore_pattern=$2

n_events=1
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1


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
    --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
    --ignorePatterns "${ignore_pattern}" \
    --inputRDOFile ${input_rdo} \
    --outputAODFile AOD.pool.root \
    --maxEvents ${n_events} \
    --multithreaded

