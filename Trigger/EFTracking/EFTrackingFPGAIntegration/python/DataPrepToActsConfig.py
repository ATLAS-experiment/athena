# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# @author: Zhaoyuan.Cui@cern.ch
# @date: Nov. 21, 2024
# @brief: A customized configuration for using ACTS spacepoint formation algorithm

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from ActsConfig.ActsUtilities import extractChildKwargs

def UseActsSpacePointFormationCfg(flags, **kwargs) -> ComponentAccumulator:
    
    acc = ComponentAccumulator()
    
    kwargs.setdefault('processOverlapSpacePoints', True)
    
    # Pixel spacepoint
    kwargs.setdefault('PixelSpacePointFormationAlg.name', 'ActsPixelSpacePointFormationAlg')
    kwargs.setdefault('PixelSpacePointFormationAlg.PixelClusters', 'FPGAPixelClusters')
    kwargs.setdefault('PixelSpacePointFormationAlg.PixelSpacePoints', 'FPGAPixelSpacePoints')
    kwargs.setdefault('PixelSpacePointFormationAlg.useCache', flags.Acts.useCache)
    kwargs.setdefault('PixelSpacePointFormationAlg.SPCache',  'FPGAPixelSpacePointCache')
    
    # Strip spacepoint
    kwargs.setdefault('StripSpacePointFormationAlg.name', 'ActsStripSpacePointFormationAlg')
    kwargs.setdefault('StripSpacePointFormationAlg.StripClusters', 'FPGAStripClusters')
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
    kwargs.setdefault('ActsPixelSeedingAlgCfg.name', 'ActsPixelSeedingAlg')
    kwargs.setdefault('ActsPixelSeedingAlgCfg.InputSpacePoints', ['FPGAPixelSpacePoints'])
    kwargs.setdefault('ActsPixelSeedingAlgCfg.OutputSeeds', 'FPGAPixelSeeds')
    kwargs.setdefault('ActsPixelSeedingAlgCfg.OutputEstimatedTrackParameters', 'FPGAPixelEstimatedTrackParams')
    acc.merge(ActsPixelSeedingAlgCfg(flags, **extractChildKwargs(prefix='ActsPixelSeedingAlgCfg.', **kwargs)))
    
    # Strip seeding
    kwargs.setdefault('ActsStripSeedingAlgCfg.name', 'ActsStripSeedingAlg')
    kwargs.setdefault('ActsStripSeedingAlgCfg.InputSpacePoints', ['FPGAStripSpacePoints'])
    kwargs.setdefault('ActsStripSeedingAlgCfg.OutputSeeds', 'FPGAStripSeeds')
    kwargs.setdefault('ActsStripSeedingAlgCfg.OutputEstimatedTrackParameters', 'FPGAStripEstimatedTrackParams')
    acc.merge(ActsStripSeedingAlgCfg(flags, **extractChildKwargs(prefix='ActsStripSeedingAlgCfg.', **kwargs)))
    
    # ACTS Tracking
    from ActsConfig.ActsTrackFindingConfig import ActsMainTrackFindingAlgCfg
    kwargs.setdefault('ActsTrackFinding.name', 'ActsTrackFindingAlg')
    kwargs.setdefault('ActsTrackFinding.SeedContainerKeys', ['FPGAPixelSeeds','FPGAStripSeeds'])
    kwargs.setdefault('ActsTrackFinding.EstimatedTrackParametersKeys', ['FPGAPixelEstimatedTrackParams','FPGAStripEstimatedTrackParams'])
    kwargs.setdefault('ActsTrackFinding.UncalibratedMeasurementContainerKeys', ["FPGAPixelClusters","FPGAStripClusters"])
    kwargs.setdefault('ActsTrackFinding.ACTSTracksLocation', 'FPGAActsTracks')
    acc.merge(ActsMainTrackFindingAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackFinding.', **kwargs)))

    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    kwargs.setdefault('ActsTrackToTrackParticleCnv.name', 'ActsTrackToTrackParticleCnvAlg')
    kwargs.setdefault('ActsTrackToTrackParticleCnv.ACTSTracksLocation', ['FPGAActsTracks'])
    kwargs.setdefault('ActsTrackToTrackParticleCnv.TrackParticlesOutKey', 'FPGATrackParticles')
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags, **extractChildKwargs(prefix='ActsTrackToTrackParticleCnv.', **kwargs)))
   
    
    return acc