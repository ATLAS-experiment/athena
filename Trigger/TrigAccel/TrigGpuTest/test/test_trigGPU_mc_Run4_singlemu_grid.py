#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: grid
# art-include: main/Athena
# art-input: mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4494_r16632
# art-input-nfiles: 1
# art-athena-mt: 8
# art-architecture: '#&nvidia'
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TriggerTest.MCExecStep import MCGridStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Generate configuration run file
run = MCGridStep(
    menu='MC_pp_run4_v1',
    global_tag=defaultConditionsTags.RUN4_MC,
    mc_campaign='Campaigns.PhaseIINoPileUp'
)

run.input = 'Single_mu_Run4'

run.flags += ['Trigger.doRuntimeNaviVal=True',
              'ITk.doTruth=False',
              'Tracking.doTruth=False',
              'Trigger.InDetTracking.doGPU=True',
              'Trigger.enabledSignatures=[\\\"Muon\\\"]']

# The full test configuration
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
