#!/usr/bin/env python
"""

Run material validation to check material maps for tracking geometry.

"""

from AthenaCommon.Logging import log
from argparse import ArgumentParser
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

# Argument parsing
parser = ArgumentParser("RunMaterialValidationITk.py")
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
parser.add_argument("--maxEvents",default=10, type=int,
                    help="The number of events to run. 0 skips execution")
parser.add_argument("--geometrytag",default="ATLAS-P2-RUN4-03-00-00", type=str,
                    help="The geometry tag to use")
parser.add_argument("--noLocalMaterial", action="store_true", default=False, 
                    help="Do NOT use local material maps")
args = parser.parse_args()

# Some info about the job
print("----MaterialValidation for ITk geometry----")
print()
print("Using Geometry Tag: "+args.geometrytag)
if args.localgeo:
    print("...overridden by local Geometry Xml files")
print()

flags.Input.isMC             = True
flags.Input.Files = []

if args.localgeo:
  flags.ITk.Geometry.AllLocal = True

flags.GeoModel.AtlasVersion = args.geometrytag
flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
flags.GeoModel.Align.Dynamic = False
flags.TrackingGeometry.MaterialSource = "COOL"

from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
detectors = args.detectors if 'detectors' in args and args.detectors else ['ITkPixel', 'ITkStrip', 'HGTD']
detectors.append('Bpipe')  # always run with beam pipe
setupDetectorFlags(flags, detectors, toggle_geometry=True)

flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

if not args.noLocalMaterial:
    flags.ITk.trackingGeometry.loadLocalDbForMaterialMaps=True
    LocalDataBaseName = flags.ITk.trackingGeometry.localDatabaseName
    flags.IOVDb.DBConnection='sqlite://;schema='+LocalDataBaseName+';dbname=OFLP200'

log.debug('Lock config flags now.')
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

### setup dumping of additional information
if args.verboseAccumulators:
  cfg.printConfig(withDetails=True)
if args.verboseStoreGate:
  cfg.getService("StoreGateSvc").Dump = True
  
log.debug('Dumping of flags now.')
flags.dump()

from TrkDetDescrAlgs.TrkDetDescrAlgsConfig import MaterialValidationCfg
cfg.merge(MaterialValidationCfg(flags))
  
cfg.printConfig(withDetails = True, summariseProps = True)

## as no input file, maxEvents should be a valid number
cfg.run(maxEvents=args.maxEvents)

