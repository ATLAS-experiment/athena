#!/usr/bin/env athena

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Run script for the large radius tracking pass on the device:
#   main pass (host) -> large radius pass data preparation (host)
#     -> traccc measurements and strip spacepoints -> traccc seeds
#     -> traccc track parameters -> traccc tracks -> ACTS tracks -> truth matching
# The host large radius pass runs on the same inputs for comparison.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    xAODToTracccMeasurementConverterAlgCfg,
    xAODToTracccSpacePointConverterAlgCfg,
    TracccTrackConverterAlgCfg,
)
from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
    DeviceLargeRadiusStripTripletSeedingAlgCfg,
    DeviceTrkParamEstimationAlgCfg,
    DeviceLargeRadiusTrackFindingAlgCfg,
)

def DeviceLargeRadiusTrackingCfg(flags, previousExtension: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    prefix = f"ITk{flags.Tracking.ActiveConfig.extension.replace('Acts', '')}"
    pixelClusters = f"{prefix}PixelClusters"
    stripClusters = f"{prefix}StripClusters"
    stripSpacePoints = [f"{prefix}StripSpacePoints", f"{prefix}StripOverlapSpacePoints"]
    tracks = f"{flags.Tracking.ActiveConfig.extension}DeviceTracks"

    # Services run first — load device detector description and magnetic field into detStore
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags))
    acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
        DeviceMagFieldObjectName="TracccMagneticField",
        HostMagFieldObjectName="TracccHostMagField",
    ))

    acc.merge(xAODToTracccMeasurementConverterAlgCfg(flags,
        name="xAODToTracccLargeRadiusMeasurementConverterAlg",
        InputPixelClusters=pixelClusters,
        InputStripClusters=stripClusters,
        OutputTracccMeasurements="TracccLargeRadiusMeasurementCollection",
        OutputMeasToPixelCluster="TracccLargeRadiusMeasToPixelCluster",
        OutputMeasToStripCluster="TracccLargeRadiusMeasToStripCluster",
        OutputLevel=DEBUG,
    ))

    acc.merge(xAODToTracccSpacePointConverterAlgCfg(flags,
        name="xAODToTracccLargeRadiusStripSpacePointConverterAlg",
        InputSpacePoints=stripSpacePoints,
        InputMeasToCluster="TracccLargeRadiusMeasToStripCluster",
        OutputTracccSpacepoints="TracccLargeRadiusStripSpacepointCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceLargeRadiusStripTripletSeedingAlgCfg(flags,
        InputTracccPixelSpacepoints="TracccLargeRadiusStripSpacepointCollection",
        OutputTracccPixelSeeds="TracccLargeRadiusStripSeedCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceTrkParamEstimationAlgCfg(flags,
        name="DeviceLargeRadiusTrkParamEstimationAlg",
        InputTracccSpacepoints="TracccLargeRadiusStripSpacepointCollection",
        InputTracccMeasurements="TracccLargeRadiusMeasurementCollection",
        InputTracccSeeds="TracccLargeRadiusStripSeedCollection",
        InputTracccMagField="TracccMagneticField",
        OutputTracccTrackParameters="TracccLargeRadiusTrkParamCollection",
        OutputLevel=DEBUG,
    ))

    acc.merge(DeviceLargeRadiusTrackFindingAlgCfg(flags,
        InputTracccMeasurements="TracccLargeRadiusMeasurementCollection",
        InputTracccMagField="TracccMagneticField",
        InputTracccTrackParameters="TracccLargeRadiusTrkParamCollection",
        InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
        OutputTracccTracks="TracccLargeRadiusTrackCollection",
        OutputLevel=DEBUG,
    ))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsTrackingGeometrySvcCfg(flags))
    acc.merge(TracccTrackConverterAlgCfg(flags,
        name="TracccLargeRadiusTrackConverterAlg",
        InputPixelClusters="ITkPixelClusters",
        InputStripClusters="ITkStripClusters",
        InputMeasToPixelSP="TracccLargeRadiusMeasToPixelCluster",
        InputMeasToStripCl="TracccLargeRadiusMeasToStripCluster",
        InputTracks="TracccLargeRadiusTrackCollection",
        OutputTracks=tracks,
        HostDetectorName="TracccHostDetectorGeometry",
        OutputLevel=DEBUG,
    ))

    if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
        acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
            name=f"{tracks}TrackToTruthAssociationAlg",
            ACTSTracksLocation=tracks,
            AssociationMapOut=f"{tracks}ToTruthParticleAssociation"))
        acc.merge(ActsTrackFindingValidationAlgCfg(flags,
            name=f"{tracks}TrackFindingValidationAlg",
            TrackToTruthAssociationMap=f"{tracks}ToTruthParticleAssociation"))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()

    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.Exec.MaxEvents = 1

    flags.Tracking.doPixelDigitalClustering = True
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True

    # Keep calo/muon out of the tracking geometry: the calo volumes cannot be
    # converted to a consistent Detray geometry
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

    # Truth on the host tracks is only run for the CKF output
    flags.Tracking.ITkActsLargeRadiusPass.storeSiSPSeededTracks = True

    flags.fillFromArgs()

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

    # Truth conversion, needed by the truth association of the clusters
    if mainFlags.Input.isMC and mainFlags.Output.doGEN_AOD2xAOD:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        acc.merge(GEN_AOD2xAODCfg(mainFlags))

    # Main pass on the host, providing the PRD association for the large radius pass
    from InDetConfig.ITkActsDataPreparationConfig import ITkActsDataPreparationCfg
    from InDetConfig.ITkActsPatternRecognitionConfig import ITkActsTrackReconstructionCfg
    acc.merge(ITkActsDataPreparationCfg(mainFlags))
    acc.merge(ITkActsTrackReconstructionCfg(mainFlags))

    # Large radius pass data preparation on the host
    previousExtension = mainFlags.Tracking.ActiveConfig.extension
    acc.merge(ITkActsDataPreparationCfg(lrtFlags, previousExtension=previousExtension))

    # Large radius pass reconstruction on the host and on the device
    acc.merge(ITkActsTrackReconstructionCfg(lrtFlags, previousExtension=previousExtension))
    acc.merge(DeviceLargeRadiusTrackingCfg(lrtFlags, previousExtension=previousExtension))

    acc.printConfig(withDetails=True, summariseProps=True)

    statusCode = acc.run(mainFlags.Exec.MaxEvents)
    assert statusCode.isSuccess(), "Application execution did not succeed"
