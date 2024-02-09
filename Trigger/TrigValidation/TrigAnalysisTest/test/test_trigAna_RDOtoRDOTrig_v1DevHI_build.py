#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of the RDOtoRDOTrigger transform with threads=1
# art-type: build
# art-include: main/Athena
# art-include: 23.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

preExec = ';'.join([
  'setMenu=\'Dev_HI_run3_v1_TriggerValidation_prescale\'',
  'doWriteBS=False',
  'doWriteRDOTrigger=True',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
  'flags.Trigger.enableL1CaloPhase1=False',
  'flags.Trigger.L1.Menu.doHeavyIonTobThresholds=True',
])

ex = ExecStep.ExecStep()
ex.type = 'Reco_tf'
ex.input = 'pbpb'
ex.threads = 1
ex.args = '--outputRDO_TRIGFile=RDO_TRIG.pool.root'
ex.args += ' --ignorePatterns "Py:Configurable.+attempt to add a duplicate.+"'
ex.args += ' --preExec="all:{:s};"'.format(preExec)
ex.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-11"'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

chaindump = test.get_step("ChainDump")
chaindump.args = '--json --yaml ref_RDOtoRDOTrig_v1DevHI_build.new'

import sys
sys.exit(test.run())
