# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsUtilities import extractChildKwargs


def ActsSpacePointCacheCreatorAlgCfg(flags,
                                     name: str = "ActsSpacePointCacheCreatorAlg",
                                     **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelSpacePointCacheKey", "ActsPixelSpacePointCache_Back")
    kwargs.setdefault("StripSpacePointCacheKey", "ActsStripSpacePointCache_Back")
    kwargs.setdefault("StripOverlapSpacePointCacheKey", "ActsStripOverlapSpacePointCache_Back")
    acc.addEventAlgo(CompFactory.ActsTrk.Cache.CreatorAlg(name, **kwargs))
    return acc

def ActsPixelSpacePointToolCfg(flags,
                               name: str = "ActsPixelSpacePointTool",
                               **kwargs: dict) -> ComponentAccumulator:
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass

    acc = ComponentAccumulator()
    if isFastPrimaryPass(flags):
        kwargs.setdefault('UseMaxVariance', True)
    acc.setPrivateTools(CompFactory.ActsTrk.PixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsCorePixelSpacePointToolCfg(flags,
                                   name: str = "ActsCorePixelSpacePointTool",
                                   **kwargs: dict) -> ComponentAccumulator:
    from InDetConfig.ITkActsHelpers import isFastPrimaryPass

    acc = ComponentAccumulator()
    if isFastPrimaryPass(flags):
        kwargs.setdefault('UseMaxVariance', True)

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsTrackingGeometrySvcCfg(flags))

    acc.setPrivateTools(CompFactory.ActsTrk.CorePixelSpacePointFormationTool(name, **kwargs))
    return acc

def ActsStripSpacePointToolCfg(flags,
                               name: str = "ActsStripSpacePointTool",
                               **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("useSCTLayerDep_OverlapCuts", False)

    kwargs.setdefault("useBeamSpotConstraint", flags.Acts.SpacePoints.useBeamSpotConstraintStrips)

    # Cosmic cut values are taken from InDetConfig.SiSpacePointFormationConfig
    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("StripLengthTolerance", 0.05)
        kwargs.setdefault("OverlapLimitOpposite", 5)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    acc.setPrivateTools(CompFactory.ActsTrk.StripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsCoreStripSpacePointToolCfg(flags,
                                   name: str = "ActsCoreStripSpacePointTool",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("useSCTLayerDep_OverlapCuts", False)

    # This tool has no equivalent of the Athena tool's useBeamSpotConstraint=False
    if not flags.Acts.SpacePoints.useBeamSpotConstraintStrips:
        raise RuntimeError("Acts.SpacePoints.useBeamSpotConstraintStrips=False is not supported "
                           "by CoreStripSpacePointFormationTool, use Acts.SpacePointStrategy.ActsTrk")

    # Cosmic cut values are taken from InDetConfig.SiSpacePointFormationConfig
    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("Mode", "Cosmic")
        kwargs.setdefault("StripLengthTolerance", 0.05)
        kwargs.setdefault("OverlapLimitOpposite", 5)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    acc.setPrivateTools(CompFactory.ActsTrk.CoreStripSpacePointFormationTool(name, **kwargs))
    return acc

def ActsPixelSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsPixelSpacePointPreparationAlg",
                                         *,
                                         useCache: bool = False,
                                         **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'ITkPixelSpacePoints')
    kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')
    
    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsPixelSpacePointPreparationMonitoringTool")))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsStripSpacePointPreparationAlgCfg(flags,
                                         name: str = "ActsStripSpacePointPreparationAlg",
                                         *,
                                         useCache: bool = False,
                                         **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'ITkStripSpacePoints')
    kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')
    
    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsStripSpacePointPreparationMonitoringTool")))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.SpacePointCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsStripOverlapSpacePointPreparationAlgCfg(flags,
                                                name: str = 'ActsStripOverlapSpacePointPreparationAlg',
                                                *,
                                                useCache: bool = False,
                                                **kwargs: dict) -> ComponentAccumulator:
     kwargs.setdefault('InputCollection', 'ITkStripOverlapSpacePoints')
     return ActsStripSpacePointPreparationAlgCfg(flags, name=name, useCache=useCache, **kwargs)

def ActsPixelSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsPixelSpacePointFormationAlg",
                                       *,
                                       useCache: bool = False,
                                       **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    kwargs.setdefault('PixelClusters', 'ITkPixelClusters')
    kwargs.setdefault('PixelSpacePoints', 'ITkPixelSpacePoints') 
    kwargs.setdefault('ExtraOutputs',
                      [('xAOD::SpacePointContainer' , f'StoreGateSvc+{kwargs["PixelSpacePoints"]}.measurements')])
    
    if useCache:
        kwargs.setdefault('SPCacheBackend', 'ActsPixelSpacePointCache_Back')
        kwargs.setdefault('SPCache', 'ActsPixelSpacePointCache')
    
    if 'SpacePointFormationTool' not in kwargs:
        from ActsConfig.ActsConfigFlags import SpacePointStrategy
        if flags.Acts.PixelSpacePointStrategy is SpacePointStrategy.ActsCore:
            # The algorithm holds the geometry context this tool needs
            from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
            acc.merge(ActsGeometryContextAlgCfg(flags))
            # ContextUtility also carries a magnetic field key, which is unused here
            kwargs.setdefault('MagneticContextKey', '')
            kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsCorePixelSpacePointToolCfg(flags)))
        else:
            kwargs.setdefault("SpacePointFormationTool", acc.popToolsAndMerge(ActsPixelSpacePointToolCfg(flags)))

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsPixelSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsPixelSpacePointFormationMonitoringToolCfg(flags)))


    if useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelCacheSpacePointFormationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelSpacePointFormationAlg(name, **kwargs))
    
    return acc

