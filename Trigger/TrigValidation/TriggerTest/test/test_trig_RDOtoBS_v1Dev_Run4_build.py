#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: athena test using simulated BS from RDOtoBS
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.

from TrigValTools.TrigValSteering import Test, ExecStep, PyStep, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
import os
from contextlib import suppress

#Remove created.BS to avoid complaints
def cleanup():
   with suppress(FileNotFoundError):
      os.remove("created.BS")

ex_rm = PyStep.PyStep(cleanup)

# RDO -> BS step
rdo2bs = ExecStep.ExecStep('RDOtoBS')
rdo2bs.type = 'Reco_tf'
rdo2bs.input = 'ttbar_pu200_Run4'
rdo2bs.max_events = 1
rdo2bs.args += ' --outputBSFile=created.BS'
rdo2bs.args += ' --preExec="flags.Detector.EnableITkStrip=False"'

# BSRDO -> RAW step
ex = ExecStep.ExecStep('BSRDOtoRAW')
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.threads = 1 
ex.input = ''
ex.args += '--filesInput created.BS'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run4_v1"', 'Trigger.doLVL1=True', 'Trigger.EDMVersion=4',
            'Trigger.enabledSignatures=[]', f'IOVDb.GlobalTag="{defaultConditionsTags.RUN4_MC}"',
            f'GeoModel.AtlasVersion="{defaultGeometryTags.RUN4}"', 'GeoModel.Align.Dynamic=False', 'Input.isMC=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_rm, rdo2bs, ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
