#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: athena test using simulated BS from RDOtoBS
# art-type: build
# art-include: main/Athena/x86_64-el9-gcc15-opt
# Skipping art-output which has no effect for build tests.

from TrigValTools.TrigValSteering import Test, ExecStep, PyStep, CheckSteps
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags

nevents = 1

#Remove created.BS to avoid complaints
ex_rm = PyStep.RmStep("created.BS")

rdotrig_ref = ExecStep.ExecStep('RDOtoRDOTrig')
rdotrig_ref.type = 'Reco_tf'
rdotrig_ref.input = 'ttbar'
rdotrig_ref.threads = 1
rdotrig_ref.max_events = nevents
rdotrig_ref.args = '--outputRDO_TRIGFile=RDO_TRIG.ref.pool.root'
rdotrig_ref.args += ' --preInclude "all:Campaigns.MC23e"'
rdotrig_ref.args += f' --conditionsTag \'{defaultConditionsTags.RUN3_MC}\''
rdotrig_ref.flags = [
   'Trigger.triggerMenuSetup=\'Dev_pp_run3_v1_TriggerValidation_prescale\'',
   'Trigger.AODEDMSet=\'AODFULL\''
]

#expert-monitoring.root will be lost when we run the BSRDOtoRAW step
mon_mv = PyStep.MvStep("expert-monitoring.root", "expert-monitoring.ref.root")

chaindump_rdotrig = CheckSteps.ChainDumpStep(name="ChainDump_RDOTrig", input_file="expert-monitoring.ref.root", suffix=".ref")

# RDO -> BS step
rdo2bs = ExecStep.ExecStep('RDOtoBS')
rdo2bs.type = 'Reco_tf'
rdo2bs.input = 'ttbar'
rdo2bs.max_events = nevents
rdo2bs.args += ' --outputBSFile=created.BS'

# BSRDO -> RAW step
ex = ExecStep.ExecStep('BSRDOtoRAW')
ex.type = 'athena'
ex.job_options = 'TriggerJobOpts/runHLT.py'
ex.threads = 1
ex.input = ''
ex.args += ' --filesInput created.BS'
ex.args += ' --preExec "from AthenaConfiguration.DetectorConfigFlags import disableDetectors;disableDetectors(flags,[\'TRT\',\'MBTS\']);"'
ex.args += ' --preInclude "Campaigns.MC23e"'
ex.flags = [
   'Trigger.triggerMenuSetup="Dev_pp_run3_v1_TriggerValidation_prescale"', 'Trigger.doLVL1=True',
   'Trigger.AODEDMSet=\'AODFULL\'',
   'InDet.doTruth=False',
   'Tracking.doTruth=False',
   'Egamma.doTruthAssociation=False',
   'Reco.PostProcessing.GeantTruthThinning=False',
   'Trigger.enableL1CaloLegacy=False',
   'Trigger.enabledSignatures=\'[\"Jet\"]\'',
   f'IOVDb.GlobalTag=\'{defaultConditionsTags.RUN3_MC}\'',
   f'GeoModel.AtlasVersion=\'{defaultGeometryTags.RUN3}\'',
]

diff = CheckSteps.DiffRootStep('diff-root')
diff.reference = 'RDO_TRIG.ref.pool.root'
diff.input_file = 'RDO_TRIG.pool.root'


test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_rm, rdotrig_ref, mon_mv, rdo2bs, ex]
test.check_steps = CheckSteps.default_check_steps(test) + [chaindump_rdotrig] + [diff]

import sys
sys.exit(test.run())
