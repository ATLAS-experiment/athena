# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ActsDataPreparationCfg(flags,
                              *,
                              previousExtension: str = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Region of interest creation
    from ActsConfig.ActsRegionsOfInterestConfig import ActsInDetRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsInDetRegionsOfInterestCreatorAlgCfg(flags,
                                                 name = f"{flags.Tracking.ActiveConfig.extension}RegionsOfInterestCreatorAlg",
                                                 RoIs = f"{flags.Tracking.ActiveConfig.extension}RegionOfInterest"))
    
    # Cluster formation
    # This includes Pixel, SCT clusters
    from ActsConfig.ActsClusterizationInDetConfig import ActsIDClusterizationCfg
    acc.merge(ActsIDClusterizationCfg(flags,
                                    previousActsExtension = previousExtension))

    # Space Point Formation
    from ActsConfig.ActsSpacePointFormationInDetConfig import ActsIDSpacePointFormationCfg
    acc.merge(ActsIDSpacePointFormationCfg(flags,
                                         previousActsExtension = previousExtension))
    
    # Truth
    # this truth must only be done if you do PRD and SpacePointformation
    # If you only do the latter (== running on ESD) then the needed input (simdata)
    # is not in ESD but the resulting truth (clustertruth) is already there ...
    if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsInDetTruthParticleHitCountAlgCfg
        acc.merge(ActsTruthAssociationAlgCfg(flags))
        acc.merge(ActsInDetTruthParticleHitCountAlgCfg(flags))

    return acc

