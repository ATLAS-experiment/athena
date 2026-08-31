#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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

nevents = 1

ex_rm = PyStep.PyStep(cleanup)

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
   'Trigger.L1.dogFex=False',
   'Trigger.enableL1CaloLegacy=False',
   'Trigger.enabledSignatures=\'[\"Jet\"]\'',
   f'IOVDb.GlobalTag=\'{defaultConditionsTags.RUN3_MC}\'',
   f'GeoModel.AtlasVersion=\'{defaultGeometryTags.RUN3}\'',
]

class DiffRootStep(CheckSteps.RefComparisonStep):
   def __init__(self,name):
       super(DiffRootStep, self).__init__(name)
       self.executable = 'acmd.py diff-root'
       self.input_file = None
       self.auto_report_result = True
   def configure(self, test):
       if self.reference is None:
           self.log.error('Missing reference for %s', self.name)
       if self.input_file is None:
           self.log.error('Missing input for %s', self.name)
       super(DiffRootStep,self).configure(test)
       self.args += ' {} {}'.format(self.reference, self.input_file)
   def run(self, dry_run=False):
       retcode, cmd = super(DiffRootStep, self).run(dry_run)
       return retcode, cmd
diff = DiffRootStep('diff-root')
diff.reference = 'RDO_TRIG.ref.pool.root'
diff.input_file = 'RDO_TRIG.pool.root'

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ex_rm, rdotrig_ref, rdo2bs, ex]
test.check_steps = CheckSteps.default_check_steps(test) + [diff]

import sys
sys.exit(test.run())
