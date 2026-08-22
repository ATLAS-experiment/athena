#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg, DeviceSPFormationAlgCfg
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceGBTSSeedingAlgCfg, DeviceTripletSeedingAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg

def GPUSeedingCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        # DeviceDetectorName = "TracccDeviceDetectorGeometry",
        # HostDetectorName = "TracccHostDetectorGeometry",
        OutputLevel = DEBUG))

    acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
        TracccCells = "TracccCellCollection",
        OutputLevel = DEBUG
    ))

    acc.merge(DeviceClusterizationAlgCfg(flags,
        InputTracccCells="TracccCellCollection",
        OutputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccClusters="TracccClusterCollection",
        RetrieveClusterCells=flags.Tracking.doTruth,
        OutputLevel = DEBUG,
            
    ))

    acc.merge(DeviceSPFormationAlgCfg(flags,
        InputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        OutputLevel = DEBUG))

    acc.merge(DeviceTripletSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        OutputTracccPixelSeeds="TracccPixelTripletSeedCollection",
        OutputLevel = DEBUG))

    acc.merge(DeviceGBTSSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        InputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccPixelSeeds="TracccPixelGBTSSeedCollection",
        OutputLevel = DEBUG))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Tracking.doPixelDigitalClustering = True

    flags.Exec.MaxEvents = 1

    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    acc.merge(GPUSeedingCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"