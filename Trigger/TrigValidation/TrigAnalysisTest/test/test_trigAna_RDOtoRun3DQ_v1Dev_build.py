#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD followed by HLT monitoring step with Run-3 DQ framework
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'ESD\'',
])

rdo2aod = ExecStep.ExecStep('RDOtoAOD')
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar'
rdo2aod.threads = 4
rdo2aod.concurrent_events = 4
rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG" --valid=True'
rdo2aod.args += ' --CA "all:True"'
rdo2aod.args += ' --preExec="all:{:s};"'.format(preExec)
rdo2aod.args += ' --preInclude "all:Campaigns.MC23e"'
rdo2aod.args += ' --conditionsTag "default:OFLCOND-MC23-SDR-RUN3-05-03"'

dq = ExecStep.ExecStep('Run3DQ')
dq.type = 'other'
dq.executable = 'Run3DQTestingDriver.py'
dq.input = ''
# very bad scaling vs number of threads, the more threads, the slower... (ATR-29610)
dq.args = '--threads=2'
dq.args += ' --dqOffByDefault'
dq.args += ' Input.Files="[\'AOD.pool.root\']" DQ.Steering.doHLTMon=True Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\''

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [rdo2aod,dq]
test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
