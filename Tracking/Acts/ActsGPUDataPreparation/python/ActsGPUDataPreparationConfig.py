# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg, DeviceMemoryResourceToolCfg
from AthDeviceComps.DeviceConfigFlags import DeviceBackend

# ============================================================
# Tool configurations
# ============================================================

def CUDAClusterizerToolCfg(flags,
                                name="CUDAClusterizerTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))
    kwargs.setdefault("CellSorting", flags.Acts.Device.doCellSorting)

    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDAClusterizationAlgProviderTool(name, **kwargs))
    return acc

def DeviceClusterizationProviderToolCfg(flags,
                                   name="DeviceClusterizationProviderTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported device backend: {flags.Acts.DeviceBackend}")   
        
    else:    
        acc.setPrivateTools(acc.popToolsAndMerge(CUDAClusterizerToolCfg(flags)))
        return  acc
  


# ============================================================
# Algorithm configurations
# ============================================================

def DeviceClusterizationAlgCfg(flags,
                               name="DeviceClusterizationAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("InputTracccCells", "TracccCells")
    kwargs.setdefault("OutputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("OutputTracccClusters", "TracccClusterCollection")
    kwargs.setdefault("RetrieveClusterCells", False)
    kwargs.setdefault("ClusteringAlgProviderTool", acc.popToolsAndMerge(DeviceClusterizationProviderToolCfg(flags)))
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")

    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceClusterizationAlg(name, **kwargs))
    return acc
