#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1_HLTReprocessing_prescale menu only dumping options for SMK generation for HLT reprocessings
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = ''  # No input file needed to generate config
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"',
            'Trigger.doLVL1=True',
            'Trigger.L1MuonSim.NSWVetoMode=False',
            'Trigger.L1MuonSim.doMMTrigger=False',
            'Trigger.L1MuonSim.doPadTrigger=False',
            'Trigger.L1MuonSim.doStripTrigger=False']
ex.args = '-M --dump-config-exit'
ex.perfmon = False  # Don't want PerfMon in SMK for HLT reprocessing
ex.fpe_auditor = False  # Don't want FPEAuditor in SMK for HLT reprocessing

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
# Only keep relevant checks from the defaults
test.check_steps = [chk for chk in CheckSteps.default_check_steps(test)
                    if type(chk) is CheckSteps.CheckLogStep]
# No log merging because we don't fork - force checking only the mother log
for chk in test.check_steps:
    chk.log_file = 'athenaHLT.log'

import sys
sys.exit(test.run())
