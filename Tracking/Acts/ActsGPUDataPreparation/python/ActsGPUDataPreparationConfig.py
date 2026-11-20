# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

from AthDeviceComps.AthDeviceCompsConfig import MemoryResourcesToolCfg, CopyToolCfg, DeviceMemoryResourceToolCfg
from AthDeviceComps.DeviceConfigFlags import DeviceBackend
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg

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


def CUDAStripSPFormationToolCfg(flags, name="CUDAStripSPFormationTool", **kwargs):
    """Configure only the CUDA strip SP algorithm provider."""
    from AthCUDAServices.AthCUDAServicesConfig import StreamToolCfg

    acc = ComponentAccumulator()
    kwargs.setdefault("MemoryResourcesTool", acc.popToolsAndMerge(MemoryResourcesToolCfg(flags)))
    kwargs.setdefault("CopyProviderTool", acc.popToolsAndMerge(CopyToolCfg(flags)))
    kwargs.setdefault("StreamTool", acc.popToolsAndMerge(StreamToolCfg(flags)))
    acc.setPrivateTools(CompFactory.ActsTrk.CUDAStripSPFormationAlgProviderTool(name, **kwargs))
    return acc


def StripSpacepointInputsCondAlgCfg(flags, name="StripSpacepointInputsCondAlg", **kwargs):
    """Build and upload immutable inputs for the alignment/topology IOV."""
    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported Strip SP backend: {flags.Device.Backend}")
    acc = ComponentAccumulator()
    acc.merge(ITkStripReadoutGeometryCfg(flags))
    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
    if "SPFormationAlgProviderTool" not in kwargs:
        kwargs["SPFormationAlgProviderTool"] = acc.popToolsAndMerge(
            CUDAStripSPFormationToolCfg(flags, name + "Provider"))
    acc.addCondAlgo(CompFactory.ActsTrk.StripSpacepointInputsCondAlg(name, **kwargs))
    return acc


def DeviceStripSPFormationAlgCfg(flags, name="DeviceStripSPFormationAlg", **kwargs):
    """Configure strip SP formation without host conversion or output streams."""
    if flags.Device.Backend is not DeviceBackend.CUDA:
        raise ValueError(f"Unsupported Strip SP backend: {flags.Device.Backend}")
    if flags.Beam.Type is not BeamType.Collisions:
        raise ValueError("GPU Strip SP formation currently supports collision beam spots")
    acc = ComponentAccumulator()
    # Geometry keys belong to the conditions producer, not the event Algorithm.
    inputsKey = kwargs.setdefault("InputStripSPConditions", "TracccStripSpacepointConditions")
    conditions = {key: kwargs.pop(key) for key in
                  ("StripDetectorElements", "StripProperties", "GeoIdMapping") if key in kwargs}
    conditions["OutputStripSPConditions"] = inputsKey
    acc.merge(StripSpacepointInputsCondAlgCfg(flags, **conditions))
    acc.merge(BeamSpotCondAlgCfg(flags))
    if "SPFormationAlgProviderTool" not in kwargs:
        kwargs["SPFormationAlgProviderTool"] = acc.popToolsAndMerge(
            CUDAStripSPFormationToolCfg(flags, name + "Provider"))
    acc.addEventAlgo(CompFactory.ActsTrk.DeviceStripSPFormationAlg(name, **kwargs))
    return acc
