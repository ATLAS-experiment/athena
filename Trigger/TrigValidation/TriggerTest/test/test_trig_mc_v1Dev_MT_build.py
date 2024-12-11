#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_pp_run3_v1 menu with 2 slots and 2 threads
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar'
ex.threads = 2
ex.concurrent_events = 2
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"',
            'IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-05-03"']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Use RootComp reference from test_trig_mc_v1Dev_build
test.get_step('RootComp').ref_test_name = 'trig_mc_v1Dev_build'

import sys
sys.exit(test.run())
