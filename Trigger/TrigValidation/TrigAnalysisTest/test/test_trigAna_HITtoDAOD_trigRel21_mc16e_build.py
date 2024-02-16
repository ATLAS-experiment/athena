#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test running HITS->RDO in main/23.0, then RDO->RDO_TRIG in 21.0, then RDO_TRIG->AOD in main/23.0, then AOD->DAOD with multiprocess in main
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena
# Skipping art-output which has no effect for build tests.
# If you create a grid version, check art-output in existing grid tests.

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps, Input, Step
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

# HITS -> RDO step in main/23.0
hit2rdo = ExecStep.ExecStep('HITtoRDO')
hit2rdo.type = 'Reco_tf'
hit2rdo.input = 'ttbar_HITS_MC16'
hit2rdo.args =  '--inputRDO_BKGFile=' + Input.get_input('pileup_RDO_MC20e').paths[0]
hit2rdo.args += ' --outputRDOFile=RDO.pool.root'
hit2rdo.args += ' --preInclude "all:Campaigns.MC20e"'
hit2rdo.args += ' --postInclude "default:PyJobTransforms.UseFrontier"'
hit2rdo.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-11"'
hit2rdo.args += ' --steering "doOverlay"'
hit2rdo.args += ' --autoConfiguration="everything"'
hit2rdo.args += ' --CA "all:True"'

# RDO -> RDO_TRIG step in 21.0
rdo2rdotrig = ExecStep.ExecStep('RDOtoRDOTrigger')
rdo2rdotrig.type = 'Reco_tf'
rdo2rdotrig.input = ''
rdo2rdotrig.imf = False
rdo2rdotrig.explicit_input = True
rdo2rdotrig.args = '--inputRDOFile=RDO.pool.root'
rdo2rdotrig.args += ' --outputRDO_TRIGFile=RDO_TRIG.pool.root'
rdo2rdotrig.args += ' --asetup="RDOtoRDOTrigger:Athena,21.0,latest"'
rdo2rdotrig.args += ' --triggerConfig="MCRECO:MC_pp_v7_BulkMCProd_mc_prescale"'
rdo2rdotrig.args += ' --imf="all:True"'
rdo2rdotrig.args += ' --conditionsTag="RDOtoRDOTrigger:OFLCOND-MC16-SDR-RUN2-08-02"'
rdo2rdotrig.args += ' --preInclude "all:Campaigns/MC20e.py"'

# Clear AthFile cache from r21 because it is incompatible with py3 r22 (ATR-21489)
rm_cache = ExecStep.ExecStep('ClearAthFileCache')
rm_cache.type = 'other'
rm_cache.input = ''
rm_cache.executable = 'rm'
rm_cache.args = '-f athfile-cache.ascii.gz'
rm_cache.auto_report_result = False  # Do not set art-result for this step
rm_cache.output_stream = Step.Step.OutputStream.STDOUT_ONLY  # Do not create a log file for this step

# RDO_TRIG -> AOD step in main/23.0
rdotrig2aod = ExecStep.ExecStep('RDOTriggertoAOD')
rdotrig2aod.type = 'Reco_tf'
rdotrig2aod.input = ''
rdotrig2aod.explicit_input = True
rdotrig2aod.args = '--inputRDO_TRIGFile=RDO_TRIG.pool.root'
rdotrig2aod.args += ' --outputAODFile=AOD.pool.root'
rdotrig2aod.args += ' --preExec="all:flags.Trigger.AODEDMSet=\'AODFULL\'"'
rdotrig2aod.args += ' --postExec="all:from OutputStreamAthenaPool.OutputStreamConfig import addToAOD; extraContent=[\'xAOD::TrigCompositeContainer#HLTNav_R2ToR3Summary\',\'xAOD::TrigCompositeAuxContainer#HLTNav_R2ToR3SummaryAux.\']; cfg.merge(addToAOD(flags, extraContent));"'
rdotrig2aod.args += ' --conditionsTag="all:OFLCOND-MC16-SDR-RUN2-11"'
rdotrig2aod.args += ' --preInclude "all:Campaigns.MC20e"'
rdotrig2aod.args += ' --steering "doRDO_TRIG"'
rdotrig2aod.args += ' --CA "all:True"'

# AOD -> DAOD step in main
aod2daod = ExecStep.ExecStep('AODtoDAOD')
aod2daod.type = 'Derivation_tf'
aod2daod.input = ''
aod2daod.forks = 4
aod2daod.explicit_input = True
aod2daod.args = '--inputAODFile=AOD.pool.root'
aod2daod.args += ' --outputDAODFile=DAOD.pool.root'
aod2daod.args += ' --formats=PHYS'
aod2daod.args += ' --CA'
aod2daod.args += ' --sharedWriter=True --athenaMPMergeTargetSize "DAOD_*:0"'
aod2daod.args += ' --asetup="all:Athena,main,latest"'

# Define the test with the above steps
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [hit2rdo, rdo2rdotrig, rm_cache, rdotrig2aod, aod2daod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
