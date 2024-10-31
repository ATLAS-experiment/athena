#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.args = '--CA'
ex.input = 'ttbar'
ex.threads = 1
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'Trigger.doRuntimeNaviVal=True',
            'Output.doWriteRDO=False'] #TODO enable once fixes issue with missing containers

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_mc_newJO_build.new'

# Change RegTest pattern
#regtest = test.get_step('RegTest')
#regtest.regex = r'TrigSignatureMoniMT\s*INFO\sHLT_.*|TrigSignatureMoniMT\s*INFO\s-- #[0-9]+ (Events|Features).*'

import sys
sys.exit(test.run())
