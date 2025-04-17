#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the Dev_pp_run3_v1 menu with pileup80 ttbar sample
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TriggerTest.MCExecStep import MCBuildStep
from TrigValTools.TrigValSteering import Test, CheckSteps

ex = MCBuildStep(menu='Dev_pp_run3_v1_TriggerValidation_prescale')
ex.input = 'ttbar_pu80'
# the conditions override is needed because the RDO was produced with a single beamspot
ex.args += ' --postExec \'from IOVDbSvc.IOVDbSvcConfig import addOverride; cfg.merge(addOverride(flags, "/Indet/Beampos", "IndetBeampos-RunDep-MC21-BestKnowledge-002"));\''

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
