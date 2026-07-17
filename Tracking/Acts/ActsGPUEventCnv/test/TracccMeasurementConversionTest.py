#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the full GPU clusterization chain:
#   RDO -> traccc cells -> traccc measurements -> xAOD clusters

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG
from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import (
      JSONDeviceDetectorDescriptionProviderSvcCfg
    , DeviceDetectorDescriptionCondAlgCfg
)
from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
      RDOtoTracccCellConverterAlgCfg
    , TracccMeasurementConverterAlgCfg
    , TracccMeasurementDeviceConverterAlgCfg
    , ClusterValidationAlgCfg
)

def TracccMeasurementConversionCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))

    # At the moment the JSON service provides the detector objects
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags
        # , HostConditionsObjectName="JSONTracccHostCondConfig"
        # , HostDigitizationObjectName="JSONTracccHostDigitizationConfig"
        # , DeviceConditionsObjectName="JSONTracccDeviceCondConfig"
        # , DeviceDigitizationObjectName="JSONTracccDeviceDigitizationConfig"
        # , DeviceDetectorName="JSONTracccDeviceDetectorGeometry"
        # , HostDetectorName="JSONTracccHostDetectorGeometry"
        # , OutputLevel = DEBUG
    ))
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags
        # , HostDetectorName = "JSONTracccHostDetectorGeometry"
        , HostConditionsObjectName="DynTracccHostCondConfig"
        , HostDigitizationObjectName="DynTracccHostDigitizationConfig"
        , DeviceConditionsObjectName="DynTracccDeviceCondConfig"
        , DeviceDigitizationObjectName="DynTracccDeviceDigitizationConfig"
        # , OutputLevel = DEBUG
    ))
    acc.merge(RDOtoTracccCellConverterAlgCfg(flags))
    acc.merge(DeviceClusterizationAlgCfg(flags))

    ref_pixels = "ITkTracccPixelClusters"
    ref_strips = "ITkTracccStripClusters"
    mon_pixels = "ITkTracccPixelClustersFromDevice"
    mon_strips = "ITkTracccStripClustersFromDevice"

    acc.merge(TracccMeasurementConverterAlgCfg(flags
        , OutputPixelClusters = ref_pixels
        , OutputStripClusters = ref_strips
        , ConvertClustersWithCells = False
        , OutputLevel = DEBUG
        ))
    acc.merge(TracccMeasurementDeviceConverterAlgCfg(flags
        , DeviceConditionsObjectName = "DynTracccDeviceCondConfig"
        , OutputPixelClusters = mon_pixels
        , OutputStripClusters = mon_strips
        , OutputLevel = DEBUG
        ))
    acc.merge(ClusterValidationAlgCfg(flags
        , ReferencePixels = ref_pixels
        , ReferenceStrips = ref_strips
        , MonitoredPixels = mon_pixels
        , MonitoredStrips = mon_strips
        ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    # ---- Input ----
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Tracking.doPixelDigitalClustering = True
    flags.Tracking.doTruth = False
    
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

    acc.merge(TracccMeasurementConversionCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"