# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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
