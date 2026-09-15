#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the host data preparation + device pattern recognition chain:
#   RDO -> xAOD clusters -> xAOD space points -> traccc measurements/spacepoints
#       -> traccc seeds -> traccc track parameters -> traccc tracks -> ACTS tracks

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    xAODToTracccMeasurementConverterAlgCfg,
    xAODToTracccSpacePointConverterAlgCfg,
    TracccTrackConverterAlgCfg,
)
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
    DeviceTripletSeedingAlgCfg,
    DeviceTrkParamEstimationAlgCfg,
    DeviceTrackFindingAlgCfg,
)

def HostDataPreparationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags))

    from ActsConfig.ActsClusterizationConfig import (
        ActsPixelClusterizationAlgCfg,
        ActsStripClusterizationAlgCfg,
    )
    acc.merge(ActsPixelClusterizationAlgCfg(flags))
    acc.merge(ActsStripClusterizationAlgCfg(flags))

    from ActsConfig.ActsSpacePointFormationConfig import (
        ActsPixelSpacePointFormationAlgCfg,
        ActsStripSpacePointFormationAlgCfg,
    )
    acc.merge(ActsPixelSpacePointFormationAlgCfg(flags))
    acc.merge(ActsStripSpacePointFormationAlgCfg(flags))

    return acc

def DevicePatternRecognitionCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Services run first — load device detector description and magnetic field into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
    ))
    acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
        DeviceMagFieldObjectName="TracccMagneticField",
        HostMagFieldObjectName="TracccHostMagField",
    ))

    acc.merge(xAODToTracccMeasurementConverterAlgCfg(flags,
        InputPixelClusters="ITkPixelClusters",
        InputStripClusters="ITkStripClusters",
        OutputTracccMeasurements="TracccMeasurementCollection",
        OutputMeasToPixelCluster="TracccMeasToPixelCluster",
        OutputMeasToStripCluster="TracccMeasToStripCluster",
        OutputLevel=DEBUG,
    ))

    acc.merge(xAODToTracccSpacePointConverterAlgCfg(flags,
        name="xAODToTracccPixelSpacePointConverterAlg",
        InputSpacePoints=["ITkPixelSpacePoints"],
        InputMeasToCluster="TracccMeasToPixelCluster",
        OutputTracccSpacepoints="TracccPixelSpacepointCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(xAODToTracccSpacePointConverterAlgCfg(flags,
        name="xAODToTracccStripSpacePointConverterAlg",
        InputSpacePoints=["ITkStripSpacePoints", "ITkStripOverlapSpacePoints"],
        InputMeasToCluster="TracccMeasToStripCluster",
        OutputTracccSpacepoints="TracccStripSpacepointCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceTripletSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
        OutputTracccPixelSeeds="TracccPixelSeedCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceTrkParamEstimationAlgCfg(flags,
        InputTracccSpacepoints="TracccPixelSpacepointCollection",
        InputTracccMeasurements="TracccMeasurementCollection",
        InputTracccSeeds="TracccPixelSeedCollection",
        InputTracccMagField="TracccMagneticField",
        OutputTracccTrackParameters="TracccTrkParamCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceTrackFindingAlgCfg(flags,
        InputTracccMeasurements="TracccMeasurementCollection",
        InputTracccMagField="TracccMagneticField",
        InputTracccTrackParameters="TracccTrkParamCollection",
        InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
        OutputTracccTracks="TracccTrackCollection",
        OutputLevel=DEBUG,
    ))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsTrackingGeometrySvcCfg(flags))
    acc.merge(TracccTrackConverterAlgCfg(flags,
        InputPixelClusters="ITkPixelClusters",
        InputStripClusters="ITkStripClusters",
        InputMeasToPixelSP="TracccMeasToPixelCluster",
        InputMeasToStripCl="TracccMeasToStripCluster",
        InputTracks="TracccTrackCollection",
        OutputTracks="ActsTracccTracks",
        GeoIdMapping="TracccGeometryIdMapping",
        HostDetectorName="TracccHostDetectorGeometry",
        OutputLevel=DEBUG,
    ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 1

    # Set the Main Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")

    flags.Tracking.doPixelDigitalClustering = True

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

    acc.merge(HostDataPreparationCfg(flags))
    acc.merge(DevicePatternRecognitionCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(flags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
