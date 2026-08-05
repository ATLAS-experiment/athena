# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ITkActsGlobalDataPrepartionCfg(flags) -> ComponentAccumulator:
    # We schedule FS data preparation for all tracking passes downstream
    acc = ComponentAccumulator()

    # FS RoI
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags))

    # Clustering
    from ActsConfig.ActsClusterizationConfig import ActsMainClusterizationCfg
    kwargs_clusters = dict()
    #kwargs_clusters['PixelClusterizationAlg.name'] = "GlobalPixelClusterizationAlg"
    #kwargs_clusters['StripClusterizationAlg.name'] =  "GlobalStripClusterizationAlg"
    acc.merge(ActsMainClusterizationCfg(flags,
                                        runCacheCreation=False,
                                        runReconstruction=True,
                                        runPreparation=False,
                                        **kwargs_clusters))

    # Space Point Formation
    processStrips = flags.Detector.EnableITkStrip
    if processStrips:
        if flags.Tracking.doITkFastTracking:
            processStrips = False
            # activate if any secondary pass using strips is enabled
            if flags.Acts.doITkConversion or \
               flags.Acts.doLargeRadius:
                processStrips = True

    kwargs_space_points = dict()
    #kwargs_space_points['PixelSpacePointFormationAlg.name'] = "GlobalPixelSpacePointFormationAlg"
    #kwargs_space_points['StripSpacePointFormationAlg.name'] = "GlobalStripSpacePointFormationAlg"
    from ActsConfig.ActsSpacePointFormationConfig import ActsMainSpacePointFormationCfg
    acc.merge(ActsMainSpacePointFormationCfg(flags,
                                             processStrips=processStrips,
                                             runCacheCreation=False,
                                             runReconstruction=True,
                                             runPreparation=False,
                                             processOverlapSpacePoints=True,
                                             **kwargs_space_points))

    # Truth
    # this truth must only be done if you do PRD and SpacePointformation
    # If you only do the latter (== running on ESD) then the needed input (simdata)
    # is not in ESD but the resulting truth (clustertruth) is already there ...
    if flags.Tracking.doTruth:
        from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
        acc.merge(ActsTruthAssociationAlgCfg(flags))
        acc.merge(ActsTruthParticleHitCountAlgCfg(flags))
        if flags.Acts.doTruthInspection:
            from ActsConfig.ActsInspectTruthContentConfig import ActsInspectTruthContentAlgCfg
            acc.merge(ActsInspectTruthContentAlgCfg(flags))

    return acc

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

    # PLR is reconstructed as a sidecar cluster stream only. It is kept
    # separate from the standard ITk tracking inputs and is therefore only
    # scheduled once on the primary/validation pass.
    from InDetConfig.ITkActsHelpers import isPrimaryPass, isValidationPass
    if flags.Detector.EnablePLR and (isPrimaryPass(flags) or isValidationPass(flags)):
        from ActsConfig.ActsClusterizationConfig import ActsPLRClusterizationAlgCfg
        acc.merge(ActsPLRClusterizationAlgCfg(
            flags,
            name=f"{flags.Tracking.ActiveConfig.extension}PLRClusterizationAlg",
            RoIs=f"{flags.Tracking.ActiveConfig.extension}RegionOfInterest",
        ))

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
        if flags.Acts.doTruthInspection:
            from ActsConfig.ActsInspectTruthContentConfig import ActsInspectTruthContentAlgCfg
            acc.merge(ActsInspectTruthContentAlgCfg(flags))
        
    return acc
