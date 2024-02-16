#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test Run 2 to Run 3 EDM Conversion (Navigation & config metadata)
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = 'data_run2_reco_run3_AOD'
aod2daod.explicit_input = True
aod2daod.args = ' --formats=PHYS --outputDAODFile=DAOD.pool.root --CA True'
aod2daod.args += ' --preExec="flags.Trigger.doEDMVersionConversion=True;"'
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
import sys
sys.exit(test.run())
