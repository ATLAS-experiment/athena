#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: athenaEF test using simulated BS from RDOtoBS
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc14-opt
# Skipping art-output which has no effect for build tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

# RDO -> BS step
rdo2bs = ExecStep.ExecStep('RDOtoBS')
rdo2bs.type = 'Reco_tf'
rdo2bs.input = 'ttbar_pu200_Run4'
rdo2bs.max_events = 1
rdo2bs.args += ' --outputBSFile=created.BS'
rdo2bs.args += ' --preExec="ConfigFlags.Detector.EnableITkStrip=False"'

ex = ExecStep.ExecStep()
ex.type = 'athenaEF'
ex.job_options = 'TriggerJobOpts.runHLT'
ex.input = ''
ex.args += '--file created.BS'
ex.flags = ['Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"', 'Trigger.doLVL1=True',
            'Trigger.enabledSignatures=[]', 'IOVDb.GlobalTag="{defaultConditionsTags.RUN4_MC}"',
            'GeoModel.AtlasVersion="ATLAS-P2-RUN4-04-00-00"', 'GeoModel.Align.Dynamic=False', 'Input.isMC=True']

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [rdo2bs, ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
