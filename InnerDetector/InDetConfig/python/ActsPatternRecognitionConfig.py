# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ActsTrackReconstructionCfg(flags,
                                  *,
                                  previousExtension: str = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Vertex reconstruction using spacepoints
    if flags.Tracking.ActiveConfig.useHoughVertexFilter:
        from InDetPriVxFinder.HoughVtxFinderConfig import HoughVtxFinderCfg
        acc.merge(HoughVtxFinderCfg(flags,
                                    inputPixelSpacePoints = "PixelSpacePoints"))

    # Seeding
    from ActsConfig.ActsSeedingInDetConfig import ActsInDetSeedingCfg
    acc.merge(ActsInDetSeedingCfg(flags))

    # CKF
    from ActsConfig.ActsTrackFindingInDetConfig import ActsInDetTrackFindingCfg
    acc.merge(ActsInDetTrackFindingCfg(flags))
    
    # Ambiguity Resolution 
    # !!! NOT VALIDAtED FOR INNER DETECTOR ACTS TRACKING !!!
    if flags.Acts.doAmbiguityResolution:
        from ActsConfig.ActsTrackFindingInDetConfig import ActsInDetAmbiguityResolutionCfg
        acc.merge(ActsInDetAmbiguityResolutionCfg(flags))

    # PRD association
    from ActsConfig.ActsPrdAssociationConfig import ActsPrdAssociationAlgCfg
    acc.merge(ActsPrdAssociationAlgCfg(flags,
                                       name = f'{flags.Tracking.ActiveConfig.extension}PrdAssociationAlg',
                                       InputTrackCollection = f'{flags.Tracking.ActiveConfig.extension}Tracks',
                                       previousActsExtension = previousExtension))

    # Truth
    if flags.Tracking.doTruth:
        # Run truth on CKF tracks
        # This is only necessary if we are asking for these tracks to be persistified with the
        # - flag: Tracking.ActiveConfig.storeSiSPSeededTracks set to True OR
        # - flag: flags.Acts.doAmbiguityResolution set to False
        if flags.Tracking.ActiveConfig.storeSiSPSeededTracks or not flags.Acts.doAmbiguityResolution:
            from ActsConfig.ActsTruthConfig import ActsInDetTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acts_tracks = f"{flags.Tracking.ActiveConfig.extension}Tracks"
            acc.merge(ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))
            
            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))            

        # Run truth on the tracks from ambiguity resolution. This is only necessary if
        # - flag: flags.Acts.doAmbiguityResolution set to True
        if flags.Acts.doAmbiguityResolution:
            # !!! NOT VALIDATED YET FOR INNER DETECTOR ACTS TRACKING !!!
            acts_tracks = f"{flags.Tracking.ActiveConfig.extension}ResolvedTracks"
            from ActsConfig.ActsTruthConfig import ActsInDetTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
            acc.merge(ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                                        name = f"{acts_tracks}TrackToTruthAssociationAlg",
                                                        ACTSTracksLocation = acts_tracks,
                                                        AssociationMapOut = f"{acts_tracks}ToTruthParticleAssociation"))
            
            acc.merge(ActsTrackFindingValidationAlgCfg(flags,
                                                       name = f"{acts_tracks}TrackFindingValidationAlg",
                                                       TrackToTruthAssociationMap = f"{acts_tracks}ToTruthParticleAssociation"))

    return acc

