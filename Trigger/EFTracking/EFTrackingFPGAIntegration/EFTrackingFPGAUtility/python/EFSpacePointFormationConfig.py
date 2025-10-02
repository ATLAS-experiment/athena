# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def EFSpacePointFormationCfg(flags, previousActsExtension = None, **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    processPixels = flags.Detector.EnableITkPixel
    processStrips = flags.Detector.EnableITkStrip
        
    kwargs = dict()
    kwargs.setdefault('processPixels', processPixels)
    kwargs.setdefault('processStrips', processStrips)

    # Similarly to Clusterization, space point formation is a three step process at maximum:
    #   (1) Cache Creation
    #   (2) Space Point formation algorithm (reconstruction of space points)
    #   (3) Preparation of collection for downstream algorithms
    # What step is scheduled depends on the tracking pass and the activation
    # or de-activation of caching mechanism.
    
    # Secondary passes do not need cache creation, that has to be performed
    # on the primary pass, and only if the caching is enabled.
    # Reconstruction can run on secondary passes only if the caching is enabled,
    # this is because we may need to process detector elements not processed
    # on the primary pass.
    # Preparation has to be performed on secondary passes always, and on primary
    # pass only if cache is enabled. In the latter case it is used to collect all
    # the clusters from all views before passing them to the downstream algorithms


    # Primary pass
    kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
    kwargs.setdefault('runReconstruction', True)
    kwargs.setdefault('runPreparation', flags.Acts.useCache)

    # Overlap Space Points may not be required
    processOverlapSpacePoints = processStrips

    kwargs.setdefault('processOverlapSpacePoints', processOverlapSpacePoints)
        
    # Name of the RoI to be used
    roisName = f'{flags.Tracking.ActiveConfig.extension}RegionOfInterest'
    
    # Cluster Collection name(s) and Space Point Collection name(s)
    # The name depends on the tracking pass as well as the cache mechanism
    pixelClustersName = 'ITkPixelClusters'
    stripClustersName = 'ITkStripClusters'
    pixelSpacePointsName = 'FPGAITkPixelSpacePoints'
    stripSpacePointsName = 'FPGAITkStripSpacePoints'
    stripOverlapSpacePointsName = 'ITkStripOverlapSpacePoints'
    # if cache is enabled, add "_Cached" at the end
    if flags.Acts.useCache:
        pixelClustersName += "_Cached"
        stripClustersName += "_Cached"

    # Primary collections for space points (i.e. produced by primary pass)
    primaryPixelSpacePointsName = 'FPGAITkPixelSpacePoints'
    primaryStripSpacePointsName = 'FPGAITkStripSpacePoints'
    primaryStripOverlapSpacePointsName = 'ITkStripOverlapSpacePoints'
        
    # Configuration for (1)
    if kwargs['runCacheCreation']:
        kwargs.setdefault('SpacePointCacheCreatorAlg.name', f'{flags.Tracking.ActiveConfig.extension}SpacePointCacheCreatorAlg')

    # Configuration for (2)
    if kwargs['runReconstruction']:
        if kwargs['processPixels']:
            kwargs.setdefault('PixelSpacePointFormationAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSpacePointFormationAlg')
            kwargs.setdefault('PixelSpacePointFormationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('PixelSpacePointFormationAlg.SPCache',  f'{flags.Tracking.ActiveConfig.extension}PixelSpacePointCache')
            kwargs.setdefault('PixelSpacePointFormationAlg.PixelClusters', pixelClustersName)
            kwargs.setdefault('PixelSpacePointFormationAlg.PixelSpacePoints', pixelSpacePointsName)

        if kwargs['processStrips']:
            kwargs.setdefault('StripSpacePointFormationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg')
            kwargs.setdefault('StripSpacePointFormationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripSpacePointFormationAlg.SPCache', f'{flags.Tracking.ActiveConfig.extension}StripSpacePointCache')
            kwargs.setdefault('StripSpacePointFormationAlg.StripClusters', stripClustersName)
            kwargs.setdefault('StripSpacePointFormationAlg.StripSpacePoints', stripSpacePointsName)

            # Handling of Overlap Space Points
            kwargs.setdefault('StripSpacePointFormationAlg.ProcessOverlapForStrip', kwargs['processOverlapSpacePoints'])
            kwargs.setdefault('StripSpacePointFormationAlg.OSPCache', f'{flags.Tracking.ActiveConfig.extension}StripOverlapSpacePointCache')
            if kwargs['processOverlapSpacePoints']:
                kwargs.setdefault('StripSpacePointFormationAlg.StripOverlapSpacePoints', stripOverlapSpacePointsName)
            else:
                # Disable keys
                kwargs.setdefault('StripSpacePointFormationAlg.StripOverlapSpacePoints', '')

    # Configuration for (3)
    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            kwargs.setdefault('PixelSpacePointPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSpacePointPreparationAlg')
            kwargs.setdefault('PixelSpacePointPreparationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('PixelSpacePointPreparationAlg.OutputCollection', f'{pixelSpacePointsName}_Cached' if kwargs['runReconstruction'] else pixelSpacePointsName)
            # The input is one between the collection (w/o cache) and the IDC (w/ cache)
            if not flags.Acts.useCache:
                # Take the collection from the reconstruction step. If not available take the collection from the primary pass
                kwargs.setdefault('PixelSpacePointPreparationAlg.InputCollection', pixelSpacePointsName if kwargs['runReconstruction'] else primaryPixelSpacePointsName)
                kwargs.setdefault('PixelSpacePointPreparationAlg.InputIDC', '')
            else:
                kwargs.setdefault('PixelSpacePointPreparationAlg.InputCollection', '')
                kwargs.setdefault('PixelSpacePointPreparationAlg.InputIDC', f'{flags.Tracking.ActiveConfig.extension}PixelSpacePointCache')

                
        if kwargs['processStrips']:
            kwargs.setdefault('StripSpacePointPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSpacePointPreparationAlg')
            kwargs.setdefault('StripSpacePointPreparationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripSpacePointPreparationAlg.OutputCollection', f'{stripSpacePointsName}_Cached' if kwargs['runReconstruction'] else stripSpacePointsName)
            # The input is one between the collection (w/o cache) and the IDC (w/ cache)
            if not flags.Acts.useCache:
                # Take the collection from the reconstruction step. If not available take the collection from the primary pass
                kwargs.setdefault('StripSpacePointPreparationAlg.InputCollection', stripSpacePointsName if kwargs['runReconstruction'] else primaryStripSpacePointsName)
                kwargs.setdefault('StripSpacePointPreparationAlg.InputIDC', '')
            else:
                kwargs.setdefault('StripSpacePointPreparationAlg.InputCollection', '')
                kwargs.setdefault('StripSpacePointPreparationAlg.InputIDC', f'{flags.Tracking.ActiveConfig.extension}StripSpacePointCache')

        if kwargs['processOverlapSpacePoints']:
            kwargs.setdefault('StripOverlapSpacePointPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripOverlapSpacePointPreparationAlg')
            kwargs.setdefault('StripOverlapSpacePointPreparationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripOverlapSpacePointPreparationAlg.OutputCollection',  f'{stripOverlapSpacePointsName}_Cached' if kwargs['runReconstruction'] else stripOverlapSpacePointsName)
            # The input is one between the collection (w/o cache) and the IDC (w/ cache)
            if not flags.Acts.useCache:
                # Take the collection from the reconstruction step. If not available take the collection from the primary pass
                kwargs.setdefault('StripOverlapSpacePointPreparationAlg.InputCollection', stripOverlapSpacePointsName if kwargs['runReconstruction'] else primaryStripOverlapSpacePointsName)
                kwargs.setdefault('StripOverlapSpacePointPreparationAlg.InputIDC', '')
            else:
                kwargs.setdefault('StripOverlapSpacePointPreparationAlg.InputCollection', '')
                kwargs.setdefault('StripOverlapSpacePointPreparationAlg.InputIDC', f'{flags.Tracking.ActiveConfig.extension}StripOverlapSpacePointCache')

    # Analysis algo(s)
    if flags.Acts.SpacePoints.doAnalysis:
        # Run analysis code on the resulting space point collection produced by this tracking pass        
        # This collection is the result of (3) if it ran, else the result of (2). We are sure at least one of them run
        if kwargs['processPixels']:
            kwargs.setdefault('PixelSpacePointAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelSpacePointAnalysisAlg')
            kwargs.setdefault('PixelSpacePointAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelSpacePointAnalysisAlg.SpacePointContainerKey', kwargs['PixelSpacePointPreparationAlg.OutputCollection'] if kwargs['runPreparation'] else kwargs['PixelSpacePointFormationAlg.PixelSpacePoints'])

        if kwargs['processStrips']:
            kwargs.setdefault('StripSpacePointAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripSpacePointAnalysisAlg')
            kwargs.setdefault('StripSpacePointAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripSpacePointAnalysisAlg.SpacePointContainerKey', kwargs['StripSpacePointPreparationAlg.OutputCollection'] if kwargs['runPreparation'] else kwargs['StripSpacePointFormationAlg.StripSpacePoints'])

        if kwargs['processOverlapSpacePoints']:
            kwargs.setdefault('StripOverlapSpacePointAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripOverlapSpacePointAnalysisAlg')
            kwargs.setdefault('StripOverlapSpacePointAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripOverlapSpacePointAnalysisAlg.SpacePointContainerKey', kwargs['StripOverlapSpacePointPreparationAlg.OutputCollection'] if kwargs['runPreparation'] else kwargs['StripSpacePointFormationAlg.StripOverlapSpacePoints'])

    from ActsConfig.ActsSpacePointFormationConfig import ActsMainSpacePointFormationCfg            
    acc.merge(ActsMainSpacePointFormationCfg(flags, RoIs=roisName, **kwargs))
    return acc

