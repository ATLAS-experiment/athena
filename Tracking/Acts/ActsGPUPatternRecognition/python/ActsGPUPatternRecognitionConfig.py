# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg
from AthDeviceComps.DeviceConfigFlags import DeviceBackend

# ============================================================
# CUDA Tool configurations
# ============================================================

def CUDASeedingToolCfg(flags,
                                name="CUDASPFormationTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDASeedingAlgProviderTool(name, **kwargs))
    return acc

def CUDATrkParamToolCfg(flags,
                                name="CUDATrkParamTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDATrkParamAlgProviderTool(name, **kwargs))
    return acc

def CUDATrackFindingToolCfg(flags,
                                name="CUDATrackFindingTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDATrackFindingAlgProviderTool(name, **kwargs))
    return acc

# ============================================================
# Tool configurations
# ============================================================

def DeviceSeedingProviderToolCfg(flags,
                                   name="DeviceSeedingProviderTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported device backend: {flags.Acts.DeviceBackend}")   
        
    else:    
        acc.setPrivateTools(acc.popToolsAndMerge(CUDASeedingToolCfg(flags)))
        return  acc  

def DeviceTrkParamProviderToolCfg(flags,
                                   name="DeviceTrkParamAlgProviderTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported device backend: {flags.Acts.DeviceBackend}")   
        
    else:    
        acc.setPrivateTools(acc.popToolsAndMerge(CUDATrkParamToolCfg(flags)))
        return  acc 

def DeviceTrackFindingToolCfg(flags,
                                   name="DeviceTrackFindingAlgProviderTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported device backend: {flags.Acts.DeviceBackend}")   
        
    else:    
        acc.setPrivateTools(acc.popToolsAndMerge(CUDATrackFindingToolCfg(flags)))
        return  acc 

# ============================================================
# Algorithm configurations
# ============================================================

def DeviceGBTSSeedingAlgCfg(flags,
                               name="DeviceGBTSSeedingAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("InputTracccPixelSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("InputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("OutputTracccPixelSeeds", "TracccPixelSeeds")

    from ActsConfig.ActsSeedingConfig import ActsGbtsLayerToolCfg
    kwargs.setdefault("layerNumberTool", acc.popToolsAndMerge(ActsGbtsLayerToolCfg(flags)))

    kwargs.setdefault("SeedingAlgProviderTool", acc.popToolsAndMerge(DeviceSeedingProviderToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceGBTSSeedingAlg(name, **kwargs))
    return acc


def DeviceTripletSeedingAlgCfg(flags,
                               name="DeviceTripletSeedingAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("InputTracccPixelSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("OutputTracccPixelSeeds", "TracccPixelSeeds")
    
    kwargs.setdefault("SeedingAlgProviderTool", acc.popToolsAndMerge(DeviceSeedingProviderToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceTripletSeedingAlg(name, **kwargs))
    return acc

def DeviceTrkParamEstimationAlgCfg(flags,
                               name="DeviceTrkParamEstimationAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("InputTracccSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("InputTracccSeeds", "TracccPixelSeeds")
    kwargs.setdefault("InputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("InputTracccMagField","TracccDeviceMagField")
    kwargs.setdefault("OutputTracccTrackParameters", "TracccTrkParam")
    
    kwargs.setdefault("TrkParamAlgProviderTool", acc.popToolsAndMerge(DeviceTrkParamProviderToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceTrkParamEstimationAlg(name, **kwargs))
    return acc

def DeviceTrackFindingAlgCfg(flags,
                               name="DeviceTrackFindingAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("InputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("InputTracccTrackParameters", "TracccTrkParam")
    kwargs.setdefault("InputTracccMagField","TracccDeviceMagField")
    kwargs.setdefault("InputTracccDetectorGeometry","TracccDeviceGeometry")
    kwargs.setdefault("OutputTracccTracks", "TracccTracks")
    
    kwargs.setdefault("TrackFindingAlgProviderTool", acc.popToolsAndMerge(DeviceTrackFindingToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceTrackFindingAlg(name, **kwargs))
    return acc   