def ActsStripSpacePointFormationAlgCfg(flags,
                                       name: str = "ActsStripSpacePointFormationAlg",
                                       *,
                                       useCache: bool = False,
                                       **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))
    
    from InDetConfig.SiSpacePointFormationConfig import ITkSiElementPropertiesTableCondAlgCfg
    acc.merge(ITkSiElementPropertiesTableCondAlgCfg(flags))
    
        
    kwargs.setdefault('StripClusters', 'ITkStripClusters')
    kwargs.setdefault('StripSpacePoints', 'ITkStripSpacePoints')
    kwargs.setdefault('StripOverlapSpacePoints', 'ITkStripOverlapSpacePoints')
    kwargs.setdefault('ExtraOutputs',
                      [('xAOD::SpacePointContainer' , f'StoreGateSvc+{kwargs["StripSpacePoints"]}.measurements'),
                       ('xAOD::SpacePointContainer' , f'StoreGateSvc+{kwargs["StripOverlapSpacePoints"]}.measurements')])

    
    if useCache:
        kwargs.setdefault('SPCacheBackend', 'ActsStripSpacePointCache_Back')
        kwargs.setdefault('SPCache', 'ActsStripSpacePointCache')
        kwargs.setdefault('OSPCacheBackend', 'ActsStripOverlapSpacePointCache_Back')
        kwargs.setdefault('OSPCache', 'ActsStripOverlapSpacePointCache')

    from ActsConfig.ActsConfigFlags import SpacePointStrategy
    useActsCore = flags.Acts.SpacePointStrategy is SpacePointStrategy.ActsCore

    if 'SpacePointFormationTool' not in kwargs:
        if useActsCore:
            kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsCoreStripSpacePointToolCfg(flags)))
        else:
            kwargs.setdefault('SpacePointFormationTool', acc.popToolsAndMerge(ActsStripSpacePointToolCfg(flags)))

    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault('OverrideBeamSpot', True)
        kwargs.setdefault('VertexX', 0)
        kwargs.setdefault('VertexZ', 0)
        # The Athena implementation needs the far-away point for the vertical track
        # hypothesis; ActsCore in cosmic mode ignores the vertex and must stay at the origin
        kwargs.setdefault('VertexY', 0 if useActsCore else 99999999)
        kwargs.setdefault('ProcessOverlapForStrip', False)

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsStripSpacePointFormationMonitoringToolCfg
        kwargs.setdefault("MonTool", acc.popToolsAndMerge(ActsStripSpacePointFormationMonitoringToolCfg(flags)))

    if useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripCacheSpacePointFormationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripSpacePointFormationAlg(name, **kwargs))
    return acc

