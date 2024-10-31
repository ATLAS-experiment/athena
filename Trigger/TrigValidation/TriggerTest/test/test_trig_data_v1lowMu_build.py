#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the lowMu menu
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data_lowmu'
ex.threads = 1
precommand = ''.join([
  "setMenu=\'PhysicsP1_pp_lowMu_run3_v1\';",
  "doL1Sim=True;",
  "doWriteBS=False;",
  "doWriteRDOTrigger=True;",
  "from AthenaConfiguration.AllConfigFlags import ConfigFlags;",
  "ConfigFlags.Trigger.L1.doAlfaCtpin=True;",
])
ex.args = '-c "{:s}"'.format(precommand)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
