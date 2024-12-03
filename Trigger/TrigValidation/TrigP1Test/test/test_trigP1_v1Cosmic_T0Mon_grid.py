#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Test of cosmic P1+Tier0 workflow, runs athenaHLT with Cosmic_run3_v1 menu followed by offline reco and monitoring
# art-type: grid
# art-athena-mt: 4
# art-include: main/Athena
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.new
# art-output: *.json
# art-output: *.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Common import find_file

# HLT step (BS->BS)
hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts/runHLT_standalone.py'
hlt.forks = 1
hlt.threads = 4
hlt.concurrent_events = 4
hlt.input = 'data_cos'
hlt.max_events = 2000
hltPrecommand = ''.join([
  "setMenu='Cosmic_run3_v1';",
  "doCosmics=True;",
  "doL1Sim=True;",
])
hlt.args = f'-c "{hltPrecommand}"'
hlt.args += ' -o output'
hlt.args += ' --dump-config-reload'

# Extract the physics_Main stream out of the BS file with many streams
filter_bs = ExecStep.ExecStep('FilterBS')
filter_bs.type = 'other'
filter_bs.executable = 'trigbs_extractStream.py'
filter_bs.input = ''
filter_bs.args = '-l 0' # data_cos input includes files from multiple LBs, see ATR-26461
filter_bs.args += ' -s Main ' + find_file('*_HLTMPPy_output.*.data')

# Tier-0 reco step (BS->AOD)
tzrecoPreExec = ' '.join([
  "flags.Trigger.triggerMenuSetup=\'Cosmic_run3_v1\';",
  "flags.Trigger.AODEDMSet=\'AODFULL\';",
  "flags.Exec.FPE=500;",
])

tzreco = ExecStep.ExecStep('Tier0Reco')
tzreco.type = 'Reco_tf'
tzreco.threads = 4
tzreco.concurrent_events = 4
tzreco.input = ''
tzreco.explicit_input = True
tzreco.max_events = 2000
tzreco.args = '--inputBSFile=' + find_file('*.physics_Main*._athenaHLT*.data')  # output of the previous step
tzreco.args += ' --outputAODFile=AOD.pool.root'
tzreco.args += ' --geometryVersion=\'ATLAS-R3S-2021-03-00-00\'' # RecExConfig AutoConfiguration use outdated default
tzreco.args += ' --conditionsTag=\'CONDBR2-BLKPA-2022-08\''     # RecExConfig AutoConfiguration use outdated default
tzreco.args += ' --preExec="{:s}"'.format(tzrecoPreExec)
tzreco.args += ' --CA'

# Tier-0 monitoring step (AOD->HIST)
tzmon = ExecStep.ExecStep('Tier0Mon')
tzmon.type = 'other'
tzmon.executable = 'Run3DQTestingDriver.py'
tzmon.input = ''
tzmon.args = '--threads=4'
tzmon.args += ' --dqOffByDefault'
# HLT jet monitoring disabled as it requires offline PFlow jets which can't be reconstructed for cosmics
tzmon.args += ' Input.Files="[\'AOD.pool.root\']" DQ.Steering.doHLTMon=True DQ.Steering.HLT.doJet=False Trigger.triggerMenuSetup=\'Cosmic_run3_v1\''

# The full test
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hlt, filter_bs, tzreco, tzmon]
test.check_steps = CheckSteps.default_check_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['HistCount', 'RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
