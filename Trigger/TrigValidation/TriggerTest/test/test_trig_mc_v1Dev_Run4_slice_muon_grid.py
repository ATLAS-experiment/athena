#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger RDO->RDO_TRIG athena test of the muon slice in Dev_pp_run4_v1 menu
# art-type: grid
# art-include: main/Athena/x86_64-el9-gcc14-opt
# art-architecture: '#x86_64-intel'
# FIXME: no detail about this file but metadata looks suspicious (MC23a campaign and conditions run number, mixed with Run4 geometry and global tag)
# /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/myRDO.R4.pool.root
# for now uploaded it to rucio dataset, replace at next occasion!
# art-input: group.trig-hlt.MuonGeomRTT.myRDO.R4
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
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

import os
os.environ["PATHRESOLVER_DEVAREARESPONSE"] = "WARNING"

from TriggerTest.MCExecStep import MCGridStep
from TrigValTools.TrigValSteering import Test, CheckSteps
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags

ex = MCGridStep(menu='Dev_pp_run4_v1', signatures=['Muon'])

# FIXME: MC inputs should be on EOS, not cvmfs
ex.input = f'{MuonPhaseIITestDefaults.RDO_R4[0]}'

ex.flags+=[ 'GeoModel.SQLiteDB=True',
           f'GeoModel.SQLiteDBFullPath={MuonPhaseIITestDefaults.GEODB_R4}',
           f'GeoModel.AtlasVersion={defaultGeometryTags.RUN4}',
           f'IOVDb.GlobalTag={defaultConditionsTags.RUN4_MC}',
            'Trigger.Offline.SA.Muon.scheduleActsReco=True',
            'ITk.doTruth=False',
            'Tracking.doTruth=False',
            'Trigger.enableL1CaloPhase1=True',
            'Acts.TrackingGeometry.UseBlueprint=True',
            'Common.MsgSuppression=False',
            'Trigger.enableL1CaloLegacy=False']
ex.imf = False

test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
test.check_steps = CheckSteps.default_check_steps(test)

import sys
sys.exit(test.run())
