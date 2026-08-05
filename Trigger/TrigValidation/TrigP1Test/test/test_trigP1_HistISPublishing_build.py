#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: athenaEF test with IS and OH publishing
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, PyStep, CheckSteps
from TrigP1Test import TrigP1TestSteps
import re

ex = ExecStep.ExecStep()
ex.type = 'athenaEF'
ex.job_options = 'TrigExamples.TrigExISHistConfig.TrigExISHistCfg'
ex.input = 'data'
ex.args = '-M'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = TrigP1TestSteps.default_check_steps_OHMon(test, 'r0000500306_athenaEF_Histogramming.root:run_500306/lb_-1')

# Make the RootComp step required but only compare histograms of the MonAlg
for t in test.check_steps:
   if isinstance(t, CheckSteps.RootCompStep):
      t.required = True
      t.args += ' --select MonAlg'

# Add IS check step
def _checkIS():
   """Check that IS dump contains the relevant lines"""
   pat = re.compile(r'\s*Flag\s*U8\[80\]\s*1, 2,.*, 79, 80')

   with open('r0000500306_athenaEF_DF.txt') as f:
      found = any(pat.match(line) for line in f)

   if not found:
      print('ERROR Pattern %s not found in %s', pat.pattern, f.name)
      return 1

   return 0

checkIS = PyStep.PyStep(_checkIS, name='CheckIS')
checkIS.required = True
checkIS.depends_on_exec = True
test.check_steps.append(checkIS)

import sys
sys.exit(test.run())
