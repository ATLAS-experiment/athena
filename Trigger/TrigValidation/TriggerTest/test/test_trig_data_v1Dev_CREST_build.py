#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger BS->RDO_TRIG athena test of the Dev_pp_run3_v1 menu using CREST
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Deduce CREST tag
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from TriggerJobOpts.TriggerConfigFlags import trigGlobalTag
flags = initConfigFlags()
flags.Input.isMC = False
globalTag = trigGlobalTag(flags).replace('CONDBR2','CREST')

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'data'
ex.threads = 1
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"',
            'Trigger.doLVL1=True',
            f'IOVDb.GlobalTag="{globalTag}"',
            'IOVDb.CrestServer="http://crest-j23.cern.ch:9090/api-v5.0"']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
