#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=1


ignore_pattern="Acts.+FindingAlg.+ERROR.+Propagation.+reached.+the.+step.+count.+limit,Acts.+FindingAlg.+ERROR.+Propagation.+failed:.+PropagatorError:..+Propagation.+reached.+the.+configured.+maximum.+number.+of.+steps.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+CombinatorialKalmanFilter.+failed:.+CombinatorialKalmanFilterError:5.+Propagation.+reaches.+max.+steps.+before.+track.+finding.+is.+finished.+with.+the.+initial.+parameters,Acts.+FindingAlg.Acts.+ERROR.+failed.+to.+extrapolate.+track"


export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
  	     flags.Acts.doITkConversion=True; \
	     flags.Acts.doLargeRadius=True; \
	     flags.Acts.doLowPt=True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsWorkflowFlags" \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --outputESDFile ESD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
