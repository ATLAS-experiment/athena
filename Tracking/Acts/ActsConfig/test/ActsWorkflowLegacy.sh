#!/usr/bin/bash
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This test schedules both Athena and Acts workflows in parallel
# For Acts we schedule both primary and secondary passes without the caching mechanism

# ttbar mu=200 input
input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1
n_events=5

ignore_pattern=""

Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
   	     flags.Acts.doITkConversion=True; \
	     flags.Acts.doLargeRadius=True; \
	     flags.Acts.doLowPt=True; \
	     flags.Detector.EnableCalo=True;" \
  --preInclude "InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsLegacyWorkflowFlags" \
  --outputDAOD_IDTRKVALIDFile DAOD.IDTRKVALID.pool.root \
  --outputDAOD_IDTIDEFile DAOD.CTIDE.pool.root \
  --ignorePatterns "${ignore_pattern}" \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events}
