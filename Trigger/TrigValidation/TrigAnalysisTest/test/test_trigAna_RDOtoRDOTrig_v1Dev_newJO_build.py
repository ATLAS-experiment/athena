#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of the CA RDOtoRDOTrigger transform with threads=1
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
])

ex = ExecStep.ExecStep()
ex.type = 'Reco_tf'
ex.input = 'ttbar'
ex.threads = 1
ex.args = ' --CA'
ex.args += ' --outputRDO_TRIGFile=RDO_TRIG.pool.root'
ex.args += ' --preExec="all:{:s};"'.format(preExec)

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Use legacy test for root comparison
test.get_step('RootComp').ref_test_name = 'trigAna_RDOtoRDOTrig_v1Dev_build'

# Add a step comparing counts against a reference
chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_RDOtoRDOTrig_v1Dev_build.new'
refcomp = CheckSteps.ChainCompStep("CountRefComp")
refcomp.input_file = 'ref_RDOtoRDOTrig_v1Dev_build.new'
refcomp.args += ' --patch'
refcomp.reference_from_release = True # installed from TrigAnalysisTest/share
refcomp.required = False # Final exit code depends on this step
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
