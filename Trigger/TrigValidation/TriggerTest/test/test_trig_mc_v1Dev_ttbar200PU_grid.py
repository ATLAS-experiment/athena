#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena for Run4 with ttbar mu=200
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
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
from AthenaConfiguration.TestDefaults import defaultConditionsTags

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar_pu200_Run4'
ex.threads = 8
ex.concurrent_events = 8
ex.flags = [ 'Trigger.triggerMenuSetup="MC_pp_run4_v1"',
             'ITk.doTruth=False',
             'Tracking.doTruth=False',
            f'IOVDb.GlobalTag={defaultConditionsTags.RUN4_MC}']

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
