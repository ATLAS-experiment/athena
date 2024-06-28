#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

Run material mapping for tracking geometry.
Uses as MaterialStepsCollections as input.

"""
from AthenaCommon.Logging import log
from argparse import ArgumentParser
from AthenaConfiguration.AllConfigFlags import initConfigFlags

# Argument parsing
parser = ArgumentParser("RunMaterialMappingITk.py")
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
parser.add_argument("--inputfile",
                    default="MaterialStepCollection.root",
                    help="The input material step file to use")
args = parser.parse_args()

# Some info about the job
print("----MaterialMapping for ITk geometry----")
print()
print("Using Geometry Tag: "+args.geometrytag)
if args.localgeo:
    print("...overridden by local Geometry Xml files")
print("Input File:"+args.inputfile)
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()

flags = initConfigFlags()
flags.IOVDb.DBConnection = f'sqlite://;schema={flags.ITk.trackingGeometry.localDatabaseName};dbname=OFLP200'

# necessity to create a new PoolFileCatalog
import os
if os.path.exists('./PoolFileCatalog.xml') :
  print('[!] PoolFileCatalog exists in the run directory (may use old PFN!)')
  print('[>] Deleting it now !')
  os.remove('./PoolFileCatalog.xml')

flags.Input.isMC             = True

import glob
flags.Input.Files = glob.glob(args.inputfile)

if args.localgeo:
  flags.ITk.Geometry.AllLocal = True
  
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
detectors = args.detectors if 'detectors' in args and args.detectors else ['ITkPixel', 'ITkStrip', 'HGTD']
detectors.append('Bpipe')  # always run with beam pipe
setupDetectorFlags(flags, detectors, toggle_geometry=True)

flags.GeoModel.AtlasVersion = args.geometrytag
flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
flags.GeoModel.Align.Dynamic = False
flags.TrackingGeometry.MaterialSource = "None"

flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

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

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge(PoolReadCfg(flags))

from TrkDetDescrAlgs.TrkDetDescrAlgsConfig import ITkMaterialMappingCfg
cfg.merge(ITkMaterialMappingCfg(flags, 
                                name="ITkMaterialMapping"))
  
cfg.printConfig(withDetails = True, summariseProps = True)

events = args.maxEvents
if events<=0:
  events = 10000000000
cfg.run(maxEvents=events)

