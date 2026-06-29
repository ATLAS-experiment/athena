#!/usr/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Simple ComponentAccumulator configuration for running
# LinearTransformExampleAlg



from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

import sys

def LinearTransformExampleCfg(flags):
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.AthSYCL.LinearTransformExampleAlg())
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    # Set up the job's flags.
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10
    flags.Input.Files = []
    flags.fillFromArgs()
    flags.lock()

    # Set up the main services.
    acc = MainServicesCfg(flags)

    # Merge example algorithm config
    acc.merge(LinearTransformExampleCfg(flags))

    # Run the configuration.
    sys.exit(acc.run().isFailure())
