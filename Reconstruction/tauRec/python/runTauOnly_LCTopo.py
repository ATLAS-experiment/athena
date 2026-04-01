#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Simple script to run a
# Tau job
#
# Usefull for quick testing using LCTopo jets for tau seeding
# run with
#
# athena runTauOnly_LCTopo.py 
# or
# python runTauOnly_LCTopo.py

import sys

def _run():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # input
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
    flags.Exec.MaxEvents = 20
    flags.Input.Files = defaultTestFiles.RDO_RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Reconstruction

    # output
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Output.AODFileName = "myAOD.pool.root"

    # uncomment given something like export ATHENA_CORE_NUMBER=2
    # flags.Concurrency.NumThreads = 2

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=True,
                       toggle_geometry=True, keep_beampipe=True)

    # Schedule Tau Reco
    from tauRec.ConfigurationHelpers import StandaloneTauRecoFlags
    StandaloneTauRecoFlags(flags)
    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    # keep only tau containers
    from tauRec.ConfigurationHelpers import tauSpecialContent
    tauSpecialContent(flags,acc)

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
    statusCode = _run()
    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())


