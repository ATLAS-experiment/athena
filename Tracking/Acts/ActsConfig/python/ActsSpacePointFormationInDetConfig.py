# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# The configurations of the Acts space point formation for the Inner Detector Pixel and SCT detectors.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsUtilities import extractChildKwargs

def ActsIDPixelSpacePointToolCfg(flags,
                               name: str = "ActsIDPixelSpacePointTool",
                               **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.PixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsIDStripSpacePointToolCfg(flags,
                               name: str = "ActsIDStripSpacePointTool",
                               **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    kwargs.setdefault("isITk", False)
    kwargs.setdefault("useSCTLayerDep_OverlapCuts", True) # not sure about it, but default in SCT workflow, is true
    # StripGapParameter
    kwargs.setdefault("StripGapParameter", 0.0)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.StripSpacePointFormationTool(name, **kwargs))
    return acc

# not validated yet
def ActsIDCoreStripSpacePointToolCfg(flags,
                                   name: str = "ActsIDCoreStripSpacePointTool",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # from ActsConfig.ActsGeometryConfig import ActsDetectorElementToActsGeometryIdMappingAlgCfg
    # acc.merge( ActsDetectorElementToActsGeometryIdMappingAlgCfg(flags) )
    # kwargs.setdefault('DetectorElementToActsGeometryIdMapKey', 'DetectorElementToActsGeometryIdMap')
    kwargs.setdefault("useSCTLayerDep_OverlapCuts", True)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))
    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault('TrackingGeometryTool', acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
        
    acc.setPrivateTools(CompFactory.ActsTrk.CoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsIDPixelSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsIDPixelSpacePointPreparationAlg",
                                         *,
                                         useCache: bool = False,
                                         **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'PixelSpacePoints')
    kwargs.setdefault('DetectorElements', 'PixelDetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsIDStripSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsIDStripSpacePointPreparationAlg",
                                         *,
                                         useCache: bool = False,
                                         **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'SCT_SpacePoints')
    kwargs.setdefault('DetectorElements', 'SCT_DetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsIDStripOverlapSpacePointPreparationAlgCfg(flags,
                                                name: str = 'ActsIDStripOverlapSpacePointPreparationAlg',
                                                *,
                                                useCache: bool = False,
                                                **kwargs: dict) -> ComponentAccumulator:
     kwargs.setdefault('InputCollection', 'OverlapSpacePoints')
     return ActsIDStripSpacePointPreparationAlgCfg(flags, name=name, useCache=useCache, **kwargs)

def ActsIDPixelSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsIDPixelSpacePointFormationAlg",
                                       *,
                                       useCache: bool = False,
                                       **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc.merge(PixelReadoutGeometryCfg(flags))

    kwargs.setdefault('PixelClusters', 'PixelClusters')
    kwargs.setdefault('PixelSpacePoints', 'PixelSpacePoints')

    kwargs.setdefault('PixelDetectorElements','PixelDetectorElementCollection')

    if useCache:
        kwargs.setdefault('SPCacheBackend', 'ActsPixelSpacePointCache_Back')
        kwargs.setdefault('SPCache', 'ActsPixelSpacePointCache')

    if 'SpacePointFormationTool' not in kwargs:
        kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsIDPixelSpacePointToolCfg(flags)))
 
    if useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelCacheSpacePointFormationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    
    return acc

def ActsIDStripSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsIDStripSpacePointFormationAlg",
                                       *,
                                       useCache: bool = False,
                                       **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from InDetConfig.SiSpacePointFormationConfig import InDetSiElementPropertiesTableCondAlgCfg
    acc.merge(InDetSiElementPropertiesTableCondAlgCfg(flags))
        
    kwargs.setdefault('StripClusters', 'SCT_Clusters')
    kwargs.setdefault('StripSpacePoints', 'SCT_SpacePoints')
    kwargs.setdefault('StripOverlapSpacePoints', 'OverlapSpacePoints')

    kwargs.setdefault('StripDetectorElements', 'SCT_DetectorElementCollection')
    kwargs.setdefault('StripElementPropertiesTable', 'SCT_ElementPropertiesTable')

    if useCache:
        kwargs.setdefault('SPCacheBackend', 'ActsStripSpacePointCache_Back')
        kwargs.setdefault('SPCache', 'ActsStripSpacePointCache')
        kwargs.setdefault('OSPCacheBackend', 'ActsStripOverlapSpacePointCache_Back')
        kwargs.setdefault('OSPCache', 'ActsStripOverlapSpacePointCache')

    if 'SpacePointFormationTool' not in kwargs:
        # from ActsConfig.ActsConfigFlags import SpacePointStrategy
        # if flags.Acts.SpacePointStrategy is SpacePointStrategy.ActsCore:
        #     kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsIDCoreStripSpacePointToolCfg(flags)))
        # else:
        #     kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsIDStripSpacePointToolCfg(flags)))
        kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsIDStripSpacePointToolCfg(flags)))

    if useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripCacheSpacePointFormationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsIDMainSpacePointFormationCfg(flags,
                                   *,
                                   RoIs: str = "ActsRegionOfInterest",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
  
    kwargs.setdefault('processPixels', flags.Detector.EnablePixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableSCT)
    kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
    kwargs.setdefault('runReconstruction', True)
    kwargs.setdefault('runPreparation', flags.Acts.useCache)  
    kwargs.setdefault('processOverlapSpacePoints', True)

    # has not been validated, CacheCreator copied from ActsSpacePointFormationConfig.py
    # if kwargs['runCacheCreation']:
    #     acc.merge(ActsSpacePointCacheCreatorAlgCfg(flags, **extractChildKwargs(prefix='SpacePointCacheCreatorAlg.', **kwargs)))

    if kwargs['runReconstruction']:
        if kwargs['processPixels']:
            acc.merge(ActsIDPixelSpacePointFormationAlgCfg(flags,**extractChildKwargs(prefix='PixelSpacePointFormationAlg.', **kwargs)))
        if kwargs['processStrips']:
            acc.merge(ActsIDStripSpacePointFormationAlgCfg(flags, **extractChildKwargs(prefix='StripSpacePointFormationAlg.', **kwargs)))
    
    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            acc.merge(ActsIDPixelSpacePointPreparationAlgCfg(flags,
                                                           RoIs=RoIs,
                                                           **extractChildKwargs(prefix='PixelSpacePointPreparationAlg.', **kwargs)))

        if kwargs['processStrips']:
            acc.merge(ActsIDStripSpacePointPreparationAlgCfg(flags,
                                                            RoIs=RoIs,
                                                            **extractChildKwargs(prefix='StripSpacePointPreparationAlg.', **kwargs)))

        if kwargs['processOverlapSpacePoints']:
            acc.merge(ActsIDStripOverlapSpacePointPreparationAlgCfg(flags,
                                                                    RoIs=RoIs,
                                                                    **extractChildKwargs(prefix='StripOverlapSpacePointPreparationAlg.', **kwargs)))

    return acc

def ActsIDSpacePointFormationCfg(flags,
                               *,
                               previousActsExtension = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    processPixels = flags.Detector.EnablePixel
    processStrips = flags.Detector.EnableSCT
        
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
    if flags.Tracking.ActiveConfig.extension in ['ActsConversion']:
        processOverlapSpacePoints = False
    kwargs.setdefault('processOverlapSpacePoints', processOverlapSpacePoints)
    
    # Name of the RoI to be used
    roisName = f'{flags.Tracking.ActiveConfig.extension}RegionOfInterest'
    # Large Radius pass uses the same roi as the primary pass (FS roi)
    if flags.Tracking.ActiveConfig.extension == 'ActsLargeRadius':
        roisName = 'ActsRegionOfInterest'
      
    # Cluster Collection name(s) and Space Point Collection name(s)
    # The name depends on the tracking pass as well as the cache mechanism
    pixelClustersName = 'PixelClusters'
    stripClustersName = 'SCT_Clusters'
    pixelSpacePointsName = 'PixelSpacePoints'
    stripSpacePointsName = 'SCT_SpacePoints'
    stripOverlapSpacePointsName = 'OverlapSpacePoints'

    # if cache is enabled, add "_Cached" at the end
    # if flags.Acts.useCache:
    #     pixelClustersName += "_Cached"
    #     stripClustersName += "_Cached"

    # Primary collections for space points (i.e. produced by primary pass)
    primaryPixelSpacePointsName = 'PixelSpacePoints'
    primaryStripSpacePointsName = 'SCT_SpacePoints'
    primaryStripOverlapSpacePointsName = 'OverlapSpacePoints'
        
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
            # Prd Map
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('PixelSpacePointPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')
               
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
            # Prd Map
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('StripSpacePointPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')

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
            # Prd Map
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('StripOverlapSpacePointPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')
 
    # Analysis algo(s)
    if flags.Acts.doAnalysis:
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
               
    acc.merge(ActsIDMainSpacePointFormationCfg(flags, RoIs=roisName, **kwargs))
    return acc