#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger BS->RDO_TRIG athena test of the Dev_pp_run3_v1 menu
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'data'
ex.threads = 1
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'Trigger.forceEnableAllChains=True',
            'Trigger.doRuntimeNaviVal=True'] # Perform runtime graph vaidation in this test
ex.max_events = 15 # nominal is 20, reduce to avoid occasional timeout

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default MessageCount settings
# We are trying to lower the limits step by step
# Ultimately there should be no per-event messages
msgcount = test.get_step("MessageCount")
msgcount.thresholds = {
  'WARNING': 500,  # Remaining warnings are mostly from ATLASRECTS-3866
  'INFO': 600,
  'other': 20
}
msgcount.required = True # make the test exit code depend on this step

import sys
sys.exit(test.run())
