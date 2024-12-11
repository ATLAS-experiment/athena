#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the tau slice in Dev_pp_run3_v1 menu
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar'
ex.threads = 1
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1"',
            'IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-05-03"',
            'Trigger.enabledSignatures=[\\\"Tau\\\"]']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
