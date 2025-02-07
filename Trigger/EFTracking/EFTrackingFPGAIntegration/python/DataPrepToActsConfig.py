# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# @author: Zhaoyuan.Cui@cern.ch
# @date: Nov. 21, 2024
# @brief: A customized configuration for using ACTS spacepoint formation algorithm

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from ActsConfig.ActsUtilities import extractChildKwargs

pixelContainer = "FPGAPixelClusters"
stripContainer = "FPGAStripClusters"

def UseActsSpacePointFormationCfg(flags, **kwargs) -> ComponentAccumulator:
    
    acc = ComponentAccumulator()
    
    kwargs.setdefault('processOverlapSpacePoints', True)
    
    # Pixel spacepoint
    kwargs.setdefault('PixelSpacePointFormationAlg.name', 'FPGAActsPixelSpacePointFormationAlg')
    kwargs.setdefault('PixelSpacePointFormationAlg.PixelClusters', pixelContainer)
    kwargs.setdefault('PixelSpacePointFormationAlg.PixelSpacePoints', 'FPGAPixelSpacePoints')
    kwargs.setdefault('PixelSpacePointFormationAlg.useCache', flags.Acts.useCache)
    kwargs.setdefault('PixelSpacePointFormationAlg.SPCache',  'FPGAPixelSpacePointCache')
    
    # Strip spacepoint
    kwargs.setdefault('StripSpacePointFormationAlg.name', 'FPGAActsStripSpacePointFormationAlg')
    kwargs.setdefault('StripSpacePointFormationAlg.StripClusters', stripContainer)
    kwargs.setdefault('StripSpacePointFormationAlg.StripSpacePoints', 'FPGAStripSpacePoints')
    kwargs.setdefault('StripSpacePointFormationAlg.useCache', flags.Acts.useCache)
    kwargs.setdefault('StripSpacePointFormationAlg.SPCache', 'FPGASpacePointCache')
    
    # Handling of Overlap Space Points
    kwargs.setdefault('StripSpacePointFormationAlg.ProcessOverlapForStrip', kwargs['processOverlapSpacePoints'])
    kwargs.setdefault('StripSpacePointFormationAlg.OSPCache', 'FPGAStripOverlapSpacePointCache')
    if kwargs['processOverlapSpacePoints']:
        kwargs.setdefault('StripSpacePointFormationAlg.StripOverlapSpacePoints', 'FPGAStripOverlapSpacePoints')
    else:
        # Disable keys
        kwargs.setdefault('StripSpacePointFormationAlg.StripOverlapSpacePoints', '')
        
    from ActsConfig.ActsSpacePointFormationConfig import ActsPixelSpacePointFormationAlgCfg, ActsStripSpacePointFormationAlgCfg
    acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,**extractChildKwargs(prefix='PixelSpacePointFormationAlg.', **kwargs)))
            
    acc.merge(ActsStripSpacePointFormationAlgCfg(flags, **extractChildKwargs(prefix='StripSpacePointFormationAlg.', **kwargs)))
    
    return acc


