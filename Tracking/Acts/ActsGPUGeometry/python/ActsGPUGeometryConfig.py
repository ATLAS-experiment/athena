# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg

# ============================================================
# Service configurations
# ============================================================

def JSONDeviceDetectorDescriptionProviderSvcCfg(flags, **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()


    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("GeometryFile",     "dev/ACTS/detray-itk/detray_detector_geometry-for-fun.json")
    kwargs.setdefault("DigitizationFile", "dev/ACTS/detray-itk/ITk_digitization_config.json")
    kwargs.setdefault("ConditionsFile",   "dev/ACTS/detray-itk/ITk_conditions_config.json")
    kwargs.setdefault("MapFile",          "dev/ACTS/detray-itk/athenaIdentifierToDetrayMap.txt")
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    kwargs.setdefault("DeviceDetectorName", "TracccDeviceDetectorGeometry")
    kwargs.setdefault("HostDetectorName", "TracccHostDetectorGeometry")
    svc = CompFactory.ActsTrk.JSONDeviceDetectorDescriptionProviderSvc(**kwargs)
    acc.addService(svc, primary=True, create=True)
    return acc

def DeviceDetectorDescriptionCondAlgCfg(flags, name="ActsDeviceDetectorDescriptionCondAlg", **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            "TrackingGeometryTool",
            acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )

    kwargs.setdefault("GeometryFile",     "dev/ACTS/detray-itk/detray_detector_geometry-for-fun.json")
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")

    the_alg = CompFactory.ActsTrk.DeviceDetectorDescriptionCondAlg(name, **kwargs)
    acc.addCondAlgo(the_alg, primary = True)

    return acc 


def DeviceDetectorDescriptionValidationAlgCfg(flags, name="ActsDeviceDetectorDescriptionValidationAlg", **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    
    kwargs.setdefault("HostDesignObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("HostCondKey", "TracccHostCondConfig")
    kwargs.setdefault("RefHostDesignObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("RefHostCondKey", "TracccHostCondConfig")
    
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceDetectorDescriptionValidationAlg(name, **kwargs))
    return acc        
