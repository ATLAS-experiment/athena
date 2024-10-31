#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger athenaHLT test of the PhysicsP1_pp_run3_v1 menu, with Run2 EB data as input
# art-type: build
# art-include: main/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps


##################################################
# Test definition
##################################################

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'data_run2_EB'
ex.args = '-c "setMenu=\'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale\';doL1Sim=True;flags.GeoModel.AtlasVersion=\'ATLAS-R2-2016-01-00-01\';flags.IOVDb.GlobalTag=\'CONDBR2-HLTP-2018-04\';"'
ex.args += ' --dump-config-exit'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
