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

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))
    
    kwargs.setdefault("SeedingAlgProviderTool", acc.popToolsAndMerge(DeviceSeedingProviderToolCfg(flags)))
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceTripletSeedingAlg(name, **kwargs))
    return acc

def DeviceLargeRadiusStripTripletSeedingAlgCfg(flags,
                                               name="DeviceLargeRadiusStripTripletSeedingAlg",
                                               **kwargs) -> ComponentAccumulator:
    # Cuts following ActsLargeRadiusStripSeedingToolCfg, restricted to what the
    # traccc triplet seeder supports
    kwargs.setdefault("InputTracccPixelSpacepoints", "TracccStripSpacepoints")
    kwargs.setdefault("OutputTracccPixelSeeds", "TracccStripSeeds")
    # Seed finder
    kwargs.setdefault("zMin", -3000.)
    kwargs.setdefault("zMax", 3000.)
    kwargs.setdefault("rMin", 350.)
    kwargs.setdefault("rMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("collisionRegionMin", -flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("collisionRegionMax", flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPTSeed)
    kwargs.setdefault("impactMax", flags.Tracking.ActiveConfig.maxPrimaryImpactSeed)
    kwargs.setdefault("cotThetaMax", 5.0)
    kwargs.setdefault("deltaRMin", 50.)
    kwargs.setdefault("deltaRMax", 250.)
    kwargs.setdefault("gridDeltaRMax", 400.)
    kwargs.setdefault("deltaZMax", 850.)
    kwargs.setdefault("sigmaScattering", 2.)
    kwargs.setdefault("maxPtScattering", 1.e9)
    kwargs.setdefault("radLengthPerSeed", 0.098045)
    kwargs.setdefault("maxSeedsPerSpM", 1)
    kwargs.setdefault("phiBinDeflectionCoverage", 3)
    # Seed filter
    kwargs.setdefault("deltaInvHelixDiameter", 0.00003)
    kwargs.setdefault("impactWeightFactor", 1.)
    kwargs.setdefault("compatSeedWeight", 100.)
    kwargs.setdefault("filterDeltaRMin", 20.)
    kwargs.setdefault("compatSeedLimit", 4)
    # Disable the radius based weights and cuts of the pixel seeding
    kwargs.setdefault("goodSpBMinRadius", 1.e9)
    kwargs.setdefault("goodSpTMaxRadius", -1.)
    kwargs.setdefault("seedMinWeight", -1.e9)
    kwargs.setdefault("spBMinRadius", 0.)
    return DeviceTripletSeedingAlgCfg(flags, name, **kwargs)

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

def DeviceLargeRadiusTrackFindingAlgCfg(flags,
                                        name="DeviceLargeRadiusTrackFindingAlg",
                                        **kwargs) -> ComponentAccumulator:
    # Cuts following ActsTrackFindingToolCfg for the large radius pass,
    # restricted to what the traccc track finding supports
    from AthenaCommon.SystemOfUnits import MeV
    kwargs.setdefault("chi2Max", flags.Tracking.ActiveConfig.Xi2max[0])
    kwargs.setdefault("minTrackCandidatesPerTrack", flags.Tracking.ActiveConfig.minClusters[0])
    kwargs.setdefault("maxNumSkippingPerCand", flags.Tracking.ActiveConfig.maxHoles[0])
    kwargs.setdefault("maxNumConsecutiveSkipped", flags.Tracking.ActiveConfig.maxHoles[0])
    kwargs.setdefault("minPt", flags.Tracking.ActiveConfig.minPT[0] / MeV)
    return DeviceTrackFindingAlgCfg(flags, name, **kwargs)   