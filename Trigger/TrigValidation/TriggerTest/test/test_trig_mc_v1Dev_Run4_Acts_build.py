#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test with full Run 4 menu using Acts tracking 
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Generate configuration run file
run = MCBuildStep(
    menu='MC_pp_run4_v1',
    global_tag=defaultConditionsTags.RUN4_MC,
    mc_campaign='Campaigns.PhaseIIPileUp200'
)

run.input = 'ttbar_pu200_Run4'

actsTracking = True

run.flags += [f'Trigger.useActsTracking={actsTracking}',
             f'Acts.GsfRefitActs={actsTracking}',
             f'Acts.useCache={actsTracking}',
             'Tracking.doITkFastTracking=True',
             'Trigger.doRuntimeNaviVal=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
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
