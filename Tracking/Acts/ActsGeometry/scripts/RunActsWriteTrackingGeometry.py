#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Dump the ACTS tracking geometry to a file.
"""

from AthenaCommon.Logging import log
from argparse import ArgumentParser
from AthenaConfiguration.AllConfigFlags import initConfigFlags

parser = ArgumentParser("RunActsWriteTrackingGeometry.py")
parser.add_argument(
    "-V",
    "--verboseAccumulators",
    default=False,
    action="store_true",
    help="Print full details of the AlgSequence",
)
parser.add_argument(
    "-S",
    "--verboseStoreGate",
    default=False,
    action="store_true",
    help="Dump the StoreGate(s) each event iteration",
)
parser.add_argument(
    "--geometrytag",
    default="",
    type=str,
    help="The geometry tag to use. If not specified, the default RUN4 tag will be used.",
)
parser.add_argument(
    "--dumpMaterialJson",
    default=False,
    action="store_true",
    help="Also dump the material assigned to the geometry to a Json material map",
)
args = parser.parse_args()

flags = initConfigFlags()

flags.Input.isMC = True
flags.Input.Files = []

from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags

flags.GeoModel.AtlasVersion = args.geometrytag if args.geometrytag else defaultGeometryTags.RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

flags.Detector.GeometryBpipe = False
flags.Detector.GeometryHGTD = False
flags.Detector.GeometryITkPixel = False
flags.Detector.GeometryITkStrip = False
flags.Detector.GeometryCalo = False
from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
flags.GeoModel.SQLiteDBFullPath = MuonPhaseIITestDefaults.GEODB_R4
flags.GeoModel.SQLiteDB = False
flags.Detector.GeometryMuon = False
flags.Muon.trackGeometryMaterialMap="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/material-maps.root"

flags.GeoModel.Align.Dynamic = False
flags.Acts.TrackingGeometry.UseBlueprint = True
flags.Acts.TrackingGeometry.KeepGoingOnMaterialMergeFailure = True
flags.Acts.TrackingGeometry.ITkHgtdMaterialSource = "Default"
flags.Acts.TrackingGeometry.ITkHgtdMaterialSource = "material-maps-itk-hgtd-ATLAS-P2-RUN4-05-00-00.json"
flags.Acts.TrackingGeometry.ITkHgtdMaterialMapPath = "."

# Geometry dump runs serially
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

flags.lock()
flags.dump()

log.debug("Lock config flags now.")

from AthenaConfiguration.MainServicesConfig import MainServicesCfg

cfg = MainServicesCfg(flags)

if args.verboseAccumulators:
    cfg.printConfig(withDetails=True)
if args.verboseStoreGate:
    cfg.getService("StoreGateSvc").Dump = True

from ActsConfig.ActsGeometryConfig import ActsWriteTrackingGeometryCfg

cfg.merge(ActsWriteTrackingGeometryCfg(flags, name="ActsWriteTrackingGeometry"))
if args.dumpMaterialJson:
    from ActsConfig.ActsMaterialConfig import MaterialJsonDumpCfg
    cfg.merge(MaterialJsonDumpCfg(flags, FileName="material-maps"))


cfg.printConfig(withDetails=True, summariseProps=True)

cfg.run(1)
