#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
  Run ACTS geometry construction for ITk and Muon
"""

from pathlib import Path
import argparse

parser = argparse.ArgumentParser(description="Run ACTS geometry construction for ITk and Muon")
parser.add_argument("--gen3", action="store_true", default="True", help="Use Gen3 geometry + construction")

args = parser.parse_args()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.TestDefaults import defaultTestFiles

from MuonGeoModelTestR4.testGeoModel import configureDefaultTagsCfg, MuonPhaseIITestDefaults
flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4

flags.GeoModel.SQLiteDB = True

flags.GeoModel.Align.Dynamic = False
#This is set to false to avoid errors accessing LAr conditions.
#We only need to access the static version of the CaloDetDescr anyway because the TrackingGeometry is build
#in the initialize() of the TrackingGeometrySvc prior to conditions algorithims having been run.
flags.LAr.doAlign=False
flags.Input.Files = defaultTestFiles.EVNT

flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = True
flags.Detector.GeometryMuon = True

flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

flags.Exec.MaxEvents = 10

flags.Acts.TrackingGeometry.UseBlueprint = args.gen3


configureDefaultTagsCfg(flags)

flags.lock()
flags.dump()


from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

from ActsConfig.ActsGeometryConfig import ActsExtrapolationAlgCfg, ActsTrackingGeometrySvcCfg
from AthenaCommon.Constants import INFO

acc.merge(ActsTrackingGeometrySvcCfg(flags,
                                   OutputLevel=INFO,
                                   RunConsistencyChecks=True,
                                   BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                   printGeometry=True,
                                   ObjDebugOutput=False))

acc.merge(ActsExtrapolationAlgCfg(flags,
                                  OutputLevel=INFO,
                                  NParticlesPerEvent = int(1000),
                                  WritePropStep = True,
                                  EtaRange = [-5., 5.],
                                  PtRange = [100, 100]))

from MuonConfig.MuonConfigUtils import executeTest
executeTest(acc)