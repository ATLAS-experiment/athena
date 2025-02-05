#!/usr/bin/env python
"""Run tests for MC+MC or MC+data overlay

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags

from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg
from OverlayConfiguration.OverlaySteering import OverlayMainCfg
from OverlayConfiguration.OverlayTestHelpers import \
    CommonTestArgumentParser, \
    overlayTestFlags, postprocessAndLockFlags, printAndRun

# Argument parsing
parser = CommonTestArgumentParser("OverlayTest.py")
parser.add_argument("detectors", metavar="detectors", type=str, nargs="*",
                    help="Specify the list of detectors")
parser.add_argument("--profile", default=False, action="store_true",
                    help="Profile using VTune")
parser.add_argument("--dependencies", default=False, action="store_true",
                    help="Dependency check")
parser.add_argument("--dump", default=False, action="store_true",
                    help="Dump job options")
args = parser.parse_args()

# Some info about the job
print()
print("Overlay: {}".format("MC+data" if args.data else "MC+MC"))
print(f"Run: {args.run}")
print(f"Number of threads: {args.threads}")
print(f"Number of processes: {args.processes}")
if not args.detectors:
    print("Running complete detector")
else:
    print("Running with: {}".format(", ".join(args.detectors)))
print()
if args.profile:
    print("Profiling...")
    print()

flags = initConfigFlags()
flags.Scheduler.AutoLoadUnmetDependencies = False
if args.dependencies:
    flags.Input.FailOnUnknownCollections = True
    flags.Scheduler.CheckOutputUsage = True
    print("Checking dependencies...")
    print()

# Configure
overlayTestFlags(flags, args)
postprocessAndLockFlags(flags, args)

# Construct our accumulator to run
acc = OverlayMainCfg(flags)
if args.profile:
    from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    acc.merge(VTuneProfilerServiceCfg(flags))
if args.dump:
    # TODO: will replace with something else in the future
    pass
acc.merge(DigitizationMessageSvcCfg(flags))
if flags.Overlay.DataOverlay:
    from OverlayConfiguration.DataOverlayConditions import PPTestCfg
    acc.merge(PPTestCfg(flags))

# Count algorithm misses
if flags.Concurrency.NumThreads > 0:
    acc.getService("AlgResourcePool").CountAlgorithmInstanceMisses = True

# dump pickle
with open("ConfigOverlay.pkl", "wb") as f:
    acc.store(f)

# Print and run
sys.exit(printAndRun(acc, flags, args))
