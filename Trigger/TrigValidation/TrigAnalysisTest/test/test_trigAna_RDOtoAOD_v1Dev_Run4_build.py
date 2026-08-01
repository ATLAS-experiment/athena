#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD with threads=1
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run4_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
])

conditions = defaultConditionsTags.RUN4_MC

rdo2aod = ExecStep.ExecStep()
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar_pu200_Run4'
rdo2aod.threads = 1
rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
rdo2aod.args += ' --preInclude "all:Campaigns.MC23PhaseIIPileUp200"'
rdo2aod.args += f' --preExec "all:{preExec};"'
rdo2aod.args += ' --conditionsTag "default:' + conditions + '"'
rdo2aod.timeout = 5400 # default = 3600 s

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [rdo2aod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
