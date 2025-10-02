#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of the RDOtoRDOTrigger transform with Dev menu
# art-type: grid
# art-include: main/Athena
# art-input: valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4369_s4370_r16083_tid42189392_00
# art-input-nfiles: 1
# art-athena-mt: 8
# art-architecture: '#&nvidia'
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags

preExec = ';'.join([
  'flags.CaloRecGPU.GlobalFlags.UseCaloRecGPU=True',
  'flags.Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
  'flags.Trigger.AODEDMSet=\'AODFULL\'',
  'flags.Trigger.doRuntimeNaviVal=True', # Perform runtime graph vaidation in this test
])

ex = ExecStep.ExecStep()
ex.type = 'Reco_tf'
ex.input = 'ttbar'
ex.max_events = 500
ex.threads = 8
ex.concurrent_events = 8
ex.args = '--outputRDO_TRIGFile=RDO_TRIG.pool.root'
ex.args += ' --preExec="all:{:s};"'.format(preExec)
ex.args += ' --preInclude "all:Campaigns.MC23e"'
ex.args += ' --CA'
ex.args += f' --conditionsTag "default:{defaultConditionsTags.RUN3_MC}"'

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
