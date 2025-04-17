#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the electron/b-physics slice in Dev_pp_run3_v1 menu (input: mix of dielectron samples)
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps

ex = MCBuildStep(menu='Dev_pp_run3_v1',signatures=['Egamma'])
ex.input = 'bphysics_ee'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
