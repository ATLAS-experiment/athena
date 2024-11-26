#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger BS->RDO_TRIG athena test of the Cosmic_run3_v1 menu on express stream from a cosmic run
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

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'data_cos'
ex.threads = 8
ex.concurrent_events = 8
ex.flags = ['Trigger.triggerMenuSetup="Cosmic_run3_v1"',
            'Beam.Type=BeamType.Cosmics',
            'Trigger.forceEnableAllChains=True']

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
