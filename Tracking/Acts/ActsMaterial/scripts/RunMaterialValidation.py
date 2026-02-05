#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Run material validation
"""

from argparse import ArgumentParser
from AthenaCommon.Logging import log
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

# Argument parsing
parser = ArgumentParser("RunMaterialValidation.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--localgeo", default=False, action="store_true",
                    help="Use local geometry Xml files")
parser.add_argument("--geoModelSqLiteFile", default = "", help="Read geometry from sqlite file")
parser.add_argument("-V", "--verboseAccumulators", default=False,
                    action="store_true",
                    help="Print full details of the AlgSequence")
parser.add_argument("-S", "--verboseStoreGate", default=False,
                    action="store_true",
                    help="Dump the StoreGate(s) each event iteration")
parser.add_argument("--maxEvents",default=10, type=int,
                    help="The number of events to run. 0 skips execution")
parser.add_argument("--skipEvents",default=0, type=int,
                    help="The number of events to skip")
parser.add_argument("--geometrytag",default="ATLAS-P2-RUN4-03-00-00", type=str,
                    help="The geometry tag to use")
# parser.add_argument("--inputMaterial",
#                     required=True, type=str,
#                     help="Input material maps produced in the material mapping process.")
args = parser.parse_args()


# Some info about the job
print("----RunMaterialValidation for ITk geometry----")
print()
print("Using Geometry Tag: "+args.geometrytag)
if args.localgeo:
    print("...overridden by local Geometry Xml files")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()

# Configure
flags = initConfigFlags()
if args.localgeo:
    flags.ITk.Geometry.AllLocal = True

flags.Input.Files = []
flags.Input.isMC=True
flags.GeoModel.AtlasVersion = args.geometrytag
flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
flags.GeoModel.Align.Dynamic = False

# This should run serially
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors, setupDetectorFlags
from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors

if args.geoModelSqLiteFile:
     flags.GeoModel.SQLiteDB = True
     flags.GeoModel.SQLiteDBFullPath = args.geoModelSqLiteFile
     # hack to set Run4 for running on muon dead material geometry
     from AthenaConfiguration.Enums import LHCPeriod
     flags.GeoModel.Run = LHCPeriod.Run4
else:
    defaultDetectors = ['ITkPixel', 'ITkStrip']
    detectors = args.detectors if 'detectors' in args and args.detectors else defaultDetectors
    detectors.append('Bpipe')  # always run with beam pipe
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

flags.Acts.TrackingGeometry.UseBlueprint = True

flags.Exec.SkipEvents = args.skipEvents

log.debug('Lock config flags now.')
flags.lock()

# Construct our accumulator to run
acc = MainServicesCfg(flags)

### setup dumping of additional information
if args.verboseAccumulators:
  acc.printConfig(withDetails=True)
if args.verboseStoreGate:
  acc.getService("StoreGateSvc").Dump = True

log.debug('Dumping of ConfigFlags now.')
flags.dump()

from ActsConfig.ActsMaterialConfig import MaterialValidationCfg
acc.merge(MaterialValidationCfg(flags))

from ActsConfig.ActsMaterialConfig import MaterialTrackWriterCfg
acc.merge(MaterialTrackWriterCfg(flags, FileName="material-tracks-validation.root",
                                 MaterialTrackCollectionKey="OutputMaterialTracks"))

acc.printConfig(withDetails = True, summariseProps = True)

acc.run(maxEvents=args.maxEvents)



