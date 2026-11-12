#!/usr/bin/env athena
 
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
   # Run script for the GPU seed conversion chain:
   #   RDO -> traccc cells -> traccc measurements -> traccc spacepoints/seeds
   #       -> ActsTrk seeds on host (reference) and on device (monitored)
   #       -> SeedValidationAlg comparing the two

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import (
      DeviceClusterizationAlgCfg
    , DeviceSPFormationAlgCfg
)
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
      RDOtoTracccCellConverterAlgCfg
    , TracccMeasurementConverterAlgCfg
    , TracccSeedConverterAlgCfg
    , TracccSeedDeviceConverterAlgCfg
    , SeedValidationAlgCfg
    , TracccMeasurementDeviceConverterAlgCfg
)

from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
      DeviceTripletSeedingAlgCfg
    , DeviceGBTSSeedingAlgCfg
)
from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg


def TracccSeedConversionAlgConfig(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    host_pixel_sps   = "ITkTracccPixelSpacepoints"
    meas_to_pixel_sp = "ITkTracccMeasToPixelSP"
    # Device side
    device_meas      = "TracccMeasurements"
    device_sps       = "TracccPixelSpacepoints"
    device_seeds     = "TracccPixelSeeds"   
    # Outputs
    ref_seeds        = "ITkTracccSeeds"
    mon_seeds        = "ITkTracccSeedsFromDevice"

    # geometry stuff
    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags
        , GeoIdMappingObjectName = "TracccGeometryIdMapping"
        ))

    # clustering stuff
    acc.merge(RDOtoTracccCellConverterAlgCfg(flags))
    acc.merge(DeviceClusterizationAlgCfg(flags))

    acc.merge(TracccMeasurementConverterAlgCfg(flags
        , InputMeasurements = device_meas
        , ConvertClustersWithCells = False
        ))

    device_meas_map = "TracccMeasToPixelIndex"
    if flags.Test.ExternalMeasMap:
        # Only here to provide the map; own cluster output keys so it does not
        # clash with the host measurement converter
        acc.merge(TracccMeasurementDeviceConverterAlgCfg(flags
            , InputMeasurements      = device_meas
            , OutputPixelClusters    = "ITkTracccPixelClustersFromDevice"
            , OutputStripClusters    = "ITkTracccStripClustersFromDevice"
            , OutputMeasToPixelIndex = device_meas_map
            ))
    
    # device pixel spacepoint formation
    acc.merge(DeviceSPFormationAlgCfg(flags
        , InputTracccMeasurements = device_meas
        , OutputTracccPixelSpacepoints = device_sps
    ))
    # device seed formation
    if flags.Test.UseGBTS:
        acc.merge(DeviceGBTSSeedingAlgCfg(flags
            , InputTracccPixelSpacepoints = device_sps
            , InputTracccMeasurements     = device_meas
            , OutputTracccPixelSeeds      = device_seeds
            , GeoIdMappingObjectName      = "TracccGeometryIdMapping"
            ))
    else:
        acc.merge(DeviceTripletSeedingAlgCfg(flags
            , InputTracccPixelSpacepoints = device_sps
            , OutputTracccPixelSeeds      = device_seeds
            ))
    
    # seed conversion -- host (ref)
    acc.merge(TracccSeedConverterAlgCfg(flags
        , InputSpacepoints       = host_pixel_sps
        , InputSpacepointsDevice = device_sps
        , InputMeasToPixelSP     = meas_to_pixel_sp
        , InputSeeds             = device_seeds
        , OutputSeeds            = ref_seeds
        ))
    
     # seed conversion -- device (mon)
    acc.merge(TracccSeedDeviceConverterAlgCfg(flags
        , InputSpacepoints       = host_pixel_sps
        , InputSpacepointsDevice = device_sps
        , InputMeasurements      = device_meas
        , InputSeeds             = device_seeds
        , OutputSeeds            = mon_seeds
        , InputMeasToPixelIndex  = device_meas_map if flags.Test.ExternalMeasMap else ""
        ))
    
    acc.merge(SeedValidationAlgCfg(flags
       , ReferenceSeeds           = ref_seeds
       , MonitoredSeeds           = mon_seeds
       , CompareQuality           = False
       ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True
    flags.Acts.TrackingGeometry.UseBlueprint = True

    flags.Detector.GeometryITkPixel = True
    flags.Detector.GeometryITkStrip = True
    flags.Detector.GeometryBpipe = True
    flags.Detector.GeometryCalo = False
    flags.Detector.GeometryMuon = False
    flags.Detector.GeometryHGTD = False

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Tracking.doPixelDigitalClustering = True
    flags.Tracking.doTruth = False
    flags.Exec.MaxEvents = -1
    flags.addFlag("Test.UseGBTS", True)
    flags.addFlag("Test.ExternalMeasMap", False)

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

    acc.merge(TracccSeedConversionAlgConfig(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"