
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
    trkParamsKey = kwargs.pop("TracccTrackParametersLocation",
                              "TracccTritonTrackParameters")
    tracksKey = kwargs.pop("TracccTracksLocation", "TracccTritonTracks")

    # ---- DetectorStore object names (device geometry + magnetic field) ----
    deviceGeometry = "TracccDeviceDetectorGeometry"
    deviceMagField = "TracccDeviceMagField"

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
        DeviceDetectorName=deviceGeometry,
        GeoIdMappingObjectName="TracccGeometryIdMapping",
    ))

    # Device magnetic field, needed by track parameter estimation and by the
    # track finder's propagator.
    from ActsGPUMagField.ActsGPUMagFieldConfig import (
        JSONDeviceMagFieldProviderSvcCfg,
    )
    acc.merge(JSONDeviceMagFieldProviderSvcCfg(
        flags,
        DeviceMagFieldObjectName=deviceMagField,
        HostMagFieldObjectName="TracccHostMagField",
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
        DeviceTrkParamEstimationAlgCfg,
        DeviceTrackFindingAlgCfg,
    )
    acc.merge(DeviceTripletSeedingAlgCfg(
        flags,
        name="DeviceTripletSeedingAlg",
        InputTracccPixelSpacepoints=spKey,
        OutputTracccPixelSeeds=seedsKey,
    ))

    # ---- Track parameter estimation: seeds -> initial track parameters ----
    acc.merge(DeviceTrkParamEstimationAlgCfg(
        flags,
        name="DeviceTrkParamEstimationAlg",
        InputTracccSpacepoints=spKey,
        InputTracccMeasurements=measKey,
        InputTracccSeeds=seedsKey,
        InputTracccMagField=deviceMagField,
        OutputTracccTrackParameters=trkParamsKey,
    ))

    # ---- Track finding (fitting through finding): parameters -> tracks ----
    acc.merge(DeviceTrackFindingAlgCfg(
        flags,
        name="DeviceTrackFindingAlg",
        InputTracccMeasurements=measKey,
        InputTracccTrackParameters=trkParamsKey,
        InputTracccMagField=deviceMagField,
        InputTracccDetectorGeometry=deviceGeometry,
        OutputTracccTracks=tracksKey,
    ))

    return acc
