#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the device large radius strip seeding:
#   RDO -> xAOD clusters -> xAOD strip space points -> traccc strip spacepoints -> traccc seeds
# The host large radius strip seeding runs on the same space points for comparison.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    xAODToTracccMeasurementConverterAlgCfg,
    xAODToTracccSpacePointConverterAlgCfg,
)
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
    DeviceLargeRadiusStripTripletSeedingAlgCfg,
)

STRIP_SPACEPOINTS = ["ITkStripSpacePoints", "ITkStripOverlapSpacePoints"]

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

def HostLargeRadiusStripSeedingCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsSeedingConfig import ActsStripSeedingAlgCfg
    acc.merge(ActsStripSeedingAlgCfg(flags,
        InputSpacePoints=STRIP_SPACEPOINTS,
        OutputSeeds="ActsLargeRadiusStripSeeds",
        OutputLevel=DEBUG,
    ))

    return acc

def DeviceLargeRadiusStripSeedingCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
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
        name="xAODToTracccStripSpacePointConverterAlg",
        InputSpacePoints=STRIP_SPACEPOINTS,
        InputMeasToCluster="TracccMeasToStripCluster",
        OutputTracccSpacepoints="TracccStripSpacepointCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceLargeRadiusStripTripletSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccStripSpacepointCollection",
        OutputTracccPixelSeeds="TracccLargeRadiusStripSeedCollection",
        OutputLevel=DEBUG,
    ))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 1

    flags.Tracking.doPixelDigitalClustering = True

    flags.fillFromArgs()

    # Data preparation runs with the main pass, seeding with the large radius pass
    mainFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsPass")
    lrtFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        "Tracking.ITkActsLargeRadiusPass")

    mainFlags.lock()
    lrtFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(mainFlags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(mainFlags))

    msg_svc = acc.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(mainFlags.Common.MsgSourceLength)

    # Needed for PixelID and SCT_ID
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(mainFlags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(mainFlags))

    acc.merge(HostDataPreparationCfg(mainFlags))
    acc.merge(HostLargeRadiusStripSeedingCfg(lrtFlags))
    acc.merge(DeviceLargeRadiusStripSeedingCfg(lrtFlags))
    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(mainFlags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
