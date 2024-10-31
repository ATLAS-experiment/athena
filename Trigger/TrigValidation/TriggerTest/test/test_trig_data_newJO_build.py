#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.args = '--CA'
ex.input = 'data'
ex.threads = 1
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'Trigger.doLVL1=True',
            'Trigger.doRuntimeNaviVal=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_data_newJO_build.new'
# In future, when this is primary test will enable strict reference checking 
# The code below does so.
# refcomp = CheckSteps.ChainCompStep("CountRefComp")
# refcomp.input_file = 'ref_data_newJO_build.new'
# refcomp.required = True # Final exit code depends on this step
# CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
