# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ITkActsTrackParticleCreationCfg(flags,
                                    *,
                                    TrackContainers: list[str],
                                    TrackParticleContainer: str,
                                    persistifyCollection: bool = True,
                                    PerigeeExpression: str = None) -> ComponentAccumulator:
    # This function does the following:
    # - Creates track particles from a collection of track containers
    # - Attaches truth decoration to the track particles
    # - Persistifies the track particles
    # The function will be called within the loop on the tracking passes, if the uses asks
    # for collections to be stored in different containers, as well as just before the primary
    # vertex reconstruction, on the ensamble of track collections requested during reconstruction
    # (but always on the primary pass tracks)    

    assert isinstance(TrackContainers, list)
    for container in TrackContainers:
        assert isinstance(container, str)

    # Set the perigee expression to be used for track particle creation
    # This is used by Heavy Ion configuration
    if PerigeeExpression is None:
        PerigeeExpression = flags.Tracking.perigeeExpression

    print("Storing track and track particle containers:")
    print(f"- track collection(s): {TrackContainers}")
    print(f"- track particle collection: {TrackParticleContainer}")
        
    acc = ComponentAccumulator()

    prefix = "ActsCombined" if "ActiveConfig" not in flags.Tracking else flags.Tracking.ActiveConfig.extension
    prefix += f"To{TrackParticleContainer}"
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags,
                                                name = f"{prefix}TrackToTrackParticleCnvAlg",
                                                ACTSTracksLocation = TrackContainers,
                                                TrackParticlesOutKey = TrackParticleContainer,
                                                PerigeeExpression = PerigeeExpression))
    
    if flags.Tracking.doTruth :
        from AthenaCommon.Constants import WARNING, INFO
        track_to_truth_maps = []
        from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
        for trackContainer in TrackContainers:
            track_to_truth_maps.append(f"{trackContainer}ToTruthParticleAssociation")
            acc.merge(ActsTrackParticleTruthDecorationAlgCfg(flags,
                                                             name = f'{prefix}TruthDecorationAlg',
                                                             TrackToTruthAssociationMaps = track_to_truth_maps,
                                                             TrackParticleContainerName = TrackParticleContainer,
                                                             OutputLevel = WARNING              if len(TrackContainers)==1 else INFO,
                                                             ComputeTrackRecoEfficiency = False if len(TrackContainers)==1 else True))

    # Persistification
    # By default this is always happening, but in the case the perigee strategy is set
    # to Vertex we need to create a temporary track particle collection wrt the BeamLine
    # which does not need to be persistified
    if persistifyCollection:
        toAOD = []
        trackparticles_shortlist = [] if flags.Acts.EDM.PersistifyTracks else ['-actsTrack']
        trackparticles_variables = ".".join(trackparticles_shortlist)
        toAOD += [f"xAOD::TrackParticleContainer#{TrackParticleContainer}",
                  f"xAOD::TrackParticleAuxContainer#{TrackParticleContainer}Aux." + trackparticles_variables]
        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
        acc.merge(addToAOD(flags, toAOD))
    
    return acc


def ITkActsTrackParticlePersistificationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    # Particle creation and persistification
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