def DataPrepToActsCfg(flags, **kwargs) -> ComponentAccumulator:
    
    acc = ComponentAccumulator()
    
    # If the pass-through kernel is cluster-only, we need to do ACTS spacepoint formation
    if flags.FPGADataPrep.PassThrough.ClusterOnly:
        acc.merge(UseActsSpacePointFormationCfg(flags, **kwargs))
    
    # ACTS Seeding
    from ActsConfig.ActsSeedingConfig import ActsStripSeedingAlgCfg, ActsPixelSeedingAlgCfg
    
    # Pixel seeding
    kwargs.setdefault('ActsPixelSeedingAlgCfg.name', 'FPGAActsPixelSeedingAlg')
    kwargs.setdefault('ActsPixelSeedingAlgCfg.InputSpacePoints', ['FPGAPixelSpacePoints'])
    kwargs.setdefault('ActsPixelSeedingAlgCfg.OutputSeeds', 'FPGAPixelSeeds')
    acc.merge(ActsPixelSeedingAlgCfg(flags, **extractChildKwargs(prefix='ActsPixelSeedingAlgCfg.', **kwargs)))
    
    # Strip seeding
    kwargs.setdefault('ActsStripSeedingAlgCfg.name', 'FPGAActsStripSeedingAlg')
    kwargs.setdefault('ActsStripSeedingAlgCfg.InputSpacePoints', ['FPGAStripSpacePoints'])
    kwargs.setdefault('ActsStripSeedingAlgCfg.OutputSeeds', 'FPGAStripSeeds')
    acc.merge(ActsStripSeedingAlgCfg(flags, **extractChildKwargs(prefix='ActsStripSeedingAlgCfg.', **kwargs)))
    
    # ACTS Tracking
    from ActsConfig.ActsTrackFindingConfig import ActsMainTrackFindingAlgCfg
    kwargs.setdefault('ActsTrackFinding.name', 'FPGAActsTrackFindingAlg')
    kwargs.setdefault('ActsTrackFinding.SeedContainerKeys', ['FPGAPixelSeeds','FPGAStripSeeds'])
    kwargs.setdefault('ActsTrackFinding.UncalibratedMeasurementContainerKeys', [pixelContainer,stripContainer])
    kwargs.setdefault('ActsTrackFinding.ACTSTracksLocation', 'FPGAActsTracks')
    acc.merge(ActsMainTrackFindingAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackFinding.', **kwargs)))

    # Track to Truth association and validation
    from ActsConfig.ActsTruthConfig import ActsTruthParticleHitCountAlgCfg, ActsPixelClusterToTruthAssociationAlgCfg, ActsStripClusterToTruthAssociationAlgCfg
    kwargs.setdefault('ActsPixelClusterToTruthAssociation.name', 'FPGAActsPixelClusterToTruthAssociationAlg')
    kwargs.setdefault('ActsPixelClusterToTruthAssociation.InputTruthParticleLinks', "xAODTruthLinks")
    kwargs.setdefault('ActsPixelClusterToTruthAssociation.AssociationMapOut', "ITkFPGAPixelClustersToTruthParticles")
    kwargs.setdefault('ActsPixelClusterToTruthAssociation.Measurements', pixelContainer)
    acc.merge(ActsPixelClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix='ActsPixelClusterToTruthAssociation.', **kwargs)))

    kwargs.setdefault('ActsStripClusterToTruthAssociation.name', 'FPGAActsStripClusterToTruthAssociation')
    kwargs.setdefault('ActsStripClusterToTruthAssociation.InputTruthParticleLinks', "xAODTruthLinks")
    kwargs.setdefault('ActsStripClusterToTruthAssociation.AssociationMapOut', "ITkFPGAStripClustersToTruthParticles")
    kwargs.setdefault('ActsStripClusterToTruthAssociation.Measurements', stripContainer)
    acc.merge(ActsStripClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix='ActsStripClusterToTruthAssociation.', **kwargs)))

    kwargs.setdefault('ActsTruthParticleHitCount.name', 'FPGAActsTruthParticleHitCount')
    kwargs.setdefault('ActsTruthParticleHitCount.PixelClustersToTruthAssociationMap', "ITkFPGAPixelClustersToTruthParticles")
    kwargs.setdefault('ActsTruthParticleHitCount.StripClustersToTruthAssociationMap', "ITkFPGAStripClustersToTruthParticles")
    kwargs.setdefault('ActsTruthParticleHitCount.TruthParticleHitCountsOut', "FPGATruthParticleHitCounts")
    acc.merge(ActsTruthParticleHitCountAlgCfg(flags, **extractChildKwargs(prefix='ActsTruthParticleHitCount.', **kwargs)))

    from ActsConfig.ActsTruthConfig import ActsTrackToTruthAssociationAlgCfg, ActsTrackFindingValidationAlgCfg
    kwargs.setdefault('ActsTrackToTruthAssociation.name', 'FPGAActsTrackToTruthAssociation')
    kwargs.setdefault('ActsTrackToTruthAssociation.PixelClustersToTruthAssociationMap', "ITkFPGAPixelClustersToTruthParticles")
    kwargs.setdefault('ActsTrackToTruthAssociation.StripClustersToTruthAssociationMap', "ITkFPGAStripClustersToTruthParticles")
    kwargs.setdefault('ActsTrackToTruthAssociation.ACTSTracksLocation', "FPGAActsTracks")
    kwargs.setdefault('ActsTrackToTruthAssociation.AssociationMapOut', "FPGAActsTracksFPGAToTruthParticleAssociation")
    acc.merge(ActsTrackToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackToTruthAssociation.', **kwargs)))

    kwargs.setdefault('ActsTrackFindingValidation.name', 'FPGAActsTrackFindingValidation')
    kwargs.setdefault('ActsTrackFindingValidation.TrackToTruthAssociationMap', "FPGAActsTracksFPGAToTruthParticleAssociation")
    kwargs.setdefault('ActsTrackFindingValidation.TruthParticleHitCounts', "FPGATruthParticleHitCounts")
    acc.merge(ActsTrackFindingValidationAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackFindingValidation.', **kwargs)))

    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    kwargs.setdefault('ActsTrackToTrackParticleCnv.name', 'ActsTrackToTrackParticleCnvAlg')
    kwargs.setdefault('ActsTrackToTrackParticleCnv.ACTSTracksLocation', ['FPGAActsTracks'])
    kwargs.setdefault('ActsTrackToTrackParticleCnv.TrackParticlesOutKey', 'FPGATrackParticles')
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackToTrackParticleCnv.', **kwargs)))
   

    from ActsConfig.ActsTruthConfig import ActsTrackParticleTruthDecorationAlgCfg
    kwargs.setdefault('ActsTrackParticleTruthDecoration.name', 'FPGAActsTrackParticleTruthDecoration')
    kwargs.setdefault('ActsTrackParticleTruthDecoration.TrackToTruthAssociationMaps', ["FPGAActsTracksFPGAToTruthParticleAssociation"])
    kwargs.setdefault('ActsTrackParticleTruthDecoration.TrackParticleContainerName', 'FPGATrackParticles')
    kwargs.setdefault('ActsTrackParticleTruthDecoration.TruthParticleHitCounts', 'FPGATruthParticleHitCounts')
    kwargs.setdefault('ActsTrackParticleTruthDecoration.ComputeTrackRecoEfficiency', True)
    acc.merge(ActsTrackParticleTruthDecorationAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackParticleTruthDecoration.', **kwargs)))
    
    return acc