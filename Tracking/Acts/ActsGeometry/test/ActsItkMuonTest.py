# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
  Run ACTS gen3 geometry construction for Itk + Muon
"""
from pathlib import Path
import argparse

parser = argparse.ArgumentParser(description="Run ACTS geometry construction for ITk")
parser.add_argument("--gen3", action="store_true", default="True", help="Use Gen3 geometry + construction")

args = parser.parse_args()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags

flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
flags.GeoModel.Align.Dynamic = False
flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1']

flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryBpipe = True
flags.Detector.GeometryCalo = False
flags.Detector.GeometryMuon = True

flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

flags.Exec.MaxEvents = 200

flags.Acts.TrackingGeometry.UseBlueprint = args.gen3

from MuonGeoModelTestR4.testGeoModel import geoModelFileDefault, configureDefaultTagsCfg
flags.GeoModel.SQLiteDBFullPath = geoModelFileDefault(useR4Layout = True)

flags.GeoModel.SQLiteDB = True
configureDefaultTagsCfg(flags)

flags.lock()
flags.dump()


from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

from ActsConfig.ActsGeometryConfig import ActsExtrapolationAlgCfg, ActsTrackingGeometrySvcCfg
from AthenaCommon.Constants import INFO

tgSvc = ActsTrackingGeometrySvcCfg(flags,
                                   OutputLevel=INFO,
                                   RunConsistencyChecks=True,
                                   BlueprintGraphviz=str(Path.cwd() / "blueprint.dot"),
                                   ObjDebugOutput=False)

acc.merge(tgSvc)

alg = ActsExtrapolationAlgCfg(flags,
                              OutputLevel=INFO,
                              NParticlesPerEvent = int(1000),
                              WritePropStep = True,
                              EtaRange = [-5., 5.],
                              PtRange = [100, 100])


acc.merge(alg)
acc.printConfig()
sc = acc.run()

if sc.isFailure():
    import sys
    sys.exit(1)