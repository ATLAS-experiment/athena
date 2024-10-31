#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_pp_run3_v1 menu with pileup80 ttbar sample
# art-type: build
# art-include: main/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT_standalone.py'
ex.input = 'ttbar_pu80'
ex.threads = 1
precommand = ''.join([
   "setMenu='Dev_pp_run3_v1_TriggerValidation_prescale';",
   "doWriteBS=False;",
   "doWriteRDOTrigger=True;",
   "from IOVDbSvc.CondDB import conddb; conddb.addOverride('/Indet/Beampos','IndetBeampos-RunDep-MC21-BestKnowledge-002');"
])
ex.args = '-c "{:s}"'.format(precommand)
# the conditions override is needed because the RDO was produced with a single beamspot

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
