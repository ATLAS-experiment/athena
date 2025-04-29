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

    return acc

