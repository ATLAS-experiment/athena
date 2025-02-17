#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the unconventional tracking slice in Dev_pp_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-include: 24.0/Athena
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
conditions = defaultConditionsTags.RUN3_MC

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'SlepSlep'
ex.threads = 8
ex.concurrent_events = 8
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'IOVDb.GlobalTag="' + conditions + '"',
            'Trigger.enabledSignatures=[\\\"UnconventionalTracking\\\"]']

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
