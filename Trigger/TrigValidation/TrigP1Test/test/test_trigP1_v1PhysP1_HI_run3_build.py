#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Test of HI data 2023 workflow, runs athenaHLT with PhysP1 HI menu
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena

import sys
from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_HI_run3_v1'

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data_hi_2023'
ex.flags = [
    f'Trigger.triggerMenuSetup="{triggermenu}"', 'Trigger.doLVL1=True', 'Trigger.doZDC=True', 'Trigger.doTRT=True',
    'Input.ProjectName="data23_hi"'
]
ex.fpe_auditor = True
ex.max_events = -1
ex.args = ''

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

sys.exit(test.run())
