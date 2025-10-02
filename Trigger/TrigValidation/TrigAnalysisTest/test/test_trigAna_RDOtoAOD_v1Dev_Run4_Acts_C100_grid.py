#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test of transform RDO->RDO_TRIG->AOD with threads=1 and Acts tracking (C100)
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-input: group.trig-hlt.mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_s4345_r15583
# art-input-nfiles: 1
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

from TrigValTools.TrigValSteering import Test, CheckSteps
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

EFTrackPipeline = "C100"
from TrigAnalysisTest.test_trigAna_RDOtoAOD_v1Dev_Run4_Acts_Common import prepare_acts_rdo2aod
rdo2aod = prepare_acts_rdo2aod(EFTrackPipeline)

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [rdo2aod]
test.check_steps = CheckSteps.default_check_steps(test)
add_analysis_steps(test)

import sys
sys.exit(test.run())
