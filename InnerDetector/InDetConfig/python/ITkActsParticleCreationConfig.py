# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ITkActsTrackParticleCreationCfg(flags,
                                    *,
                                    TrackContainers: list[str],
                                    TrackParticleContainer: str) -> ComponentAccumulator:
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
                                                TrackParticlesOutKey = TrackParticleContainer))
    
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
    toAOD = []
    trackparticles_shortlist = [] if flags.Acts.EDM.PersistifyTracks else ['-actsTrack']
    trackparticles_variables = ".".join(trackparticles_shortlist)
    toAOD += [f"xAOD::TrackParticleContainer#{TrackParticleContainer}",
              f"xAOD::TrackParticleAuxContainer#{TrackParticleContainer}Aux." + trackparticles_variables]
        
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    
    return acc
