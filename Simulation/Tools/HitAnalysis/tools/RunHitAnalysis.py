#!/usr/bin/env python
"""Run HitAnalysis

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys
from argparse import ArgumentParser

# Argument parsing
parser = ArgumentParser("HitAnalysis.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--localgeo", default=False, action="store_true",
                    help="Use local geometry XML files")
parser.add_argument("-n", "--maxEvents", default=-1, type=int,
                    help="The number of events to run. -1 runs all events.")
parser.add_argument("-i", "--input",
                    help="The input HITS file to use")
args = parser.parse_args()


# Some info about the job
print("----HitAnalysis----")
print()
if args.localgeo:
    print("Using local Geometry XML files")
print(f"Input file: {args.input}")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()

# Configure
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import ProductionStep
flags = initConfigFlags()
flags.Common.ProductionStep = ProductionStep.Simulation
flags.Input.Files = [args.input]
if args.localgeo:
    flags.ITk.Geometry.AllLocal = True
flags.Output.HISTFileName = 'SiHitAnalysis.root'
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
setupDetectorFlags(flags, args.detectors, use_metadata=True, toggle_geometry=True)
flags.lock()

# Construct our accumulator to run
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))
from HitAnalysis.HitAnalysisConfig import SiHitAnalysisCfg
acc.merge(SiHitAnalysisCfg(flags))

# Execute and finish
sc = acc.run(maxEvents=args.maxEvents)

# Success should be 0
sys.exit(not sc.isSuccess())
