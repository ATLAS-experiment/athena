#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration 

from pathlib import Path
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from MuonConfig.MuonConfigUtils import configureDefaultTags
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4
flags.GeoModel.SQLiteDB = True
flags.Input.Files = defaultTestFiles.EVNT

flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1
flags.Exec.MaxEvents = 1

flags.Acts.TrackingGeometry.UseBlueprint = True
configureDefaultTags(flags)

flags.lock()
flags.dump()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
from AthenaCommon.Constants import INFO
from AthenaConfiguration.ComponentFactory import CompFactory

acc.merge(ActsTrackingGeometrySvcCfg(flags,
                                     OutputLevel=INFO,
                                     RunConsistencyChecks=True,
                                     BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                     ObjDebugOutput=False))

acc.addEventAlgo(CompFactory.ActsTrk.GeometryEnvelopeTest(ITkExitVolume="",
                                                          CaloExitVolume="ITkCalo",
                                                          MsExitVolume=""))


from MuonConfig.MuonConfigUtils import executeTest
executeTest(acc)


