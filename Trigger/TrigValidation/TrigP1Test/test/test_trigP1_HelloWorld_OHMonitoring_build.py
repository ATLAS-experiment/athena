#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: HelloWorld in athenaEF with OH monitoring
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep
from TrigP1Test import TrigP1TestSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaEF'
ex.job_options = 'AthExHelloWorld.HelloWorldConfig.HelloWorldCfg'
ex.input = 'data'
ex.args = '-M'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = TrigP1TestSteps.default_check_steps_OHMon(test, 'r0000500306_athenaEF_Histogramming.root:run_500306/lb_-1')

import sys
sys.exit(test.run())
