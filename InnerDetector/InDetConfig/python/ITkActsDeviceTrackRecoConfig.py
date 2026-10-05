# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import FlagEnum

from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg

class DataLocation(FlagEnum):
    HOST   = "host"    # Athena objects on CPU
    DEVICE = "device"  # Traccc buffers on GPU

def ITkActsDeviceTrackRecoCfg(flags, *, previousExtension=None):
    acc = ComponentAccumulator()

    # Bring up shared device infrastructure once, upfront
    print(f"Setting up GPU algorithms with {flags.Device.Backend.value} backend")

    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    # --- Clusterization ---
    if flags.Acts.Device.doClusterization:

        #TODO: remove this once MC is fixed
        if not flags.Tracking.doPixelDigitalClustering:
            raise ValueError("clusterization on device is not compatible "
                "with analog clustering at the moment due to incorrent "
                "ToT values for Pixel hits in the simulation data.")

        # Create RoI for secondary passes (e.g. LargeD0) to reuse
        from ActsConfig.ActsRegionsOfInterestConfig import ActsRegionsOfInterestCreatorAlgCfg
        acc.merge(ActsRegionsOfInterestCreatorAlgCfg(flags,
            name=f"{flags.Tracking.ActiveConfig.extension}RegionsOfInterestCreatorAlg"))

        print("Performing clusterization on device")

        # setup RDO converter
        if flags.Acts.EDM.PhaseII :
            from ActsConfig.ActsPhaseIIRawDataEdmConfig import (
                PhaseIIPixelRawDataContainerCfg,
                PhaseIIStripRawDataContainerCfg,
            )
            acc.merge(PhaseIIPixelRawDataContainerCfg(flags))
            acc.merge(PhaseIIStripRawDataContainerCfg(flags))
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import PhaseIIRDOtoTracccCellConverterAlgCfg
            acc.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
                TracccCells = "TracccCellCollection",
                ))
        else:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg
            acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
                TracccCells = "TracccCellCollection",
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

        if clustersLocation is not DataLocation.DEVICE:
            raise ValueError("Device seeding requires device clusterization "
                "(flags.Acts.Device.doClusterization=True): it reads the "
                "traccc measurement collection straight out of device memory "
                "and there is currently no host->device measurement converter.")
 
        # Pixel space point formation on device
        from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceSPFormationAlgCfg
        acc.merge(DeviceSPFormationAlgCfg(flags,
            name="DeviceSPFormationAlg",
            InputTracccMeasurements="TracccMeasurementCollection",
            OutputTracccPixelSpacepoints="TracccPixelSpacepointCollection"))

        from ActsConfig.ActsConfigFlags import SeedingStrategy

        print(f"Performing seeding on device with seeding strategy set to {flags.Acts.Device.seedingStrategy}")
        if flags.Acts.Device.seedingStrategy in [SeedingStrategy.Gbts, SeedingStrategy.GbtsFtf]:
            from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceGBTSSeedingAlgCfg
            acc.merge(DeviceGBTSSeedingAlgCfg(flags,
                name="DeviceGBTSSeedingAlg",
                InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
                InputTracccMeasurements="TracccMeasurementCollection",
                OutputTracccPixelSeeds="TracccPixelSeedCollection"))
        else:
            from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceTripletSeedingAlgCfg
            acc.merge(DeviceTripletSeedingAlgCfg(flags,
                name="DeviceTripletSeedingAlg",
                InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
                OutputTracccPixelSeeds="TracccPixelSeedCollection"))

        seedsLocation = DataLocation.DEVICE

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
                OutputPixelSpacePoints="ITkPixelSpacePoints",
                OutputPixelClusters="ITkPixelClusters",
                OutputStripClusters="ITkStripClusters"
            ))

            from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
            acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
                name=f"{flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg",
                StripClusters="ITkStripClusters",
                StripSpacePoints="ITkStripSpacePoints",
                StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))

            clustersLocation = DataLocation.HOST

            # Truth (as configured in ITkActsDataPreparationCfg)
            # this truth must only be done if you do PRD and SpacePointformation
            # If you only do the latter (== running on ESD) then the needed input (simdata)
            # is not in ESD but the resulting truth (clustertruth) is already there ...
            if flags.Tracking.doTruth:
                from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
                acc.merge(ActsTruthAssociationAlgCfg(flags))
                acc.merge(ActsTruthParticleHitCountAlgCfg(flags))

        from ActsConfig.ActsSeedingConfig import ActsSeedingCfg
        acc.merge(ActsSeedingCfg(flags))
        seedsLocation = DataLocation.HOST


    # --- Track Reconstruction ---
    if flags.Acts.Device.doTrackReconstruction:

        if clustersLocation is not DataLocation.DEVICE or seedsLocation is not DataLocation.DEVICE:
            raise ValueError("Device track reconstruction requires device clusterization "
                "(flags.Acts.Device.doClusterization=True) and device seeding "
                "(flags.Acts.Device.doSeeding=True): it reads the traccc measurement "
                "and seed collections straight out of device memory and there is "
                "currently no host->device converter for these yet.")


        from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg
        acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
            DeviceMagFieldObjectName="TracccMagneticField",
            HostMagFieldObjectName="TracccHostMagField",
        ))

        from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceTrkParamEstimationAlgCfg, DeviceTrackFindingAlgCfg
        acc.merge(DeviceTrkParamEstimationAlgCfg(flags,
            InputTracccSpacepoints="TracccPixelSpacepointCollection",
            InputTracccMeasurements="TracccMeasurementCollection",
            InputTracccSeeds="TracccPixelSeedCollection",
            InputTracccMagField="TracccMagneticField",
            OutputTracccTrackParameters="TracccTrackParameterCollection",
        ))

        acc.merge(DeviceTrackFindingAlgCfg(flags,
            InputTracccMeasurements="TracccMeasurementCollection",
            InputTracccMagField="TracccMagneticField",
            InputTracccTrackParameters="TracccTrackParameterCollection",
            InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
            OutputTracccTracks="TracccTrackCollection",
        ))
            
        if clustersLocation is not DataLocation.DEVICE:
            acc.merge(TracccMeasurementConverterAlgCfg(flags,
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputCells="TracccCellCollection",
                ConvertClustersWithCells = flags.Tracking.doTruth,
                OutputPixelClusters="ITkPixelClusters",
                OutputPixelSpacePoints="ITkPixelSpacePoints",
                OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
                OutputMeasToStripCl="ITkTracccMeasToStripCl",
                OutputStripClusters="ITkStripClusters",
                GeoIdMapping="TracccGeoIdMapping",
            ))

        from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccTrackConverterAlgCfg, TracccMeasurementConverterAlgCfg
        if clustersLocation is DataLocation.DEVICE:
            acc.merge(TracccMeasurementConverterAlgCfg(flags,
                    InputMeasurements="TracccMeasurementCollection",
                    InputClusters="TracccClusterCollection",
                    InputCells="TracccCellCollection",
                    ConvertClustersWithCells = flags.Tracking.doTruth,
                    OutputPixelClusters="ITkPixelClusters",
                    OutputPixelSpacePoints="ITkPixelSpacePoints",
                    OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
                    OutputMeasToStripCl="ITkTracccMeasToStripCl",
                    OutputStripClusters="ITkStripClusters"
            ))

            # Strip clusters were just produced above, but nothing forms strip
            # space points on the device path — mirror the host-side flow
            if flags.Tracking.ActiveConfig.useITkStripSeeding or (flags.Acts.SpacePoints.doStrip and not flags.Tracking.ActiveConfig.isSecondaryPass):
                from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
                acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
                    name=f"{flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg",
                    StripClusters="ITkStripClusters",
                    StripSpacePoints="ITkStripSpacePoints",
                    StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))

        acc.merge(TracccTrackConverterAlgCfg(flags,
            InputPixelClusters="ITkPixelClusters",
            InputStripClusters="ITkStripClusters",
            InputMeasToPixelSP="ITkTracccMeasToPixelSP",
            InputMeasToStripCl="ITkTracccMeasToStripCl",
            InputTracks="TracccTrackCollection",
            OutputTracks=f"{flags.Tracking.ActiveConfig.extension}Tracks",
        ))



    else:

        # If clusterization was on device, need to copy the measurements to host first
        if clustersLocation is DataLocation.DEVICE:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
            acc.merge(TracccMeasurementConverterAlgCfg(flags,
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputCells="TracccCellCollection",
                ConvertClustersWithCells = flags.Tracking.doTruth,
                OutputPixelClusters="ITkPixelClusters",
                OutputPixelSpacePoints="ITkPixelSpacePoints",
                OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
                OutputMeasToStripCl="ITkTracccMeasToStripCl",
                OutputStripClusters="ITkStripClusters"
            ))
            from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
            acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
                name=f"{flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg",
                StripClusters="ITkStripClusters",
                StripSpacePoints="ITkStripSpacePoints",
                StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))
            clustersLocation = DataLocation.HOST

        if seedsLocation is DataLocation.DEVICE:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccSeedConverterAlgCfg
            
            acc.merge(TracccSeedConverterAlgCfg(flags,
                name="TracccSeedConverterAlg",
                InputSpacepointsDevice="TracccPixelSpacepointCollection",
                InputSpacepoints="ITkPixelSpacePoints",
                InputMeasToPixelSP="ITkTracccMeasToPixelSP",
                InputSeeds="TracccPixelSeedCollection",
                OutputSeeds=f'{flags.Tracking.ActiveConfig.extension}PixelSeeds'))
            seedsLocation = DataLocation.HOST
                            
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

        # Extract track parameters from device seeds if requested
        if flags.Tracking.ActiveConfig.storeTrackSeeds and flags.Acts.Device.doSeeding: # for clustering only pipelines this is controlled via the ActsSeedingConfig file
            from ActsConfig.ActsSeedingConfig import ActsStoreTrackSeedsCfg
            from ActsConfig.ActsAnalysisConfig import ActsPixelSeedsToTrackParamsAlgCfg, ActsStripSeedsToTrackParamsAlgCfg
            processPixels = flags.Tracking.ActiveConfig.useITkPixelSeeding
            processStrips = flags.Tracking.ActiveConfig.useITkStripSeeding

            prefix = flags.Tracking.ActiveConfig.extension
            
            if flags.Acts.Device.doTrackReconstruction:
                from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccSeedConverterAlgCfg
                acc.merge(TracccSeedConverterAlgCfg(flags,
                    name="TracccSeedConverterAlg",
                    InputSpacepointsDevice="TracccPixelSpacepointCollection",
                    InputSpacepoints="ITkPixelSpacePoints",
                    InputMeasToPixelSP="ITkTracccMeasToPixelSP",
                    InputSeeds="TracccPixelSeedCollection",
                    OutputSeeds=f'{flags.Tracking.ActiveConfig.extension}PixelSeeds'))
            
            # Create track parameters before ActsStoreTrackSeedsCfg (following ActsSeedingCfg pattern)
            if processPixels:
                acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(
                    flags,
                    name = prefix + 'PixelSeedsToTrackParamsAlg',
                    InputSeedContainerKey = prefix + 'PixelSeeds',
                    OutputTrackParamsCollectionKey = prefix + 'PixelEstimatedTrackParams'))
            if processStrips:
                acc.merge(ActsStripSeedsToTrackParamsAlgCfg(
                    flags,
                    name = prefix + 'StripSeedsToTrackParamsAlg',
                    InputSeedContainerKey = prefix + 'StripSeeds',
                    OutputTrackParamsCollectionKey = prefix + 'StripEstimatedTrackParams'))
                
            if processPixels:
                acc.merge(ActsStoreTrackSeedsCfg(flags,processPixels=True, processStrips=False))
            if processStrips:
                acc.merge(ActsStoreTrackSeedsCfg(flags,processPixels=False, processStrips=True))
            if processPixels and processStrips:
                acc.merge(ActsStoreTrackSeedsCfg(flags,processPixels=True, processStrips=True))
                
    return acc


