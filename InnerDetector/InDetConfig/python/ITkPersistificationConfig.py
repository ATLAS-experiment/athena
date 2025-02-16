# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from InDetConfig.ITkActsHelpers import isPrimaryPass


def ITkTrackSeedsFinalCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Do not persistify anything if this is not a primary
    # pass or if the appropriate flag has not been set to True
    if not isPrimaryPass(flags) and not flags.Tracking.ActiveConfig.storeTrackSeeds:
        return acc

    extension = flags.Tracking.ActiveConfig.extension
    TrackContainer = f"SiSPSeedSegments{extension}"

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=f"{TrackContainer}DetailedTruth",
            TracksTruth=f"{TrackContainer}TruthCollection"))
        
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    acc.merge(ITkTrackParticleCnvAlgCfg(
        flags,
        name=f"{TrackContainer}CnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=(
            f"{TrackContainer}TrackParticles")))
    
    return acc


def ITkSiSPSeededTracksFinalCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Do not persistify anything if this is not a primary
    # pass or if the appropriate flag has not been set to True
    if not isPrimaryPass(flags) and not flags.Tracking.ActiveConfig.storeSiSPSeededTracks:
        return acc

    extension =	flags.Tracking.ActiveConfig.extension
    AssociationMapNameKey="PRDtoTrackMapMerge_CombinedITkTracks"
    if 'Acts' in extension:
        AssociationMapNameKey="PRDtoTrackMapMerge_CombinedITkTracks"
    elif not isPrimaryPass(flags):
        AssociationMapNameKey = f"ITkPRDtoTrackMap{extension}"

    # The following is to accomodate an hack we have in one of our ACTS ART tests
    # it will be removed
    if extension == "" and not isPrimaryPass(flags):
        AssociationMapNameKey="PRDtoTrackMapMerge_CombinedITkTracks"
        
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    acc.merge(ITkTrackParticleCnvAlgCfg(
        flags,
        name = f"SiSPSeededTracks{extension}CnvAlg",
        TrackContainerName = f"SiSPSeeded{extension}Tracks",
        xAODTrackParticlesFromTracksContainerName=(
            f"SiSPSeededTracks{extension}TrackParticles"),
        AssociationMapName=
        "" if flags.Tracking.doITkFastTracking else 
        AssociationMapNameKey)
              )
    
    return acc
