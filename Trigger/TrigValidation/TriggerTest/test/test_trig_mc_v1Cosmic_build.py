#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Cosmic_run3_v1 menu
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from TrigValTools.TrigMCCommonParams import mcDefaults

ex = MCBuildStep(
    menu='Cosmic_run3_v1',
    mc_campaign=mcDefaults.mc_campaign+'NoPileUp',
)
ex.input = 'mc_cosmics'
ex.threads = 1
ex.flags.append('Beam.Type=BeamType.Cosmics')

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
