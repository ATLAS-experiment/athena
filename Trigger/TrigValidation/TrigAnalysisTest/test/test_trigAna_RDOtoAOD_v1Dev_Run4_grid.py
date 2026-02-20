#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD with threads=8
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-input: group.trig-hlt.mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583
# art-input-nfiles: 8
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
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

preExec = ';'.join([
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run4_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
  'flags.ITk.doTruth=False',
  'flags.Tracking.doTruth=False',
])

conditions = defaultConditionsTags.RUN4_MC

rdo2aod = ExecStep.ExecStep()
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar_pu200_Run4'
rdo2aod.max_events = 800
rdo2aod.threads = 8
rdo2aod.concurrent_events = 8
rdo2aod.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
rdo2aod.args += ' --CA "all:True"'
rdo2aod.args += ' --preExec="all:{:s};"'.format(preExec)
rdo2aod.args += ' --conditionsTag "default:' + conditions + '"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [rdo2aod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
