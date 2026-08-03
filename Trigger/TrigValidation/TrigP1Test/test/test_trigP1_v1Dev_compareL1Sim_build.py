#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: athenaEF test of the Dev_pp_run3_v1 menu, to validate L1Sim
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# art-include: 24.0/Athena                                                       

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigP1Test.TrigP1TestSteps import filterBS
import copy

# STEP 1: write BS with L1Sim
writeBS = ExecStep.ExecStep('writeBS')
writeBS.type = 'athenaEF'
writeBS.args = '-o output'
writeBS.job_options = 'TriggerJobOpts.runHLT'
writeBS.input = 'data'
writeBS.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"',
                 'Trigger.doLVL1=True']

# Extract and decode physics_Main
filterMain = filterBS("Main")

# STEP 2: rerun without L1sim on BS file produced above
rerunBS = ExecStep.ExecStep('rerunBS')
rerunBS.type = 'athenaEF'
rerunBS.job_options = 'TriggerJobOpts.runHLT'
rerunBS.input = ''
rerunBS.args = '-f `find .. -name \'*.physics_Main.*.data\' | tail -n 1`'
rerunBS.workdir = 'test2'
rerunBS.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [writeBS, filterMain, rerunBS]
test.check_steps = CheckSteps.default_check_steps(test)

# Duplicate HistMerge and ChainDump checks for "test2"
chainDump1 = next(chk for chk in test.check_steps if chk.name=='ChainDump')
chainDump1.name='ChainDump1'

chainDump2 = copy.copy(chainDump1)
chainDump2.name='ChainDump2'
chainDump2.workdir = 'test2'

histmerge1 = next(chk for chk in test.check_steps if chk.name=='HistMerge')
histmerge1.name = 'HistMerge1'

histmerge2 = copy.copy(histmerge1)
histmerge2.name = 'HistMerge2'
histmerge2.workdir = 'test2'

# STEP 3: compare chain counts between 1. and 2.
rc = CheckSteps.ChainCompStep("CountRefComp")
rc.input_file = 'test2/chainDump.yml'
rc.reference = 'chainDump.yml'
rc.explicit_reference = True  # Don't check if reference exists at configuration time
rc.required = True

test.check_steps.extend([histmerge2, chainDump2, rc])

import sys
sys.exit(test.run())
