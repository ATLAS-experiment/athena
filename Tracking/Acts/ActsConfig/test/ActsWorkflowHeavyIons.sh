#!/usr/bin/bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# HI configuration with ITk
input_rdo=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.RDO_RUN4[0])")
# temporaily use default RDO (not HI events, but should be sufficient for testing the config) while problems with the older materail maps in CREST are investigated
#input_rdo=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-01/RDO_HIJING_ITk_lowstat.pool.root
# See ATLASRECTS-8424
conditions_tag=$(python -c "from AthenaConfiguration.TestDefaults import defaultConditionsTags; print(defaultConditionsTags.RUN4_MC)")
n_events=5

export ATHENA_CORE_NUMBER=1
Reco_tf.py \
  --preExec "flags.Exec.FPE=-1; \
             flags.Scheduler.CheckDependencies=True; \
             flags.Scheduler.ShowDataDeps=True; \
             flags.Scheduler.ShowDataFlow=True; \
             flags.Scheduler.ShowControlFlow = True;" \
  --preInclude "Campaigns.MC23PhaseIINoPileUp,InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude,ActsConfig.ActsCIFlags.actsHeavyIonFlags" \
  --postInclude "all:PyJobTransforms.UseFrontier" \
  --postExec "all:cfg.printConfig(withDetails=True, summariseProps=True);" \
  --conditionsTag ${conditions_tag} \
  --inputRDOFile ${input_rdo} \
  --outputAODFile AOD.pool.root \
  --maxEvents ${n_events} \
  --multithreaded
