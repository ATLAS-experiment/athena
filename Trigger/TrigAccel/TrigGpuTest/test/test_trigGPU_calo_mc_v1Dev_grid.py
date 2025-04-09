#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena CaloGPU test of the Dev_pp_run3_v1 menu
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 8
# art-architecture: '#&nvidia'
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

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar'
ex.threads = 8
ex.concurrent_events = 8
ex.flags = ['CaloRecGPU.GlobalFlags.UseCaloRecGPU=True',
            'IOVDb.GlobalTag="OFLCOND-MC23-SDR-RUN3-05"',
            'Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"']

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
