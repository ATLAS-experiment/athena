# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python
"""

Dump ACTS tracking geometry.

"""
from AthenaCommon.Configurable import Configurable
from AthenaCommon.Logging import log
from argparse import ArgumentParser
from AthenaConfiguration.AllConfigFlags import initConfigFlags

Configurable.configurableRun3Behavior = True

# Argument parsing
parser = ArgumentParser("RunActsWriteTrackingGeometry.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--localgeo", default=False, action="store_true",
                    help="Use local geometry Xml files")
parser.add_argument("-V", "--verboseAccumulators", default=False,
                    action="store_true",
                    help="Print full details of the AlgSequence")
parser.add_argument("-S", "--verboseStoreGate", default=False,
                    action="store_true",
                    help="Dump the StoreGate(s) each event iteration")
parser.add_argument("--geometrytag",default="ATLAS-P2-RUN4-03-00-00", type=str,
                    help="The geometry tag to use")
args = parser.parse_args()

# Some info about the job
print("----Dumping ACTS Tracking Geometry----")
print()
print("Using Geometry Tag: "+args.geometrytag)
if args.localgeo:
    print("...overridden by local Geometry Xml files")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()

flags = initConfigFlags()

flags.Input.isMC             = True
flags.Input.Files = []

if args.localgeo:
  flags.ITk.Geometry.AllLocal = True

from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
detectors = args.detectors if 'detectors' in args and args.detectors else ['ITkPixel', 'ITkStrip', 'HGTD']
detectors.append('Bpipe')  # always run with beam pipe
setupDetectorsFromList(flags, detectors, toggle_geometry=True)

flags.GeoModel.AtlasVersion = args.geometrytag
flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
flags.GeoModel.Align.Dynamic = False
flags.Acts.TrackingGeometry.MaterialSource = "None"

flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

flags.dump()

log.debug('Lock config flags now.')
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

### setup dumping of additional information
if args.verboseAccumulators:
  cfg.printConfig(withDetails=True)
if args.verboseStoreGate:
  cfg.getService("StoreGateSvc").Dump = True

log.debug('Dumping of ConfigFlags now.')
flags.dump()

from ActsConfig.ActsGeometryConfig import ActsAlignmentCondAlgCfg
cfg.merge(ActsAlignmentCondAlgCfg(flags,
                                  name="ActsAlignmentCondAlg"))

from ActsConfig.ActsGeometryConfig import ActsWriteTrackingGeometryCfg
cfg.merge(ActsWriteTrackingGeometryCfg(flags,
                                       name="ActsWriteTrackingGeometry"))

from AthenaConfiguration.FPEAndCoreDumpConfig import FPEAndCoreDumpCfg
cfg.merge(FPEAndCoreDumpCfg(flags))

cfg.printConfig(withDetails = True, summariseProps = True)

cfg.run(1)


