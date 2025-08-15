#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger test for Run4 with single muon
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Generate configuration run file
run = ExecStep.ExecStep()
run.type = 'athena'
run.threads = 1
#run.input = 'ttbar_pu200_Run4'
run.input = 'Single_mu_Run4'                                     #need simpler dataset until egamma run-time completely sorted out
run.job_options = 'TriggerJobOpts/runHLT.py'

from AthenaConfiguration.TestDefaults import defaultConditionsTags
run.flags = ['Trigger.enabledSignatures=["Muon"]',  #list signatures temporarily 
             'Trigger.EFTrackPipeline="F100"',
             'Trigger.doRuntimeNaviVal=True',
             'Trigger.useActsTracking=True',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
             'FPGADataPrep.xclbin="/eos/project-a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F110/kernels.hw_physicsRelease_v03.xclbin"',
             'FPGADataPrep.doF110=True',
             'FPGADataPrep.bdfID=0000:01:00.1',
             f'IOVDb.GlobalTag={defaultConditionsTags.RUN4_MC}',
             ]

# The full test configuration
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [run]
check_log = CheckSteps.CheckLogStep('CheckLog')
check_log.log_file = run.get_log_file_name()
test.check_steps = [check_log]

import sys
sys.exit(test.run())
