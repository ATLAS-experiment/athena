#!/usr/bin/env python
"""Overlay test helpers

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from argparse import ArgumentParser
from AthenaCommon.Debugging import DbgStage
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import LHCPeriod


def CommonTestArgumentParser(prog):
    """Common overlay test argument parser"""
    parser = ArgumentParser(prog=prog)
    parser.add_argument("-d", "--data", default=False,
                        action="store_true", help="Run data overlay")
    parser.add_argument("-n", "--maxEvents", default=3, type=int,
                        help="The number of events to run. 0 skips execution")
    parser.add_argument("-t", "--threads", default=1, type=int,
                        help="The number of concurrent threads to run. 0 uses serial Athena.")
    parser.add_argument("-c", "--concurrent", default=0, type=int,
                        help="The number of concurrent events to run. 0 uses the same as number of threads.")
    parser.add_argument("-V", "--verboseAccumulators", default=False, action="store_true",
                        help="Print full details of the AlgSequence for each accumulator")
    parser.add_argument("-S", "--verboseStoreGate", default=False, action="store_true",
                        help="Dump the StoreGate(s) each event iteration")
    parser.add_argument("-o", "--output", default='', type=str,
                        help="Output RDO file")
    parser.add_argument("-s", "--outputSig", default='', type=str,
                        help="Output RDO_SGNL file")
    parser.add_argument("-r", "--run", default=LHCPeriod.Run2,
                        type=LHCPeriod, choices=list(LHCPeriod))
    parser.add_argument("--disableTruth", default=False, action="store_true",
                        help="Disable truth overlay")
    parser.add_argument("--debug", default='', type=str,
                        choices=DbgStage.allowed_values,
                        help="Debugging flag: " + ','.join (DbgStage.allowed_values))
    return parser


def overlayTestFlags(flags, args):
    """Fill default overlay flags for testing"""
    if args.disableTruth:
        flags.Digitization.EnableTruth = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Overlay
    if args.data:
        flags.Overlay.DataOverlay = True
        flags.Overlay.DataOverlayConditions = "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg"
        flags.Input.isMC = False
        flags.Input.Files = defaultTestFiles.RDO_BKG_HI_RUN3_2023
        flags.Input.SecondaryFiles = defaultTestFiles.HITS_DATA_OVERLAY_HI_RUN3_2023
        flags.Output.RDOFileName = "dataOverlayRDO.pool.root"
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA23
        flags.IOVDb.DatabaseInstance = "CONDBR2"
        from Campaigns import DataOverlay2023
        DataOverlay2023(flags)
    else:
        if args.run is LHCPeriod.Run2:
            flags.Input.Files = defaultTestFiles.RDO_BKG_RUN2
            flags.Input.SecondaryFiles = defaultTestFiles.HITS_RUN2
            flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_MC
            from Campaigns import MC20e
            MC20e(flags)
        elif args.run is LHCPeriod.Run3:
            flags.Input.Files = defaultTestFiles.RDO_BKG_RUN3_2022
            flags.Input.SecondaryFiles = defaultTestFiles.HITS_RUN3_2022
            flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
            from Campaigns import MC23a
            MC23a(flags)
        elif args.run is LHCPeriod.Run4:
            flags.Input.Files = defaultTestFiles.RDO_BKG_RUN4
            flags.Input.SecondaryFiles = defaultTestFiles.HITS_RUN4
            flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
            from Campaigns import PhaseIIPileUp200
            PhaseIIPileUp200(flags)
        else:
            raise ValueError("Run not supported")
        flags.Input.MCChannelNumber = GetFileMD(flags.Input.SecondaryFiles, allowEmpty=False).get("mc_channel_number", 0)
        flags.Output.RDOFileName = "mcOverlayRDO.pool.root"
        flags.Overlay.DataOverlay = False

    if args.output:
        if args.output == 'None':
            flags.Output.RDOFileName = ''
        else:
            flags.Output.RDOFileName = args.output

    if args.outputSig:
        flags.Output.RDO_SGNLFileName = args.outputSig

    if 'detectors' in args and args.detectors:
        detectors = args.detectors
    else:
        detectors = None

    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True, use_metadata=True)

    # Moving here so that it is ahead of flags being locked. Need to
    # iterate on exact best position w.r.t. above calls
    from OverlayConfiguration.OverlayMetadata import overlayMetadataCheck
    # Handle metadata correctly
    overlayMetadataCheck(flags)


def postprocessAndLockFlags(flags, args):
    """Postprocess and lock config flags for overlay"""

    # Flags relating to multithreaded execution
    flags.Concurrency.NumThreads = args.threads
    if args.threads > 0:
        flags.Scheduler.ShowDataDeps = True
        flags.Scheduler.ShowDataFlow = True
        flags.Scheduler.ShowControlFlow = True
        flags.Concurrency.NumConcurrentEvents = args.concurrent if args.concurrent > 0 else args.threads

    flags.lock()


def printAndRun(accessor, flags, args):
    """Common debugging and execution for overlay tests"""
    # Dump config
    accessor.printConfig(withDetails=args.verboseAccumulators)
    if args.verboseStoreGate:
        accessor.getService("StoreGateSvc").Dump = True
    flags.dump()

    if args.debug:
        accessor.setDebugStage (args.debug)

    # Execute and finish
    sc = accessor.run(maxEvents=args.maxEvents)

    # Success should be 0
    return not sc.isSuccess()
