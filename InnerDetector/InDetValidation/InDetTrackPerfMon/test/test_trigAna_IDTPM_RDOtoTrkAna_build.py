#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# art-description: Test to trasform RDO->AOD and then run IDTPM for an Offline TrackAnalysis
# art-type: build
# art-include: main/Athena
# art-output: *.root

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

maxEvents = 15

## RDO->AOD
rdo2aod = ExecStep.ExecStep()
rdo2aod.type = 'Reco_tf'
rdo2aod.input = 'ttbar_pu200_Run4' # defined in TrigValTools/share/TrigValInputs.json
rdo2aod.max_events = maxEvents
rdo2aod.args = ' --CA'
rdo2aod.args += ' --outputAODFile AOD.pool.root'
rdo2aod.args += ' --steering doRAWtoALL'
rdo2aod.args += ' --preInclude InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude'

## IDTPM TrkAnalysis
aod2trkana = ExecStep.ExecStep( 'IDTPM' )
aod2trkana.type = 'other'
aod2trkana.input = ''
aod2trkana.max_events = -1
aod2trkana.required = True
aod2trkana.depends_on_previous = True
aod2trkana.executable = 'runIDTPM.py'
aod2trkana.args = ' --inputFileNames AOD.pool.root'
aod2trkana.args += ' --outputFilePrefix myIDTPM'
aod2trkana.args += ' --trkAnaCfgFile InDetTrackPerfMon/offlTrkAnaConfig.json'
aod2trkana.args += ' --writeAOD_IDTPM'
aod2trkana.args += ' --debug'

## Define test
test = Test.Test()
test.art_type = 'build'
test.exec_steps = [ rdo2aod, aod2trkana ]
test.check_steps = CheckSteps.default_check_steps( test )

import sys
sys.exit( test.run() )

