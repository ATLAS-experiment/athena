#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_HI_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
# art-athena-mt: 4
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

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.args = '--CA'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar' # TODO restore to 'pbpb' once it has supercells 
ex.threads = 4
ex.concurrent_events = 4
ex.max_events = 500
ex.flags = ['Trigger.triggerMenuSetup="Dev_HI_run3_v1_TriggerValidation_prescale"',
            'Trigger.doLVL1=True',
            'Output.RDOFileName="RDO_TRIG.pool.root"',
            'Trigger.doRuntimeNaviVal=True',
            'Trigger.L1.Menu.doHeavyIonTobThresholds=True'            
            ]

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
