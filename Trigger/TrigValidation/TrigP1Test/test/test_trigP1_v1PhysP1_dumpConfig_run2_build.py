#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of the PhysicsP1_pp_run3_v1 menu, with Run2 EB data as input
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps


##################################################
# Test definition
##################################################

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data_run2_EB'
ex.flags = ['Trigger.triggerMenuSetup="PhysicsP1_pp_run3_v1_HLTReprocessing_prescale"',
            'Trigger.doLVL1=True',
            'GeoModel.AtlasVersion="ATLAS-R2-2016-01-00-01"',
            'IOVDb.GlobalTag="CONDBR2-HLTP-2018-04"']
ex.args = '--dump-config-exit'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
