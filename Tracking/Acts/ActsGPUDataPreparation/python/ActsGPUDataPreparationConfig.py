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

def CUDASPFormationToolCfg(flags,
                                name="CUDASPFormationTool",
                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.ActsTrk.CUDASPFormationAlgProviderTool(name, **kwargs))
    return acc


def HIPClusterizerToolCfg(flags,
                          name="HIPClusterizerTool",
                          **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthHIPComps.AthHIPCompsConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(
        MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool",
                      acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))
    kwargs.setdefault("CellSorting", flags.Acts.Device.doCellSorting)

    acc.setPrivateTools(
        CompFactory.ActsTrk.HIPClusterizationAlgProviderTool(name, **kwargs))
    return acc


def HIPSPFormationToolCfg(flags,
                          name="HIPSPFormationTool",
                          **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthHIPComps.AthHIPCompsConfig import StreamToolCfg

    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(
        MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool",
                      acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.ActsTrk.HIPSPFormationAlgProviderTool(name, **kwargs))
    return acc


def DeviceClusterizationProviderToolCfg(flags,
                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if (flags.Device.Backend == DeviceBackend.CUDA):
        acc.setPrivateTools(acc.popToolsAndMerge(
            CUDAClusterizerToolCfg(flags, **kwargs)))
    elif ((flags.Device.Backend == DeviceBackend.HIPAMD) or
          (flags.Device.Backend == DeviceBackend.HIPNVIDIA)):
        acc.setPrivateTools(acc.popToolsAndMerge(
            HIPClusterizerToolCfg(flags, **kwargs)))
    else:
        raise ValueError(
            f"Unsupported device backend: {flags.Acts.DeviceBackend}")

    return acc


def DeviceSPFormationProviderToolCfg(flags,
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Device.Backend == DeviceBackend.CUDA:
        acc.setPrivateTools(acc.popToolsAndMerge(
            CUDASPFormationToolCfg(flags, **kwargs)))
    elif ((flags.Device.Backend == DeviceBackend.HIPAMD) or
          (flags.Device.Backend == DeviceBackend.HIPNVIDIA)):
        acc.setPrivateTools(acc.popToolsAndMerge(
            HIPSPFormationToolCfg(flags, **kwargs)))
    else:
        raise ValueError(
            f"Unsupported device backend: {flags.Acts.DeviceBackend}")

    return acc


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


def DeviceSPFormationAlgCfg(flags,
                               name="DeviceSPFormationAlg",
                               previousExtension: str = None,
                               **kwargs) -> ComponentAccumulator:

    assert previousExtension is None or isinstance(previousExtension, str)
    acc = ComponentAccumulator()

    kwargs.setdefault("DeviceMR", acc.popToolsAndMerge(DeviceMemoryResourceToolCfg(flags)))
    kwargs.setdefault("InputTracccMeasurements", "TracccMeas")
    kwargs.setdefault("OutputTracccPixelSpacepoints", "TracccPixelSpacepoints")
    kwargs.setdefault("SPFormationAlgProviderTool", acc.popToolsAndMerge(DeviceSPFormationProviderToolCfg(flags)))
    kwargs.setdefault("DeviceDetectorName", "TracccDeviceDetectorGeometry")

    acc.addEventAlgo(
        CompFactory.ActsTrk.DeviceSPFormationAlg(name, **kwargs))
    return acc
