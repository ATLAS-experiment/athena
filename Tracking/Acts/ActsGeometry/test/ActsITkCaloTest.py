#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
  Run ACTS geometry construction for ITk + HGTD + Calo
"""
from pathlib import Path
import argparse

parser = argparse.ArgumentParser(description="Run ACTS geometry construction for ITk + HGTD + Calo")
parser.add_argument("--gen3", action="store_true", help="Use Gen3 geometry + construction")
args = parser.parse_args()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
# Avoid errors accessing LAr conditions: the TrackingGeometry is built in
# initialize() of TrackingGeometrySvc before conditions algorithms have run.
flags.LAr.doAlign = False
flags.Input.Files = []
flags.Input.isMC = True

flags.Detector.GeometryHGTD = True
flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = True
flags.Detector.GeometryMuon = False

flags.Concurrency.NumThreads = 64
flags.Concurrency.NumConcurrentEvents = 64

flags.Exec.MaxEvents = 10

flags.Acts.TrackingGeometry.UseBlueprint = args.gen3

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
                                  NParticlesPerEvent=int(100),
                                  WritePropStep=True,
                                  EtaRange=[-5, 5],
                                  PtRange=[20, 100]))

acc.printConfig()
sc = acc.run()

if sc.isFailure():
    import sys
    sys.exit(1)
