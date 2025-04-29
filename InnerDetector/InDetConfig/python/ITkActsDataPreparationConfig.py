# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ITkActsDataPreparationCfg(flags,
                              *,
                              previousExtension: str = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Region of interest creation
    from ActsConfig.ActsRegionsOfInterestConfig import ActsRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsRegionsOfInterestCreatorAlgCfg(flags,
                                                 name = f"{flags.Tracking.ActiveConfig.extension}RegionsOfInterestCreatorAlg"))
    
    # Cluster formation
    # This includes Pixel, Strip and HGTD clusters
    from ActsConfig.ActsClusterizationConfig import ActsClusterizationCfg
    acc.merge(ActsClusterizationCfg(flags,
                                    previousActsExtension = previousExtension))

    # Space Point Formation
    from ActsConfig.ActsSpacePointFormationConfig import ActsSpacePointFormationCfg
    acc.merge(ActsSpacePointFormationCfg(flags,
                                         previousActsExtension = previousExtension))
    
    # Truth
    # this truth must only be done if you do PRD and SpacePointformation
    # If you only do the latter (== running on ESD) then the needed input (simdata)
    # is not in ESD but the resulting truth (clustertruth) is already there ...
    if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
        acc.merge(ActsTruthAssociationAlgCfg(flags))
        acc.merge(ActsTruthParticleHitCountAlgCfg(flags))
        
    return acc

