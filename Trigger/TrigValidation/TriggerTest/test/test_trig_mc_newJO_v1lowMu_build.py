#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the lowMu menu
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

import sys
from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.args = '--CA'
ex.threads = 1
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'minbias'
ex.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_lowMu_run3_v1"',
            'Trigger.disableChains=[]',  # Do not disable any chains
            'Trigger.doRuntimeNaviVal=True',
            'Trigger.L1.doAlfaCtpin=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_v1lowMu_build.new'
sys.exit(test.run())
