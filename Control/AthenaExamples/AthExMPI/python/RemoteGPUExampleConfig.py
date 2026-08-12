#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Configuration for the asynchronous MPI remote-GPU example."""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def RemoteGPUExampleCfg(flags, **kwargs):
    """Configure the rank-zero service and worker-rank algorithm."""
    result = ComponentAccumulator()
    result.addService(CompFactory.RemoteCall.RemoteGPUSvc())
    result.addEventAlgo(CompFactory.RemoteCall.RemoteGPUExampleAlg(**kwargs))
    return result


if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.Exec.MPI = False
    flags.Exec.MaxEvents = 100
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Concurrency.NumOffloadThreads = 1
    flags.Input.Files = []
    flags.fillFromArgs()
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(RemoteGPUExampleCfg(flags))

    sys.exit(acc.run().isFailure())
