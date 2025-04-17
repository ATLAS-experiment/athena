#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Cosmic_run3_v1 menu
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# art-athena-mt: 8
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TriggerTest.MCExecStep import MCGridStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from TrigValTools.TrigMCCommonParams import mcDefaults

ex = MCGridStep(
    menu='Cosmic_run3_v1',
    mc_campaign=mcDefaults.mc_campaign+'NoPileUp'
)
ex.input = 'mc_cosmics'
ex.flags.append('Beam.Type=BeamType.Cosmics')

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
