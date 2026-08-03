#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

# Core import(s).
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaCommon.Constants import DEBUG

# I/O import(s).
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

# Device import(s).
from AthDeviceComps.AthDeviceCompsConfig import \
    MemoryResourcesToolCfg, CopiesToolCfg

# System import(s).
import sys


def EventDataCopyExampleAlgCfg(flags, **kwargs):
    '''Configure the example algorithm for copying event data to a device.
    '''

    # Create an accumulator to hold the configuration.
    result = ComponentAccumulator()

    # Create the example algorithm.
    alg = CompFactory.AthExDevice.EventDataCopyExampleAlg(**kwargs)
    alg.MemoryResourcesTool = \
        result.popToolsAndMerge(MemoryResourcesToolCfg(flags, **kwargs))
    alg.CopiesTool = result.popToolsAndMerge(CopiesToolCfg(flags, **kwargs))

    # Add the algorithm to the accumulator.
    result.addEventAlgo(alg)

    # Return the result to the caller.
    return result


if __name__ == '__main__':

    # Set up the job's flags.
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 100
    flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
    flags.fillFromArgs()
    flags.lock()

    # Set up the main services.
    acc = MainServicesCfg(flags)

    # Set up the input file reading.
    acc.merge(PoolReadCfg(flags))

    # Set up the example algorithm.
    acc.merge(EventDataCopyExampleAlgCfg(flags, OutputLevel=DEBUG))

    # Run the configuration.
    sys.exit(acc.run().isFailure())
