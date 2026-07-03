#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthCUDAServices.AthCUDAServicesConfig import HostMemoryResourceToolCfg, DeviceMemoryResourceToolCfg, CopyToolCfg, StreamToolCfg

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import JSONDeviceDetectorDescriptionProviderSvcCfg, CUDAClusterizerToolCfg,  DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg

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

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14697/RDO.33629020._000001.pool.root.1"
    ]
    flags.fillFromArgs()


    # ---- Conditions and geometry ----
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag    = defaultConditionsTags.RUN4_MC
    flags.GeoModel.AtlasVersion = "ATLAS-P2-RUN4-03-00-00"
    flags.GeoModel.Align.Dynamic = False

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True

    # ---- Scheduler ----
    flags.Concurrency.NumThreads          = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Concurrency.NumProcs            = 0
    flags.Scheduler.ShowDataDeps          = True
    flags.Scheduler.ShowDataFlow          = True
    flags.Scheduler.CheckDependencies     = True

    flags.lock()
    flags.dump()

    acc = GPUClusterizationCfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"