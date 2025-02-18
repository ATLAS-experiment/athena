#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_pp_run3_v1 menu with pileup80 ttbar sample
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

from AthenaConfiguration.TestDefaults import defaultConditionsTags
conditions = defaultConditionsTags.RUN3_MC

ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.input = 'ttbar_pu80'
ex.threads = 1
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"',
            'IOVDb.GlobalTag="' + conditions + '"']
# the conditions override is needed because the RDO was produced with a single beamspot
ex.args += ' --postExec \'from IOVDbSvc.IOVDbSvcConfig import addOverride; cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002"));\''

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
