# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from ActsConfig.ActsUtilities import extractChildKwargs

def EFPassThroughClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
                      
    kwargs = dict()
    kwargs.setdefault('processPixels', flags.Detector.EnableITkPixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableITkStrip)

    kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
    kwargs.setdefault('runReconstruction', True)
    kwargs.setdefault('runPreparation', flags.Acts.useCache)

    # Name of the RoI to be used
    roisName = f'{flags.Tracking.ActiveConfig.extension}RegionOfInterest'
        
    # Custom names for the cluster collections before passthough     
    pixelClustersName = 'FPGAITkPixelClusters'
    stripClustersName = 'FPGAITkStripClusters'

    if kwargs['processPixels']:
        kwargs.setdefault('PixelClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelClusterizationAlg')
        kwargs.setdefault('PixelClusterizationAlg.useCache', flags.Acts.useCache)
        kwargs.setdefault('PixelClusterizationAlg.ClustersKey', pixelClustersName)
        kwargs.setdefault('PixelClusterizationAlg.ClusterCache', f'{flags.Tracking.ActiveConfig.extension}PixelClustersCache')
        from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg
        acc.merge(ActsPixelClusterizationAlgCfg(flags,
                                                RoIs=roisName,
                                                **extractChildKwargs(prefix='PixelClusterizationAlg.', **kwargs)))
    if kwargs['processStrips']:
        kwargs.setdefault('StripClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripClusterizationAlg')
        kwargs.setdefault('StripClusterizationAlg.useCache', flags.Acts.useCache)
        kwargs.setdefault('StripClusterizationAlg.ClustersKey', stripClustersName)
        kwargs.setdefault('StripClusterizationAlg.ClusterCache', f'{flags.Tracking.ActiveConfig.extension}StripClustersCache')
        from ActsConfig.ActsClusterizationConfig import ActsStripClusterizationAlgCfg
        acc.merge(ActsStripClusterizationAlgCfg(flags,
                                                RoIs=roisName,
                                                **extractChildKwargs(prefix='StripClusterizationAlg.', **kwargs)))
    #
    # --- FPGA Pass Through
    #
    # --- from Cluster xAOD -> FPGA -> xAOD   
    #
    from EFTrackingFPGAPipeline.DataPrepConfig import (
        DataPrepCfg)
    acc.merge(DataPrepCfg(flags))
    
    return acc 