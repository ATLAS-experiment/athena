#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Test of P1+Tier0 workflow, runs athenaHLT with PhysicsP1_pp_run3_v1 menu followed by offline reco and monitoring (incl. EDM)
# art-type: grid
# art-include: main/Athena
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
from TrigValTools.TrigValSteering.Common import find_file
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# Specify trigger menu once here:
triggermenu = 'Dev_pp_run3_v1_HLTReprocessing_prescale'

# HLT step (BS->BS)
hlt = ExecStep.ExecStep()
hlt.type = 'athenaHLT'
hlt.job_options = 'TriggerJobOpts.runHLT'
hlt.forks = 1
hlt.threads = 8
hlt.concurrent_events = 8
hlt.input = 'data_Main'
hlt.flags = [f'Trigger.triggerMenuSetup="{triggermenu}"',
             'Trigger.doLVL1=True']
hlt.args = '-o output'

# Extract the physics_FTagPEBTLA stream out of the BS file with many streams
filter_bs = ExecStep.ExecStep('FilterBS')
filter_bs.type = 'other'
filter_bs.executable = 'trigbs_extractStream.py'
filter_bs.input = ''
filter_bs.args = '-s FTagPEBTLA ' + find_file('*_HLTMPPy_output.*.data')

# Tier-0 reco step (BS->AOD)
tlarecoPreExec = f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';"

tlareco = ExecStep.ExecStep('Tier0Reco')
tlareco.type = 'Reco_tf'
tlareco.threads = 8
tlareco.concurrent_events = 8
tlareco.input = ''
tlareco.explicit_input = True
tlareco.args = '--inputBSFile=' + find_file('*.physics_FTagPEBTLA*._athenaHLT*.data')  # output of the previous step
tlareco.args += ' --outputDAOD_TLAFTAGPEBFile=DAOD_TLAFTAGPEB.pool.root'
tlareco.args += ' --conditionsTag=\'CONDBR2-BLKPA-2024-03\' --geometryVersion=\'ATLAS-R3S-2021-03-02-00\''
tlareco.args += ' --preExec="{:s}"'.format(tlarecoPreExec)

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
