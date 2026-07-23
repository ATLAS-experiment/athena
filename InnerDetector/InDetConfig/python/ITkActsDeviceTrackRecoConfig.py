# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import FlagEnum

from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg

class DataLocation(FlagEnum):
    HOST   = "host"    # Athena objects on CPU
    DEVICE = "device"  # Traccc buffers on GPU

def ITkActsDeviceTrackRecoCfg(flags, *, previousExtension=None):
    acc = ComponentAccumulator()

    # Bring up shared device infrastructure once, upfront
    print(f"Setting up GPU algorithms with {flags.Device.Backend.value} backend")

    # Setup traccc detector description objects — loads all device detector description data into detStore
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig"
    ))

    # --- Clusterization ---
    if flags.Acts.Device.doClusterization:

        # Create RoI for secondary passes (e.g. LargeD0) to reuse
        from ActsConfig.ActsRegionsOfInterestConfig import ActsRegionsOfInterestCreatorAlgCfg
        acc.merge(ActsRegionsOfInterestCreatorAlgCfg(flags,
            name=f"{flags.Tracking.ActiveConfig.extension}RegionsOfInterestCreatorAlg"))

        print("Performing clusterization on device")
        # setup RDO converter
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg
        acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
            TracccCells = "TracccCellCollection",
            HostConditionsObjectName="TracccHostCondConfig"
        ))

        # setup traccc clusterization
        from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg
        acc.merge(DeviceClusterizationAlgCfg(flags,
            InputTracccCells="TracccCellCollection",
            OutputTracccMeasurements="TracccMeasurementCollection",
            OutputTracccClusters="TracccClusterCollection",
            RetrieveClusterCells=flags.Tracking.doTruth,
            previousExtension=previousExtension))
        clustersLocation = DataLocation.DEVICE

    else:
        from InDetConfig.ITkActsDataPreparationConfig import ITkActsDataPreparationCfg
        acc.merge(ITkActsDataPreparationCfg(flags, previousExtension=previousExtension))
        clustersLocation = DataLocation.HOST

    # --- Seeding ---
    if flags.Acts.Device.doSeeding:

        raise ValueError("Unsupported: no device seeding yet!")

    else:

        # If clusterization was on device, need to copy them to host first
        # and schedule space point formation
        if clustersLocation is DataLocation.DEVICE:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
            acc.merge(TracccMeasurementConverterAlgCfg(flags,
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputCells="TracccCellCollection",
                ConvertClustersWithCells = flags.Tracking.doTruth,
                OutputPixelClusters="ITkPixelClusters",
                OutputStripClusters="ITkStripClusters"
            ))
            clustersLocation = DataLocation.HOST

            from ActsConfig.ActsSpacePointFormationConfig import ActsSpacePointFormationCfg
            acc.merge(ActsSpacePointFormationCfg(flags, previousActsExtension=previousExtension))

        from ActsConfig.ActsSeedingConfig import ActsSeedingCfg
        acc.merge(ActsSeedingCfg(flags))
        seedsLocation = DataLocation.HOST


    # --- Track Reconstruction ---
    if flags.Acts.Device.doTrackReconstruction:

        raise ValueError("Unsupported operation, we do not have this step on device yet")

    else:

        # If clusterization was on device, need to copy the measurements to host first
        if clustersLocation is DataLocation.DEVICE:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
            acc.merge(TracccMeasurementConverterAlgCfg(flags,
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputTracccCells="TracccCellCollection",
                ConvertClustersWithCells = flags.Tracking.doTruth,
                OutputPixelClusters="ITkPixelClusters",
                OutputStripClusters="ITkStripClusters"
            ))

        if seedsLocation is DataLocation.DEVICE:
            raise ValueError("Unsupported operation, we do not have this conversion yet")

        # CKF
        from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
        acc.merge(ActsTrackFindingCfg(flags))

        # Ambiguity Resolution
        if flags.Acts.doAmbiguityResolution:
            from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
            acc.merge(ActsAmbiguityResolutionCfg(flags))


    # PRD association
    from ActsConfig.ActsPrdAssociationConfig import ActsPrdAssociationAlgCfg
    acc.merge(ActsPrdAssociationAlgCfg(flags,
                                       name = f'{flags.Tracking.ActiveConfig.extension}PrdAssociationAlg',
                                       previousActsExtension = previousExtension))

    # Truth
    if flags.Tracking.doTruth:

        # schedule association of measurements to truth particles
        from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
        acc.merge(ActsTruthAssociationAlgCfg(flags))
        acc.merge(ActsTruthParticleHitCountAlgCfg(flags))
        if flags.Acts.doTruthInspection:
            from ActsConfig.ActsInspectTruthContentConfig import ActsInspectTruthContentAlgCfg
            acc.merge(ActsInspectTruthContentAlgCfg(flags))

        # Run truth on CKF tracks
        # This is only necessary if we are asking for these tracks to be persistified with the
        # - flag: Tracking.ActiveConfig.storeSiSPSeededTracks set to True OR
        # - flag: flags.Acts.doAmbiguityResolution set to False
        if flags.Tracking.ActiveConfig.storeSiSPSeededTracks or not flags.Acts.doAmbiguityResolution:
            from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acts_tracks = f"{flags.Tracking.ActiveConfig.extension}Tracks"
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))

            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))

        # Run truth on the tracks from ambiguity resolution. This is only necessary if
        # - flag: flags.Acts.doAmbiguityResolution set to True
        if flags.Acts.doAmbiguityResolution:
            acts_tracks = f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
            from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))

            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))

    return acc
