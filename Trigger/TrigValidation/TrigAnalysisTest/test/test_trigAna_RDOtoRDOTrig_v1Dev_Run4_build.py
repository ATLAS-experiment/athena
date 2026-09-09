#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG with threads=1
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run4_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
])

ex = ExecStep.ExecStep()
ex.type = 'Reco_tf'
ex.input = 'ttbar_pu200_Run4'
ex.threads = 1
ex.args = '--outputRDO_TRIGFile=RDO_TRIG.pool.root'
ex.args += ' --steering "doRDO_TRIG"'
ex.args += ' --preInclude "all:Campaigns.MC23PhaseIIPileUp200"'
ex.args += f' --preExec "all:{preExec};"'
ex.args += f' --conditionsTag "default:{defaultConditionsTags.RUN4_MC}"'
ex.flags+=   ['Tracking.doITkFastTracking=False',
              'Tracking.doPixelDigitalClustering=False',
              ]
ex.timeout = 5400 # default = 3600 s

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Add a step comparing counts against a reference
chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_RDOtoRDOTrig_v1Dev_Run4_build.new'
refcomp = CheckSteps.ChainCompStep("CountRefComp")
refcomp.input_file = 'ref_RDOtoRDOTrig_v1Dev_Run4_build.new'
refcomp.args += ' --patch'
refcomp.reference_from_release = True # installed from TrigAnalysisTest/share
refcomp.required = True # Final exit code depends on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
