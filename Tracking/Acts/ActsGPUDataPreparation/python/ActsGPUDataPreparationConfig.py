# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# ============================================================
# Service configurations
# ============================================================

def JSONDeviceDetectorDescriptionProviderSvcCfg(flags, **kwargs) -> ComponentAccumulator:

    acc = ComponentAccumulator()
    kwargs.setdefault("GeometryFile",     "dev/ACTS/detray-itk/detray_detector_geometry-for-fun.json")
    kwargs.setdefault("DigitizationFile", "dev/ACTS/detray-itk/ITk_digitization_config.json")
    kwargs.setdefault("ConditionsFile",   "dev/ACTS/detray-itk/ITk_conditions_config.json")
    kwargs.setdefault("MapFile",          "dev/ACTS/detray-itk/athenaIdentifierToDetrayMap.txt")
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("HostDigitizationObjectName", "TracccHostDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    kwargs.setdefault("HostConditionsObjectName", "TracccHostCondConfig")
    svc = CompFactory.ActsTrk.JSONDeviceDetectorDescriptionProviderSvc(**kwargs)
    acc.addService(svc, primary=True, create=True)
    return acc

# ============================================================
# Tool configurations
# ============================================================

def CUDAClusterizerToolCfg(flags,
                                name="CUDAClusterizerTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDAClusterizationAlgProviderTool(name, **kwargs))
    return acc


# ============================================================
# Algorithm configurations
# ============================================================

def DeviceClusterizationAlgCfg(flags,
                               name="DeviceClusterizationAlg",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("OutputTracccClusters", "TracccClusters")
    kwargs.setdefault("InputTracccCells",         "TracccCells")
    kwargs.setdefault("OutputTracccMeasurements", "TracccMeasurements")
    kwargs.setdefault("DeviceDigitizationObjectName", "TracccDeviceDigitizationConfig")
    kwargs.setdefault("DeviceConditionsObjectName", "TracccDeviceCondConfig")
    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceClusterizationAlg(name, **kwargs))
    return acc
