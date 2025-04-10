#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test for Run4 with ttbar mu=200
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
run = ExecStep.ExecStep()
run.type = 'athena'
run.threads = 1
run.input = 'ttbar_pu200_Run4'
run.job_options = 'TriggerJobOpts/runHLT.py'

from AthenaConfiguration.TestDefaults import defaultConditionsTags
run.flags = ['Trigger.doRuntimeNaviVal=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
            f'IOVDb.GlobalTag={defaultConditionsTags.RUN4_MC}']

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
