#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar' # TODO restore to 'pbpb', MR !68783
ex.threads = 1
ex.flags = ['Trigger.triggerMenuSetup="Dev_HI_run3_v1_TriggerValidation_prescale"',
            'IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-05-03"',
            'Trigger.doRuntimeNaviVal=True',
            'Trigger.L1.Menu.doHeavyIonTobThresholds=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

# Add a step comparing counts against a reference
chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_v1DevHI_build.new'
refcomp = CheckSteps.ChainCompStep("CountRefComp")
refcomp.input_file = 'ref_mc_v1DevHI_build.new'
refcomp.args += ' --patch'
refcomp.reference_from_release = True # installed from TriggerTest/share
refcomp.required = False # Final exit code doesn't depend on this step (disabled in the main branch only)
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, refcomp)

import sys
sys.exit(test.run())
