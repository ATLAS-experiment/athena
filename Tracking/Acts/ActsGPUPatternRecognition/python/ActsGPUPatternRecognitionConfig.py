# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg, DeviceMemoryResourceToolCfg
from AthDeviceComps.DeviceConfigFlags import DeviceBackend

# ============================================================
# Tool configurations
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

# ============================================================
# Algorithm configurations
# ============================================================

def DeviceGBTSSeedingAlgCfg(flags,
                               name="DeviceGBTSSeedingAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)                           
    acc = ComponentAccumulator()

    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("InputTracccPixelSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("InputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("OutputTracccPixelSeeds", "TracccPixelSeeds")

    from TrigFastTrackFinder.TrigFastTrackFinderConfig import ITkTrigL2LayerNumberToolCfg
    layerNumberArgs = {"UseNewLayerScheme" : True}
    kwargs.setdefault("layerNumberTool", acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags, **layerNumberArgs)))

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

    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
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

    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("InputTracccSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("InputTracccSeeds", "TracccPixelSeeds")
    kwargs.setdefault("InputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("InputTracccMagField","TracccDeviceMagField")
    kwargs.setdefault("OutputTracccTrackParameters", "TracccTrkParam")
    
    kwargs.setdefault("TrkParamAlgProviderTool", acc.popToolsAndMerge(DeviceTrkParamProviderToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceTrkParamEstimationAlg(name, **kwargs))
    return acc
