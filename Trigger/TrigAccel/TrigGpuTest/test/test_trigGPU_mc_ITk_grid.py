#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger AthenaMT test running new-style job options
# art-type: grid
# art-include: main/Athena
# art-input: group.trig-hlt.mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8557_s4422_r16128
# art-input-nfiles: 1
# art-athena-mt: 8
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
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
run = ExecStep.ExecStep()
run.type = 'athena'
run.threads = 8
run.input = 'Single_mu_Run4'
run.job_options = 'TriggerJobOpts/runHLT.py'
run.flags = ['Trigger.triggerMenuSetup="MC_pp_run4_v1"',
             'Trigger.doRuntimeNaviVal=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
             'Trigger.InDetTracking.doGPU=True',
             'Trigger.enabledSignatures=[\\\"Muon\\\"]']

# The full test configuration
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