def ActsMainSpacePointFormationCfg(flags,
                                   *,
                                   RoIs: str = "ActsRegionOfInterest",
                                   **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('processPixels', flags.Acts.SpacePoints.doPixel)
    kwargs.setdefault('processStrips', flags.Acts.SpacePoints.doStrip)
    kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
    kwargs.setdefault('runReconstruction', True)
    kwargs.setdefault('runPreparation', flags.Acts.useCache)  
    kwargs.setdefault('processOverlapSpacePoints', True)
    
    if kwargs['runCacheCreation']:
        acc.merge(ActsSpacePointCacheCreatorAlgCfg(flags, **extractChildKwargs(prefix='SpacePointCacheCreatorAlg.', **kwargs)))

    if kwargs['runReconstruction']:
        if kwargs['processPixels']:
            acc.merge(ActsPixelSpacePointFormationAlgCfg(flags,**extractChildKwargs(prefix='PixelSpacePointFormationAlg.', **kwargs)))
            
        if kwargs['processStrips']:
            acc.merge(ActsStripSpacePointFormationAlgCfg(flags, **extractChildKwargs(prefix='StripSpacePointFormationAlg.', **kwargs)))

    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            acc.merge(ActsPixelSpacePointPreparationAlgCfg(flags,
                                                           RoIs=RoIs,
                                                           **extractChildKwargs(prefix='PixelSpacePointPreparationAlg.', **kwargs)))
        if kwargs['processStrips']:
            acc.merge(ActsStripSpacePointPreparationAlgCfg(flags,
                                                           RoIs=RoIs,
                                                           **extractChildKwargs(prefix='StripSpacePointPreparationAlg.', **kwargs)))
        if kwargs['processOverlapSpacePoints']:
            acc.merge(ActsStripOverlapSpacePointPreparationAlgCfg(flags,
                                                                  RoIs=RoIs,
                                                                  **extractChildKwargs(prefix='StripOverlapSpacePointPreparationAlg.', **kwargs)))
            
    # Analysis extensions
    if flags.Acts.SpacePoints.doAnalysis:
        if kwargs['processPixels']:
            from ActsConfig.ActsAnalysisConfig import ActsPixelSpacePointAnalysisAlgCfg
            acc.merge(ActsPixelSpacePointAnalysisAlgCfg(flags, **extractChildKwargs(prefix='PixelSpacePointAnalysisAlg.', **kwargs)))
        if kwargs['processStrips']:
            from ActsConfig.ActsAnalysisConfig import ActsStripSpacePointAnalysisAlgCfg
            acc.merge(ActsStripSpacePointAnalysisAlgCfg(flags, **extractChildKwargs(prefix='StripSpacePointAnalysisAlg.', **kwargs)))
        if kwargs['processOverlapSpacePoints']:
            from ActsConfig.ActsAnalysisConfig import ActsStripOverlapSpacePointAnalysisAlgCfg
            acc.merge(ActsStripOverlapSpacePointAnalysisAlgCfg(flags, **extractChildKwargs(prefix='StripOverlapSpacePointAnalysisAlg.', **kwargs)))

    return acc

# Config to be called outside of loops over tracking passes in main reco
# Will configure subtools based on MainPass
def ActsMainSpacePointFormationStandaloneCfg(flags) -> ComponentAccumulator:
    primaryFlags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass")
    return ActsMainSpacePointFormationCfg(primaryFlags)

def ActsSpacePointFormationCfg(flags,
                               *,
                               previousActsExtension = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs = dict()

    processPixels = flags.Tracking.ActiveConfig.useITkPixelSeeding or (
        flags.Acts.SpacePoints.doPixel and not flags.Tracking.ActiveConfig.isSecondaryPass)
    processStrips = flags.Tracking.ActiveConfig.useITkStripSeeding or (
        flags.Acts.SpacePoints.doStrip and not flags.Tracking.ActiveConfig.isSecondaryPass)

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

    from InDetConfig.ITkActsHelpers import isValidationPass
    if not flags.Tracking.ActiveConfig.isSecondaryPass or isValidationPass(flags):
        # Primary pass
        # Validation passes count as primary passes
        kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
        kwargs.setdefault('runReconstruction', True)
        kwargs.setdefault('runPreparation', flags.Acts.useCache)
    else:
        # Secondary passes
        kwargs.setdefault('runCacheCreation', False)
        kwargs.setdefault('runReconstruction', flags.Acts.useCache)
        kwargs.setdefault('runPreparation', True)

    # super special configuration (TEMP)
    if flags.Acts.makeGlobalDataPreparation:
        kwargs['runCacheCreation'] = False
        kwargs['runReconstruction'] = False
        kwargs['runPreparation'] = not isPrimaryPass(flags)
        
    # Overlap Space Points may not be required
    processOverlapSpacePoints = kwargs['processStrips']
    from AthenaConfiguration.Enums import BeamType
    if flags.Tracking.ActiveConfig.extension in ['ActsConversion'] or flags.Beam.Type is BeamType.Cosmics:
        processOverlapSpacePoints = False
    kwargs.setdefault('processOverlapSpacePoints', processOverlapSpacePoints)
        
    # Name of the RoI to be used
    roisName = f'{flags.Tracking.ActiveConfig.extension}RegionOfInterest'
    # Large Radius pass uses the same roi as the primary pass (FS roi)
    if flags.Tracking.ActiveConfig.isLargeD0 and flags.Tracking.ActiveConfig.isSecondaryPass:
        from InDetConfig.ITkActsHelpers import primaryPassExtension
        roisName = f'{primaryPassExtension(flags)}RegionOfInterest'
    
    # Cluster Collection name(s) and Space Point Collection name(s)
    # The name depends on the tracking pass as well as the cache mechanism
    pixelClustersName = 'ITkPixelClusters'
    stripClustersName = 'ITkStripClusters'
    pixelSpacePointsName = 'ITkPixelSpacePoints'
    stripSpacePointsName = 'ITkStripSpacePoints'
    stripOverlapSpacePointsName = 'ITkStripOverlapSpacePoints'
    # Secondary passes modify the collection name
    if flags.Tracking.ActiveConfig.isSecondaryPass:
        pixelClustersName = f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}PixelClusters'
        stripClustersName = f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}StripClusters'
        pixelSpacePointsName = f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}PixelSpacePoints'
        stripSpacePointsName =  f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}StripSpacePoints'
        stripOverlapSpacePointsName = f'ITk{flags.Tracking.ActiveConfig.extension.replace("Acts", "")}StripOverlapSpacePoints'
    # if cache is enabled, add "_Cached" at the end
    if flags.Acts.useCache:
        pixelClustersName += "_Cached"
        stripClustersName += "_Cached"

    # Primary collections for space points (i.e. produced by primary pass)
    primaryPixelSpacePointsName = 'ITkPixelSpacePoints'
    primaryStripSpacePointsName = 'ITkStripSpacePoints'
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
                
    acc.merge(ActsMainSpacePointFormationCfg(flags, RoIs=roisName, **kwargs))


    # Persistification
    if kwargs['runReconstruction']:        
        from ActsConfig.ActsPersistificationConfig import PersistifySpacePoints
        pixelSpacePointCollections = None if not kwargs['processPixels'] else [kwargs['PixelSpacePointFormationAlg.PixelSpacePoints']]
        stripSpacePointCollections = []
        if kwargs['processStrips']:
            stripSpacePointCollections.append(kwargs['StripSpacePointFormationAlg.StripSpacePoints'])
        if kwargs['processOverlapSpacePoints']:
            stripSpacePointCollections.append(kwargs['StripSpacePointFormationAlg.StripOverlapSpacePoints'])
        if len(stripSpacePointCollections) == 0:
            stripSpacePointCollections = None

        acc.merge(PersistifySpacePoints(flags,
                                        pixelSpacePointCollections=pixelSpacePointCollections,
                                        stripSpacePointCollections=stripSpacePointCollections))
    return acc

