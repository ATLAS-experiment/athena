#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
  Run ACTS geometry construction for ITk, Muon and Calo
"""
from pathlib import Path

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from MuonConfig.MuonConfigUtils import configureDefaultTags
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4
flags.GeoModel.SQLiteDB = True
# Avoid errors accessing LAr conditions: the TrackingGeometry is built in
# initialize() of TrackingGeometrySvc before conditions algorithms have run.
flags.LAr.doAlign = False
flags.Input.Files = []
flags.Input.isMC = True

# when using SQLiteDB, temporarily switch the HGTD off
flags.Detector.GeometryHGTD = False
flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = True
flags.Detector.GeometryMuon = True

flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

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

from MuonConfig.MuonConfigUtils import executeTest
executeTest(acc)
