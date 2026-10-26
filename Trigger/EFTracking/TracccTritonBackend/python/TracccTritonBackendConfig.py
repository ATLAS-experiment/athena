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
    "clusters": "TracccTritonClusters",
    "tracks": "TracccTritonTracks",
}


def TracccTritonDeviceRecoCfg(flags,
                              sequenceName: str = DEFAULT_SEQUENCE_NAME
                              ) -> ComponentAccumulator:
    """Set up the GPU chain that the Triton backend drives per request.

    @param sequenceName name of the AthSequencer the device algorithms are
           scheduled in; the Runner looks this up and executes it.
    """

    acc = ComponentAccumulator()

    cellsKey = STOREGATE_KEYS["cells"]
    measKey = STOREGATE_KEYS["measurements"]
    clustersKey = STOREGATE_KEYS["clusters"]
    tracksKey = STOREGATE_KEYS["tracks"]

    # Intermediate products, passed algorithm to algorithm inside the
    # sequence. Nothing outside this function ever names them.
    spKey = "TracccTritonSpacepoints"
    seedsKey = "TracccTritonSeeds"
    trkParamsKey = "TracccTritonTrackParameters"

    # ---- DetectorStore object names (device geometry + magnetic field) ----
    deviceGeometry = "TracccDeviceDetectorGeometry"
    deviceMagField = "TracccDeviceMagField"

    # Device detector description (geometry + digitization + athena<->detray
    # id map) built from the ACTS Gen3 tracking geometry by a service, plus
    # the conditions part built per IOV by a conditions algorithm
    from ActsGPUGeometry.ActsGPUGeometryConfig import (
        DeviceDetectorDescriptionCondAlgCfg,
    )
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(
        flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
        DeviceDetectorName=deviceGeometry,
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
        OutputTracccClusters=clustersKey,
        RetrieveClusterCells=True,
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

    # ---- Seeding: spacepoints -> seeds ----
    from ActsConfig.ActsConfigFlags import SeedingStrategy
    from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
        DeviceGBTSSeedingAlgCfg,
        DeviceTripletSeedingAlgCfg,
        DeviceTrkParamEstimationAlgCfg,
        DeviceTrackFindingAlgCfg,
    )
    if flags.Acts.Device.seedingStrategy in (SeedingStrategy.Gbts,
                                             SeedingStrategy.GbtsFtf):
        acc.merge(DeviceGBTSSeedingAlgCfg(
            flags,
            name="DeviceGBTSSeedingAlg",
            InputTracccPixelSpacepoints=spKey,
            InputTracccMeasurements=measKey,
            OutputTracccPixelSeeds=seedsKey,
        ), sequenceName=sequenceName)
    else:
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