def ITkActsDeviceSecondaryPassTrackRecoCfg(flags, *, previousExtension=None):
    """Secondary tracking pass with data preparation on the host and
    seeding and track finding on the device.

    The clusters and space points of the pass are prepared on the host from
    the objects of the previous pass (PRD association), converted to traccc
    measurements and spacepoints, and the seeds and tracks are reconstructed
    on the device. The tracks are converted back to an ACTS track container
    named as for the host reconstruction of the pass.
    """
    acc = ComponentAccumulator()

    if previousExtension is None:
        raise ValueError("A secondary pass on the device requires a previous pass")

    extension = flags.Tracking.ActiveConfig.extension
    prefix = f"ITk{extension.replace('Acts', '')}"
    pixelClusters = f"{prefix}PixelClusters"
    stripClusters = f"{prefix}StripClusters"

    if flags.Tracking.ActiveConfig.useITkPixelSeeding:
        raise ValueError("Pixel seeding on the device is not supported for secondary passes")
    if not flags.Tracking.ActiveConfig.useITkStripSeeding:
        raise ValueError("Secondary pass on the device requires strip seeding")

    print(f"Setting up GPU algorithms for the {extension} pass with {flags.Device.Backend.value} backend")

    # Setup traccc detector description objects — loads all device detector description data into detStore
    acc.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg
    acc.merge(JSONDeviceMagFieldProviderSvcCfg(flags,
        DeviceMagFieldObjectName="TracccMagneticField",
        HostMagFieldObjectName="TracccHostMagField",
    ))

    # --- Data preparation on the host ---
    from InDetConfig.ITkActsDataPreparationConfig import ITkActsDataPreparationCfg
    acc.merge(ITkActsDataPreparationCfg(flags, previousExtension=previousExtension))

    # --- Conversion to traccc ---
    measurements = f"Traccc{extension}MeasurementCollection"
    measToPixelCluster = f"Traccc{extension}MeasToPixelCluster"
    measToStripCluster = f"Traccc{extension}MeasToStripCluster"
    stripSpacepoints = f"Traccc{extension}StripSpacepointCollection"

    from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
        xAODToTracccMeasurementConverterAlgCfg,
        xAODToTracccSpacePointConverterAlgCfg,
    )
    acc.merge(xAODToTracccMeasurementConverterAlgCfg(flags,
        name=f"{extension}MeasurementConverterAlg",
        InputPixelClusters=pixelClusters,
        InputStripClusters=stripClusters,
        OutputTracccMeasurements=measurements,
        OutputMeasToPixelCluster=measToPixelCluster,
        OutputMeasToStripCluster=measToStripCluster,
    ))

    stripSpacePoints = [f"{prefix}StripSpacePoints"]
    if extension != "ActsConversion":
        stripSpacePoints += [f"{prefix}StripOverlapSpacePoints"]
    acc.merge(xAODToTracccSpacePointConverterAlgCfg(flags,
        name=f"{extension}StripSpacePointConverterAlg",
        InputSpacePoints=stripSpacePoints,
        InputMeasToCluster=measToStripCluster,
        OutputTracccSpacepoints=stripSpacepoints,
    ))

    # --- Seeding ---
    seeds = f"Traccc{extension}StripSeedCollection"
    from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import (
        DeviceLargeRadiusStripTripletSeedingAlgCfg,
        DeviceTrkParamEstimationAlgCfg,
        DeviceLargeRadiusTrackFindingAlgCfg,
    )
    acc.merge(DeviceLargeRadiusStripTripletSeedingAlgCfg(flags,
        name=f"{extension}DeviceStripTripletSeedingAlg",
        InputTracccPixelSpacepoints=stripSpacepoints,
        OutputTracccPixelSeeds=seeds,
    ))

    # --- Track Reconstruction ---
    trackParameters = f"Traccc{extension}TrackParameterCollection"
    tracccTracks = f"Traccc{extension}TrackCollection"
    acc.merge(DeviceTrkParamEstimationAlgCfg(flags,
        name=f"{extension}DeviceTrkParamEstimationAlg",
        InputTracccSpacepoints=stripSpacepoints,
        InputTracccMeasurements=measurements,
        InputTracccSeeds=seeds,
        InputTracccMagField="TracccMagneticField",
        OutputTracccTrackParameters=trackParameters,
    ))

    acc.merge(DeviceLargeRadiusTrackFindingAlgCfg(flags,
        name=f"{extension}DeviceTrackFindingAlg",
        InputTracccMeasurements=measurements,
        InputTracccMagField="TracccMagneticField",
        InputTracccTrackParameters=trackParameters,
        InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
        OutputTracccTracks=tracccTracks,
    ))

    # The measurement to cluster maps hold the index of the cluster in its
    # owning container, so the tracks are resolved against the primary pass
    # cluster containers rather than the views of this pass
    from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccTrackConverterAlgCfg
    acc.merge(TracccTrackConverterAlgCfg(flags,
        name=f"{extension}TracccTrackConverterAlg",
        InputPixelClusters="ITkPixelClusters",
        InputStripClusters="ITkStripClusters",
        InputMeasToPixelSP=measToPixelCluster,
        InputMeasToStripCl=measToStripCluster,
        InputTracks=tracccTracks,
        OutputTracks=f"{extension}Tracks",
        GeoIdMapping="TracccGeometryIdMapping",
        HostDetectorName="TracccHostDetectorGeometry",
    ))

    # Ambiguity Resolution
    if flags.Acts.doAmbiguityResolution:
        from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
        acc.merge(ActsAmbiguityResolutionCfg(flags))

    # PRD association
    from ActsConfig.ActsPrdAssociationConfig import ActsPrdAssociationAlgCfg
    acc.merge(ActsPrdAssociationAlgCfg(flags,
                                       name = f'{extension}PrdAssociationAlg',
                                       previousActsExtension = previousExtension))

    # Truth
    if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
        if flags.Tracking.ActiveConfig.storeSiSPSeededTracks or not flags.Acts.doAmbiguityResolution:
            acts_tracks = f"{extension}Tracks"
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))
            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))

        if flags.Acts.doAmbiguityResolution:
            acts_tracks = f"{extension}ResolvedTracks"
            acc.merge(ActsTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))
            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))

    return acc
