#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for GPU geometry creation:
# JSON geometry file -> detray geometry (required)
# JSON digitization file -> traccc digitization config (required)
# JSON conditions file -> traccc conditions config (required)
# CSV map file -> athena<->detray ID map (required)
# JSON material file -> detray material (optional)
# JSON surface grid file -> detray surface grid (optional)
# CVF magnetic field -> covfie magnetic field (required)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthCUDAServices.AthCUDAServicesConfig import HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg, CopyToolCfg, StreamToolCfg

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import CUDAClusterizerToolCfg,  DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg

def GPUGeometryCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4

    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    acc.merge(GPUGeometryCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
