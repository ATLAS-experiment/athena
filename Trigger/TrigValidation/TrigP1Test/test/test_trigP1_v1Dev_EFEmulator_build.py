#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu with Phase-II DF emulator 
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# Skipping art-output which has no effect for build tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"', 'Trigger.doLVL1=True']  
ex.args = "--use-ef-emulator"

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
