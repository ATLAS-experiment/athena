#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: grid
# art-include: main/Athena
# art-architecture: '#&nvidia'
# If you create a grid version, check art-output in existing grid tests.
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
run = ExecStep.ExecStep()
run.type = 'athena'
run.args = '--CA'
run.threads = 1
run.input = 'Single_mu_Run4'
run.job_options = 'TriggerJobOpts/runHLT.py'
run.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
             'Trigger.doRuntimeNaviVal=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
             'Trigger.enableL1CaloPhase1=False',
             'Trigger.InDetTracking.doGPU=True']

# The full test configuration
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
