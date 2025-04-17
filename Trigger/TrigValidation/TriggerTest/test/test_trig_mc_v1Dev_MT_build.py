#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_pp_run3_v1 menu with 2 slots and 2 threads
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps

ex = MCBuildStep(menu='Dev_pp_run3_v1_TriggerValidation_prescale')
ex.input = 'ttbar'

ex.threads = 2
ex.concurrent_events = 2

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Use RootComp reference from test_trig_mc_v1Dev_build
test.get_step('RootComp').ref_test_name = 'trig_mc_v1Dev_build'

import sys
sys.exit(test.run())
