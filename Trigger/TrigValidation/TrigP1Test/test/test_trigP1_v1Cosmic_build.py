#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of the Cosmic_run3_v1 menu on physics_Main stream from a cosmic run, then running BS decoding follows the athenaHLT process
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

##################################################
# Helper functions to build the test steps
##################################################
from TrigP1Test.TrigP1TestSteps import filterBS, decodeBS

##################################################
# Test definition
##################################################

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data_cos'
ex.max_events = 200
ex.flags = ['Trigger.triggerMenuSetup="Cosmic_run3_v1"',
            'Trigger.doLVL1=True',
            'Beam.Type=BeamType.Cosmics']
ex.args = ' -o output'

# Extract and decode physics_Main
filterMain = filterBS("Main")
decodeMain = decodeBS("Main")

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex, filterMain, decodeMain]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
