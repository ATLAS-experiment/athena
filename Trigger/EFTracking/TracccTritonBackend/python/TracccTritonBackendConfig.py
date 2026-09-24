# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Configuration for the device (GPU) reconstruction chain used by the Traccc
Triton backend.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Name of the AthSequencer that holds the per-request device chain
DEFAULT_SEQUENCE_NAME = "TracccDeviceRecoSeq"

# The store keys the C++ side has to name for itself
STOREGATE_KEYS = {
    "cells": "TracccTritonCells",
    "measurements": "TracccTritonMeasurements",
    "tracks": "TracccTritonTracks",
    "geoIdMapping": "TracccGeometryIdMapping",
}


def TracccTritonDeviceRecoCfg(flags,
                              sequenceName: str = DEFAULT_SEQUENCE_NAME
                              ) -> ComponentAccumulator:
    """Set up the GPU chain that the Triton backend drives per request.

    @param sequenceName name of the AthSequencer the device algorithms are
           scheduled in; the Runner looks this up and executes it.
    """

    acc = ComponentAccumulator()

    # The three the Runner also names; see STOREGATE_KEYS above.
    cellsKey = STOREGATE_KEYS["cells"]
    measKey = STOREGATE_KEYS["measurements"]
    tracksKey = STOREGATE_KEYS["tracks"]

    # Intermediate products, passed algorithm to algorithm inside the
    # sequence. Nothing outside this function ever names them.
    spKey = "TracccTritonSpacepoints"
    seedsKey = "TracccTritonSeeds"
    trkParamsKey = "TracccTritonTrackParameters"

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
        GeoIdMappingObjectName=STOREGATE_KEYS["geoIdMapping"],
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

    # Everything merged into this sequence below is what the Runner executes
    acc.addSequence(CompFactory.AthSequencer(
        sequenceName,
        Sequential=True,
        IgnoreFilterPassed=True,
        StopOverride=True,
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
    ), sequenceName=sequenceName)

    # ---- Spacepoint formation: measurements -> spacepoints ----
    from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import (
        DeviceSPFormationAlgCfg,
    )
    acc.merge(DeviceSPFormationAlgCfg(
        flags,
        name="DeviceSPFormationAlg",
        InputTracccMeasurements=measKey,
        OutputTracccPixelSpacepoints=spKey,
    ), sequenceName=sequenceName)

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
    ), sequenceName=sequenceName)

    # ---- Track parameter estimation: seeds -> initial track parameters ----
    acc.merge(DeviceTrkParamEstimationAlgCfg(
        flags,
        name="DeviceTrkParamEstimationAlg",
        InputTracccSpacepoints=spKey,
        InputTracccMeasurements=measKey,
        InputTracccSeeds=seedsKey,
        InputTracccMagField=deviceMagField,
        OutputTracccTrackParameters=trkParamsKey,
    ), sequenceName=sequenceName)

    # ---- Track finding (fitting through finding): parameters -> tracks ----
    acc.merge(DeviceTrackFindingAlgCfg(
        flags,
        name="DeviceTrackFindingAlg",
        InputTracccMeasurements=measKey,
        InputTracccTrackParameters=trkParamsKey,
        InputTracccMagField=deviceMagField,
        InputTracccDetectorGeometry=deviceGeometry,
        OutputTracccTracks=tracksKey,
    ), sequenceName=sequenceName)

    return acc
