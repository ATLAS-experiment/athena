#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from TrigValTools.TrigValSteering import Test, CheckSteps
from TrigAnalysisTest.TrigAnalysisSteps import add_analysis_steps

EFTrackPipeline = "C230"
from TrigAnalysisTest.test_trigAna_RDOtoAOD_v1Dev_Run4_Acts_Common import prepare_acts_rdo2aod
rdo2aod = prepare_acts_rdo2aod(EFTrackPipeline)
rdo2aod.threads = 1
rdo2aod.max_events = 10

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [rdo2aod]

import sys
sys.exit(test.run())
