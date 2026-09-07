
"""
Configuration for the device (GPU) reconstruction chain used by the Traccc
Triton backend.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def TracccTritonDeviceRecoCfg(flags, **kwargs) -> ComponentAccumulator:
    """Set up the GPU chain that the Triton backend drives per request."""

    acc = ComponentAccumulator()

    # ---- StoreGate key contract (shared with the C++ Runner) ----
    cellsKey = kwargs.pop("TracccCellsLocation", "TracccTritonCells")
    measKey = kwargs.pop("TracccMeasurementsLocation", "TracccTritonMeasurements")
    spKey = kwargs.pop("TracccSpacepointsLocation", "TracccTritonSpacepoints")
    seedsKey = kwargs.pop("TracccSeedsLocation", "TracccTritonSeeds")

    # Device detector description (geometry + digitization + conditions +
    # athena<->detray id map) loaded once into the DetectorStore.
    from ActsGPUGeometry.ActsGPUGeometryConfig import (
        JSONDeviceDetectorDescriptionProviderSvcCfg,
    )
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(
        flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
    ))

    # ---- Clusterization: cells -> measurements ----
    from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import (
        DeviceClusterizationAlgCfg,
    )
    acc.merge(DeviceClusterizationAlgCfg(
        flags,
        InputTracccCells=cellsKey,
        OutputTracccMeasurements=measKey,
        OutputTracccClusters=seedsKey + "_Clusters",  # unused, must be unique
        RetrieveClusterCells=False,
    ))

    # ---- Spacepoint formation: measurements -> spacepoints ----
    from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import (
        DeviceSPFormationAlgCfg,
    )
    acc.merge(DeviceSPFormationAlgCfg(
        flags,
        name="DeviceSPFormationAlg",
        InputTracccMeasurements=measKey,
        OutputTracccPixelSpacepoints=spKey,
    ))

    # ---- Triplet seeding: spacepoints -> seeds ----
    from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
        DeviceTripletSeedingAlgCfg,
    )
    acc.merge(DeviceTripletSeedingAlgCfg(
        flags,
        name="DeviceTripletSeedingAlg",
        InputTracccPixelSpacepoints=spKey,
        OutputTracccPixelSeeds=seedsKey,
    ))

    return acc
