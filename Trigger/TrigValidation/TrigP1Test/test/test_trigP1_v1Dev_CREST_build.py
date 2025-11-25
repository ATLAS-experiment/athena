#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: athenaHLT test of the Dev_pp_run3_v1 menu using CREST for conditions
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# Deduce CREST tag
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from TriggerJobOpts.TriggerConfigFlags import trigGlobalTag
flags = initConfigFlags()
flags.Input.isMC = False
globalTag = trigGlobalTag(flags).replace('CONDBR2','CREST')

ex = ExecStep.ExecStep()
ex.type = 'athenaHLT'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = 'data'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_HLTReprocessing_prescale"',
            'Trigger.doLVL1=True',
            'Trigger.L1MuonSim.NSWVetoMode=False',
            'Trigger.L1MuonSim.doMMTrigger=False',
            'Trigger.L1MuonSim.doPadTrigger=False',
            'Trigger.L1MuonSim.doStripTrigger=False',
            f'IOVDb.GlobalTag="{globalTag}"',
            'IOVDb.UseCREST=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
