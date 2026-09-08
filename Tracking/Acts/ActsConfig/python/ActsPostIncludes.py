# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PersistifyActsEDMCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsPersistificationConfig import PersistifyClusters
    acc.merge(PersistifyClusters(flags,
                                 pixelClusterCollections=['ITkPixelClusters'],
                                 stripClusterCollections=['ITkStripClusters'],
                                 hgtdClusterCollections=None if not flags.Reco.EnableHGTDExtension else ['HGTD_Clusters']))

    from ActsConfig.ActsPersistificationConfig import PersistifySpacePoints
    acc.merge(PersistifySpacePoints(flags,
                                    pixelSpacePointCollections=['ITkPixelSpacePoints'],
                                    stripSpacePointCollections=['ITkStripSpacePoints', 'ITkStripOverlapSpacePoints']))

    trackPrefixes = ['Acts', 'ActsResolved',
                     'ActsLargeRadius', 'ActsLargeRadiusResolved',
                     'ActsConversion', 'ActsConversionResolved',
                     'ActsHeavyIon', 'ActsHeavyIonResolved']
    from ActsConfig.ActsPersistificationConfig import PersistifyTracks
    acc.merge(PersistifyTracks(flags,
                               extensions=trackPrefixes))

    from ActsConfig.ActsPersistificationConfig import PersistifyTrackParticles
    acc.merge(PersistifyTrackParticles(flags,
                                       trackParticleCollections=['InDetActsTrackParticles']))
    return acc

def ACTSClusterPostInclude(flags) -> ComponentAccumulator:
    # Schedule ACTS Data Preparation and Measurement persistification
    # This is used for technical efficiencies studies of tracking pipelines
    if flags.Tracking.PRDInfo.KeepOnlyOnTrackMeasurements:
        raise ValueError("The ACTSClusterPostInclude is to be used for technical efficiency computation, however the " \
                         f"config flag 'Tracking.PRDInfo.KeepOnlyOnTrackMeasurements' is set to {flags.Tracking.PRDInfo.KeepOnlyOnTrackMeasurements}, " \
                         "which is incompatible with this purpose")
    
    acc = ComponentAccumulator()
    
    # Schedule Full Scan RoI
    from ActsConfig.ActsRegionsOfInterestConfig import ActsMainRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsMainRegionsOfInterestCreatorAlgCfg(flags,
                                                     name = "ActsOfflineRegionsOfInterestCreatorAlg",
                                                     RoIs = "ActsOfflineRegionOfInterest"))
    
    # Cluster formation
    # This includes Pixel and Strip
    clusteringKwargs = dict()
    clusteringKwargs["PixelClusterizationAlg.name"] = "ActsOfflinePixelClusterizationAlg"
    clusteringKwargs["PixelClusterizationAlg.ClustersKey"] = "ITkOfflinePixelClusters"
    clusteringKwargs["StripClusterizationAlg.name"] = "ActsOfflineStripClusterizationAlg"
    clusteringKwargs["StripClusterizationAlg.ClustersKey"] = "ITkOfflineStripClusters"
    from ActsConfig.ActsClusterizationConfig import ActsMainClusterizationCfg
    acc.merge(ActsMainClusterizationCfg(flags,
                                        RoIs = "ActsOfflineRegionOfInterest",
                                        processHGTD = False,
                                        **clusteringKwargs))

    # Attach truth to clusters
    if flags.Tracking.doTruth:
        truthAssociationKwargs = dict()
        truthAssociationKwargs["PixelClusterToTruthAssociationAlg.name"] = "ActsOfflinePixelClusterToTruthAssociationAlg"
        truthAssociationKwargs["PixelClusterToTruthAssociationAlg.Measurements"] = "ITkOfflinePixelClusters"
        truthAssociationKwargs["PixelClusterToTruthAssociationAlg.AssociationMapOut"] = "ITkOfflinePixelClustersToTruthParticles"
        truthAssociationKwargs["StripClusterToTruthAssociationAlg.name"] = "ActsOfflineStripClusterToTruthAssociationAlg"
        truthAssociationKwargs["StripClusterToTruthAssociationAlg.Measurements"] = "ITkOfflineStripClusters"
        truthAssociationKwargs["StripClusterToTruthAssociationAlg.AssociationMapOut"] = "ITkOfflineStripClustersToTruthParticles"

        from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
        acc.merge(ActsTruthAssociationAlgCfg(flags,
                                             **truthAssociationKwargs))
        acc.merge(ActsTruthParticleHitCountAlgCfg(flags,
                                                  name = "ActsOfflineTruthParticleHitCountAlg",
                                                  PixelClustersToTruthAssociationMap = "ITkOfflinePixelClustersToTruthParticles",
                                                  StripClustersToTruthAssociationMap = "ITkOfflineStripClustersToTruthParticles",
                                                  TruthParticleHitCountsOut = "OfflineTruthParticleHitCounts"))
        
        from InDetConfig.InDetPrepRawDataToxAODConfig import TruthParticleIndexDecoratorAlgCfg
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

        from ActsConfig.ActsObjectDecorationConfig import ActsPixelClusterTruthDecoratorAlgCfg,ActsStripClusterTruthDecoratorAlgCfg
        acc.merge(ActsPixelClusterTruthDecoratorAlgCfg(flags,
                                                       name = "ActsOfflinePixelClusterTruthDecoratorAlgCfg",
                                                       ClusterContainer = "ITkOfflinePixelClusters",
                                                       AssociationMapOut = "ITkOfflinePixelClustersToTruthParticles",
                                                       MeasurementContainer = "ITkPixelMeasurements_offl"))
        acc.merge(ActsStripClusterTruthDecoratorAlgCfg(flags,
                                                       name = "ActsOfflineStripClusterTruthDecoratorAlgCfg",
                                                       ClusterContainer = "ITkOfflineStripClusters",
                                                       AssociationMapOut = "ITkOfflineStripClustersToTruthParticles",
                                                       MeasurementContainer = "ITkStripMeasurements_offl"))

        ## write out measurements containers in any case
        toAOD = [
            'xAOD::TrackMeasurementValidationContainer#ITkPixelMeasurements_offl',
            'xAOD::TrackMeasurementValidationAuxContainer#ITkPixelMeasurements_offlAux.',
            'xAOD::TrackMeasurementValidationContainer#ITkStripMeasurements_offl',
            'xAOD::TrackMeasurementValidationAuxContainer#ITkStripMeasurements_offlAux.'
        ]
        
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge( addToAOD( flags, toAOD ) )

    return acc
