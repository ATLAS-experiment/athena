#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Test of P1+Tier0 workflow, runs athenaHLT with PhysicsP1_pp_run3_v1 menu followed by offline reco and monitoring (incl. EDM)
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
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
from TrigValTools.TrigValSteering.Common import find_file
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

# Specify trigger menu once here:
triggermenu = 'PhysicsP1_pp_run3_v1_HLTReprocessing_prescale'

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
# cannot use 'find_file' as it only returns the last file matching the pattern
filter_bs.args = '-s DarkJetPEBTLA ' + '`find . -name "*_HLTMPPy_output.*.data"`'

# Tier-0 reco step (BS->AOD)
tlarecoPreExec = f"flags.Trigger.triggerMenuSetup=\'{triggermenu}\';"

tlareco = ExecStep.ExecStep('Tier0Reco')
tlareco.type = 'Reco_tf'
tlareco.threads = 8
tlareco.concurrent_events = 8
tlareco.input = ''
tlareco.explicit_input = True
tlareco.args = '--inputBSFile=' + find_file('*.physics_DarkJetPEBTLA*._athenaHLT*.data')  # output of the previous step
tlareco.args += ' --outputDAOD_TLADJETPEBFile=DAOD_TLADJETPEB.pool.root'
tlareco.args += f' --conditionsTag="{defaultConditionsTags.RUN3_DATA}" --geometryVersion=\'ATLAS-R3S-2021-03-02-00\''
tlareco.args += ' --preExec="{:s}"'.format(tlarecoPreExec)

# The full test
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [hlt, filter_bs, tlareco]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test, input_file='DAOD_TLADJETPEB.pool.root')
test.exec_steps = [t for t in test.exec_steps if not t.name == "TrigEDMChecker"] # TrigEDMChecker fails on TLA DAOD output due to missing HLT containers

import sys
sys.exit(test.run())
