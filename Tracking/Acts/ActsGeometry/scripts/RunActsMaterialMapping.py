# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python
"""

Run material mapping for ACTS tracking geometry

"""
from AthenaCommon.Logging import log
from argparse import ArgumentParser
from AthenaConfiguration.AllConfigFlags import initConfigFlags

# Argument parsing
parser = ArgumentParser("RunActsMaterialMapping.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("-M", "--material", required=True, type=str,
                    help="The geometry file material source. It is expected to be a path to a valid json file. You can produce one running RunActsWriteTrackingGeometry.py for the specific geometry tag")
parser.add_argument("--localgeo", default=False, action="store_true",
                    help="Use local geometry Xml files")
parser.add_argument("-V", "--verboseAccumulators", default=False,
                    action="store_true",
                    help="Print full details of the AlgSequence")
parser.add_argument("-S", "--verboseStoreGate", default=False,
                    action="store_true",
                    help="Dump the StoreGate(s) each event iteration")
parser.add_argument("--maxEvents",default=10, type=int,
                    help="The number of events to run. 0 skips execution")
parser.add_argument("--geometrytag",default="ATLAS-P2-RUN4-04-00-00", type=str,
                    help="The geometry tag to use")
parser.add_argument("--inputfile",
                    default="MaterialStepCollection.root",
                    help="The input material step file to use")
args = parser.parse_args()

# Some info about the job
print("----Material Mapping for ACTS Tracking Geometry----")
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

flags.Input.isMC  = True
flags.Input.Files = []

if args.localgeo:
  flags.ITk.Geometry.AllLocal = True

flags.GeoModel.AtlasVersion = args.geometrytag
from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
detectors = args.detectors if 'detectors' in args and args.detectors else ['ITkPixel', 'ITkStrip', 'HGTD']
detectors.append('Bpipe')  # always run with beam pipe
setupDetectorsFromList(flags, detectors, toggle_geometry=True)

flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
flags.GeoModel.Align.Dynamic = False
if args.material.find(".json") == -1:
  from AthenaConfiguration.ComponentAccumulator import ConfigurationError
  raise ConfigurationError("Invalid material source. It must be a json file!")
flags.Acts.TrackingGeometry.MaterialSource = args.material
flags.Acts.TrackingGeometry.MaterialCalibrationFolder = "."

flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

import glob
FileList = glob.glob(args.inputfile)
flags.Input.Files = FileList

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

from ActsConfig.ActsGeometryConfig import ActsMaterialTrackWriterSvcCfg
cfg.merge(ActsMaterialTrackWriterSvcCfg(flags,
                                        "ActsMaterialTrackWriterSvc",
                                        FilePath="MaterialTracks_mapping.root",
                                        TreeName="material-tracks"))

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge(PoolReadCfg(flags))

from ActsConfig.ActsGeometryConfig import ActsMaterialMappingCfg
cfg.merge(ActsMaterialMappingCfg(flags, "ActsMaterialMappingCfg",
                                 mapSurfaces = True,
                                 mapVolumes = True))

from AthenaConfiguration.FPEAndCoreDumpConfig import FPEAndCoreDumpCfg
cfg.merge(FPEAndCoreDumpCfg(flags))

cfg.printConfig(withDetails = True, summariseProps = True)

events = args.maxEvents
if events<=0:
  events = 100000000000
cfg.run(maxEvents=events)



