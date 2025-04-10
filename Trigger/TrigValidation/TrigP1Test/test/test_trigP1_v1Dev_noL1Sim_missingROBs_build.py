#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu with detector ROBs removed
# art-type: build                                                                  
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Keep only the L1 ROBs enabled in this test
l1_robs = '[0x770001, 0x760001, 0x7500ac, 0x7500ad, 0x7300a8, 0x7300a9, 0x7300aa, 0x7300ab, 0x910081, 0x910091, 0x941000, 0x941100]'

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'Trigger.forceEnableAllChains=True']
ex.args = '--ros2rob="{{\'ROS-TRIGP1TEST-00\': {:s}}}"'.format(l1_robs)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
