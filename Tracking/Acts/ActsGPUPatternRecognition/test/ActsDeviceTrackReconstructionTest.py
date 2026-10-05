#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg, DeviceSPFormationAlgCfg
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceGBTSSeedingAlgCfg, DeviceTripletSeedingAlgCfg, DeviceTrkParamEstimationAlgCfg, DeviceTrackFindingAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg, TracccMeasurementConverterAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg

def GPUTrackingCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Service runs first — loads all device detector description data into detStore
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
        DeviceMagFieldObjectName="TracccMagneticField",
        HostMagFieldObjectName="TracccHostMagField",
        OutputLevel = DEBUG
    ))
   
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

    acc.merge(DeviceTrkParamEstimationAlgCfg(flags,
        InputTracccSpacepoints="TracccPixelSpacepointCollection",
        InputTracccMeasurements="TracccMeasurementCollection",
        InputTracccSeeds="TracccPixelGBTSSeedCollection",
        OutputTracccTrackParameters="TracccTrkParamCollection",
        InputTracccMagField="TracccMagneticField",
        OutputLevel = DEBUG
    ))

    acc.merge(DeviceTrackFindingAlgCfg(flags,
        InputTracccMeasurements="TracccMeasurementCollection",
        InputTracccMagField="TracccMagneticField",
        InputTracccTrackParameters="TracccTrkParamCollection",
        InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
        OutputTracccTracks="TracccTrackCollection",
        OutputLevel = DEBUG
    ))
    

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Tracking.doPixelDigitalClustering = True
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True

    # Keep calo/muon out of the tracking geometry: the calo volumes cannot be
    # converted to a consistent Detray geometry
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

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

    acc.merge(GPUTrackingCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"