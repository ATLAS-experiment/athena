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
args = parser.parse_args()

flags = initConfigFlags()

flags.Input.isMC = True
flags.Input.Files = []

from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags

flags.GeoModel.AtlasVersion = args.geometrytag if args.geometrytag else defaultGeometryTags.RUN4
flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

flags.Detector.GeometryBpipe = True
flags.Detector.GeometryHGTD = True
flags.Detector.GeometryITkPixel = True
flags.Detector.GeometryITkStrip = True
flags.Detector.GeometryCalo = False
flags.Detector.GeometryMuon = False

flags.GeoModel.Align.Dynamic = False
flags.Acts.TrackingGeometry.UseBlueprint = True
flags.Acts.TrackingGeometry.KeepGoingOnMaterialMergeFailure = True
flags.Acts.TrackingGeometry.MaterialSource = "None"

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

cfg.printConfig(withDetails=True, summariseProps=True)

cfg.run(1)
