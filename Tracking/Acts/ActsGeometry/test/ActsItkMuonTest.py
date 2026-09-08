#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
  Run ACTS geometry construction for ITk and Muon
"""
from pathlib import Path

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from MuonConfig.MuonConfigUtils import configureDefaultTags
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4
flags.GeoModel.SQLiteDB = True
flags.Input.Files = []
flags.Input.isMC = True

# when using SQLiteDB, temporarily switch the HGTD off
flags.Detector.GeometryHGTD = False
flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = False
flags.Detector.GeometryMuon = True

flags.Concurrency.NumThreads = 64
flags.Concurrency.NumConcurrentEvents = 64

flags.Exec.MaxEvents = 10

flags.Acts.TrackingGeometry.UseBlueprint = True
configureDefaultTags(flags)

flags.lock()
flags.dump()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from ActsConfig.ActsGeometryConfig import ActsExtrapolationAlgCfg, ActsTrackingGeometrySvcCfg
from AthenaCommon.Constants import INFO

acc.merge(ActsTrackingGeometrySvcCfg(flags,
                                     OutputLevel=INFO,
                                     RunConsistencyChecks=True,
                                     BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                     ObjDebugOutput=False))

acc.merge(ActsExtrapolationAlgCfg(flags,
                                  OutputLevel=INFO,
                                  NParticlesPerEvent=int(1000),
                                  WritePropStep=True,
                                  EtaRange=[-5., 5.],
                                  PtRange=[100, 100]))

acc.printConfig()
sc = acc.run()

if sc.isFailure():
    import sys
    sys.exit(1)
