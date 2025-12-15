#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""

Run encoding and monitoring of the RDO to pixel bytestream conversion

"""
from argparse import ArgumentParser

# Argument parsing
parser = ArgumentParser("RunITkPixelEncodingMonitoring.py")
parser.add_argument("-V", "--verboseAccumulators", default=False,
                    action="store_true",
                    help="Print full details of the AlgSequence")
parser.add_argument("-S", "--verboseStoreGate", default=False,
                    action="store_true",
                    help="Dump the StoreGate(s) each event iteration")
parser.add_argument("--maxEvents",default=10, type=int,
                    help="The number of events to run. 0 skips execution")
parser.add_argument("--inputFile",
                    default="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1",
                    help="The input RDO file to use")
args = parser.parse_args()

# Some info about the job
print("----Run encoding and monitoring of the RDO to pixel bytestream conversion----")
print()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

# make logging more verbose
from AthenaCommon.Logging import log
from AthenaCommon.Constants import DEBUG, INFO
log.setLevel(INFO)

flags.Input.isMC  = True
import glob
flags.Input.Files = glob.glob(args.inputFile)

flags.Detector.GeometryCalo  = False
flags.Detector.GeometryMuon  = False

# This should run serially for the moment.
flags.Concurrency.NumThreads = 1
flags.Concurrency.NumConcurrentEvents = 1

log.debug('Lock config flags now.')
flags.lock()

make logging more verbose
from AthenaCommon.Logging import log
from AthenaCommon.Constants import INFO #DEBUG
log.setLevel(INFO)

### setup dumping of additional information
if args.verboseAccumulators:
  cfg.printConfig(withDetails=True)
if args.verboseStoreGate:
  cfg.getService("StoreGateSvc").Dump = True

log.debug('Dumping of ConfigFlags now.')
flags.dump()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge(PoolReadCfg(flags))

from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
cfg.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))

# Adds the encoding alg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
cfg.merge( ITkPixelEncodingAlgCfg(flags, doMonitoring = True, doExpertPlots  = True) )

# loop over the number of events specified as arguments (default to 10)
cfg.run(args.maxEvents)
