#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test for Run4 with single muon
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Phase II No-Pileup MC settings consistent with input file
run = MCBuildStep(
    menu='MC_pp_run4_v1',
    global_tag=defaultConditionsTags.RUN4_MC,
    mc_campaign='Campaign.PhaseIINoPileUp'
)
run.input = 'Single_mu_Run4'

run.flags += [
    'Trigger.doRuntimeNaviVal=True',
    'ITk.doTruth=False',
    'Tracking.doTruth=False',
    'Trigger.enableL1CaloPhase1=False',
]

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
