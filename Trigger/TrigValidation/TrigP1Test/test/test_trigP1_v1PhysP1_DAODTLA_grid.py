#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# art-description: Test of P1+Tier0 workflow, runs athenaHLT with PhysicsP1_pp_run3_v1 menu followed by offline reco and monitoring (incl. EDM)
# art-type: grid
# art-include: main/Athena
# art-athena-mt: 4
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
from TrigValTools.TrigValSteering.Common import find_file
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale'

# HLT step (BS->BS)
hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts/runHLT_standalone.py'
hlt.forks = 1
hlt.threads = 4
hlt.concurrent_events = 4
hlt.input = 'data_Main'
hlt.args = f'-c "setMenu=\'{triggermenu}\';doL1Sim=True;"'
hlt.args += ' -o output'

# Extract the physics_Main stream out of the BS file with many streams
filter_bs = ExecStep.ExecStep('FilterBS')
filter_bs.type = 'other'
filter_bs.executable = 'trigbs_extractStream.py'
filter_bs.input = ''
filter_bs.args = '-s TLA ' + find_file('*_HLTMPPy_output.*.data')

# Tier-0 reco step (BS->AOD)
tlarecoPreExec = ' '.join([
  f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';",
    "flags.Trigger.DecodeHLT=False;",
    "flags.Trigger.doLVL1=False;",
    "flags.Trigger.L1.doCalo=False;",
    "flags.Trigger.L1.doCTP=False;",
])

tlareco = ExecStep.ExecStep('Tier0Reco')
tlareco.type = 'Reco_tf'
tlareco.threads = 4
tlareco.concurrent_events = 4
tlareco.input = ''
tlareco.explicit_input = True
tlareco.args = '--inputBSFile=' + find_file('*.physics_TLA*._athenaHLT*.data')  # output of the previous step
tlareco.args += ' --outputDAOD_TLAFile=DAOD_TLA.pool.root'
tlareco.args += ' --conditionsTag=\'CONDBR2-BLKPA-2022-08\' --geometryVersion=\'ATLAS-R3S-2021-03-00-00\''
tlareco.args += ' --preExec="{:s}"'.format(tlarecoPreExec)
tlareco.args += ' --CA'

# The full test
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hlt, filter_bs, tlareco]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

# Overwrite default histogram file name for checks
for step in [test.get_step(name) for name in ['RootComp']]:
    step.input_file = 'ExampleMonitorOutput.root'

import sys
sys.exit(test.run())
