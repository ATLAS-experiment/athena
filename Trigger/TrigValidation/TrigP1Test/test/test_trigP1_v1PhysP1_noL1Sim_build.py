#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the PhysicsP1_pp_run3_v1 menu
# art-type: build                                                                  
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data'
ex.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_run3_v1_HLTReprocessing_prescale"',
            'Trigger.forceEnableAllChains=True',
            'Trigger.disableChains=\'["HLT_cosmic_id_L1MU3V_EMPTY","HLT_cosmic_id_L1MU8VF_EMPTY"]\'' # Temporary workaround for ATR-25459
            ]

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
