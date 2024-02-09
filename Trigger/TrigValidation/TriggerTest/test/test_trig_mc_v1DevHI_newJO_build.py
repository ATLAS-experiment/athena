#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.args = '--CA'
ex.threads = 1
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar' # TODO restore to once HI MC has supercells 'pbpb'
ex.flags = ['Trigger.triggerMenuSetup="Dev_HI_run3_v1_TriggerValidation_prescale"',
            'Trigger.doRuntimeNaviVal=True',
            'Trigger.L1.Menu.doHeavyIonTobThresholds=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_v1DevHI_build.new'

import sys
sys.exit(test.run())
