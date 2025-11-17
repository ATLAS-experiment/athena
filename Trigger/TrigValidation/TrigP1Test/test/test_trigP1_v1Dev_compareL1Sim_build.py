#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu, to validate L1Sim
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigP1Test.TrigP1TestSteps import filterBS

# STEP 1: write BS with L1Sim
writeBS = ExecStep.ExecStep('writeBS')
writeBS.type = 'athenaHLT'
writeBS.args = '-o output'
writeBS.job_options = 'TriggerJobOpts.runHLT'
writeBS.input = 'data'
writeBS.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"',
            'Trigger.doLVL1=True',
            'Trigger.L1MuonSim.NSWVetoMode=False',
            'Trigger.L1MuonSim.doMMTrigger=False',
            'Trigger.L1MuonSim.doPadTrigger=False',
            'Trigger.L1MuonSim.doStripTrigger=False']

# Extract and decode physics_Main
filterMain = filterBS("Main")

# STEP 2: rerun without L1sim on BS file produced above
rerunBS = ExecStep.ExecStep('rerunBS')
rerunBS.type = 'athenaHLT'
rerunBS.job_options = 'TriggerJobOpts.runHLT'
rerunBS.input = ''
rerunBS.args = '-f `find .. -name \'*Main*_athenaHLT*.data\' | tail -n 1`'
rerunBS.workdir = 'test2'
rerunBS.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"']


test = Test.Test()
test.art_type = 'build'
test.exec_steps = [writeBS, filterMain, rerunBS]
chainDump1 = next(chk for chk in CheckSteps.default_check_steps(test) if chk.name in 'ChainDump')
chainDump1.name='ChainDump1'

chainDump2 = next(chk for chk in CheckSteps.default_check_steps(test) if chk.name in 'ChainDump')
chainDump2.name='ChainDump2'
chainDump2.workdir = 'test2'
test.check_steps = [chainDump1, chainDump2]


# STEP 3: compare chain counts between 1. and 2.
rc = CheckSteps.ChainCompStep("CountRefComp")
rc.input_file = 'test2/chainDump.yml'
rc.reference = 'chainDump.yml'
rc.explicit_reference = True  # Don't check if reference exists at configuration time
rc.required = True
CheckSteps.add_step_after_type(test.check_steps, CheckSteps.ChainDumpStep, rc)


import sys
sys.exit(test.run())
