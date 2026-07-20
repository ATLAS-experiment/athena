#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthCUDAServices.AthCUDAServicesConfig import HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg, CopyToolCfg, StreamToolCfg

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import CUDAClusterizerToolCfg,  DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg

def GPUClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    hostMR   = acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags, name="HostMR"))
    deviceMR = acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags, name="DeviceMR"))
    copyTool = acc.popToolsAndMerge(CopyToolCfg(flags, name="CopyProviderTool"))
    streamTool = acc.popToolsAndMerge(StreamToolCfg(flags, name="StreamTool"))

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostMR   = hostMR,
        DeviceMR = deviceMR,
        CopyProviderTool = copyTool))

    clusterizerTool = acc.popToolsAndMerge(CUDAClusterizerToolCfg(flags,
        HostMR  = hostMR,
        DeviceMR = deviceMR,
        CopyProviderTool = copyTool,
        StreamTool = streamTool))

    acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
        HostMR  = hostMR,
        DeviceMR = deviceMR,
        CopyProviderTool = copyTool))

    acc.merge(DeviceClusterizationAlgCfg(flags,
        ClusteringAlgProviderTool = clusterizerTool,
        CopyProviderTool = copyTool,
        DeviceMR = deviceMR))

    acc.merge(TracccMeasurementConverterAlgCfg(flags,
        CopyProviderTool = copyTool,
        HostMR  = hostMR))

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

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    acc.merge(GPUClusterizationCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"