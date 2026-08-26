#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for GPU mag field creation:
# CVF magnetic field -> covfie magnetic field (required)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg

def GPUMagFieldCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
        OutputLevel = DEBUG))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 1
    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    acc.merge(GPUMagFieldCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
