# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ITkActsTrackReconstructionCfg(flags,
                                  *,
                                  previousExtension: str = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Seeding
    from ActsConfig.ActsSeedingConfig import ActsSeedingCfg
    acc.merge(ActsSeedingCfg(flags))

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




    # Particle creation and persistification
    from InDetConfig.ITkActsParticleCreationConfig import ITkActsTrackParticleCreationCfg
    # Tracks from CKF and SiSPSeededTracks{extension}TrackParticles
    if flags.Tracking.ActiveConfig.storeSiSPSeededTracks:
        # Naming convention for track particles: SiSPSeededTracks{extension}TrackParticles
        acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                  TrackContainers = [f"{flags.Tracking.ActiveConfig.extension}Tracks"],
                                                  TrackParticleContainer = f'SiSPSeededTracks{flags.Tracking.ActiveConfig.extension}TrackParticles'))

    # Track from CKF or ambiguity resolution if we want separate containers
    # In case no ambiguity resolution is scheduled, the CKF tracks will be used instead of those from the ambiguity resolution
    if flags.Tracking.ActiveConfig.storeSeparateContainer:
        # If we do not want the track collection to be merged with another collection
        # then we immediately create the track particles from it
        # Naming convention for track particles: InDet{extension}TrackParticles
        acts_tracks = f"{flags.Tracking.ActiveConfig.extension}Tracks" if not flags.Acts.doAmbiguityResolution else f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
        acc.merge(ITkActsTrackParticleCreationCfg(flags,
                                                  TrackContainers = [acts_tracks],
                                                  TrackParticleContainer = f'InDet{flags.Tracking.ActiveConfig.extension}TrackParticles'))
        
            
    return acc

