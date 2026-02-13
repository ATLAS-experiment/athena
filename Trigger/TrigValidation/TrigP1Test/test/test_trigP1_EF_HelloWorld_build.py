#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: HelloWorld in AthenaEF
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# Skipping art-output which has no effect for build tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaEF'
ex.job_options = 'AthExHelloWorld.HelloWorldConfig.HelloWorldCfg'
ex.input = 'data'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
