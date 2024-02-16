# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Simple script to run a
# Calo/Tracking/Egamma job
#
# Usefull for quick testing
# run with
# python runEgammaOnly.py
# or
# pythong -m egammaConfig.runEgammaOnly

import sys


def _run(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    flags.Exec.MaxEvents = args.maxEvents

    from AthenaConfiguration.TestDefaults import defaultTestFiles

    if not args.inputFileList:
        flags.Input.Files = defaultTestFiles.RDO_RUN2
    else:
        flags.Input.Files = args.inputFileList

    from AthenaConfiguration.Enums import ProductionStep

    flags.Common.ProductionStep = ProductionStep.Reconstruction

    # output
    flags.Output.AODFileName = args.outputAODFile

    # uncomment given something like export ATHENA_CORE_NUMBER=2
    # flags.Concurrency.NumThreads = 2

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(
        flags, None, use_metadata=True, toggle_geometry=True, keep_beampipe=True
    )

    # egamma Only
    from egammaConfig.egammaOnlyFromRawFlags import egammaOnlyFromRaw

    egammaOnlyFromRaw(flags)

    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering

    acc = RecoSteering(flags)

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg

    acc.merge(DigitizationMessageSvcCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels

    setupLoggingLevels(flags, acc)

    # Print reco domain status
    from RecJobTransforms.RecoConfigFlags import printRecoFlags

    printRecoFlags(flags)

    # running
    statusCode = acc.run()
    return statusCode


if __name__ == "__main__":
    statusCode = None

    # Argument parsing
    from argparse import ArgumentParser

    parser = ArgumentParser("egammaOnly")
    parser.add_argument(
        "-m",
        "--maxEvents",
        default=20,
        type=int,
        help="The number of events to run. -1 runs all events.",
    )
    parser.add_argument(
        "-i", "--inputFileList", nargs="*", help="list of input ESD files"
    )
    parser.add_argument(
        "-o", "--outputAODFile", default="myAOD.pool.root", help="Output file name"
    )
    args = parser.parse_args()

    statusCode = _run(args)

    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())
