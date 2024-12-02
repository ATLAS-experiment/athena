# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from ActsConfig.ActsUtilities import extractChildKwargs

def ActsHgtdClusteringToolCfg(flags,
                              name: str = "ActsHgtdClusteringTool",
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.HgtdClusteringTool(name, **kwargs))    
    return acc

def ActsHgtdTimedClusteringToolCfg(flags,
                                   name: str = "ActsHgtdTimedClusteringTool",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.ActsTrk.HgtdTimedClusteringTool(name, **kwargs))    
    return acc

def ActsHgtdClusterizationAlgCfg(flags,
                                 name: str = "ActsHgtdClusterizationAlg",
                                 **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    acc.merge(HGTD_ReadoutGeometryCfg(flags))

    kwargs.setdefault('RDOContainerName', 'HGTD_RDOs')
    kwargs.setdefault('ClusterContainerName', 'HGTD_Clusters')

    if 'ClusteringTool' not in kwargs:
        from HGTD_Config.HGTD_ConfigFlags import  ClusteringStrategy
        if flags.HGTD.Acts.ClusteringStrategy is ClusteringStrategy.SinglePad:
            kwargs.setdefault('ClusteringTool', acc.popToolsAndMerge(ActsHgtdClusteringToolCfg(flags)))
        else:
            kwargs.setdefault('ClusteringTool', acc.popToolsAndMerge(ActsHgtdTimedClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsHgtdClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsHgtdClusterizationMonitoringToolCfg(flags)))        
        
    acc.addEventAlgo(CompFactory.ActsTrk.HgtdClusterizationAlg(name, **kwargs))
    return acc



def ActsPixelClusteringToolCfg(flags,
                               name: str = "ActsPixelClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelChargeCalibCondAlgCfg, ITkPixelOfflineCalibCondAlgCfg
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    acc.merge(ITkPixelOfflineCalibCondAlgCfg(flags))

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
    acc.merge(ITkPixelReadoutManagerCfg(flags))
    
    if 'PixelRDOTool' not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkPixelRDOToolCfg
        kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))

    if "PixelLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge( ITkPixelLorentzAngleToolCfg(flags) ))

    kwargs.setdefault(
        'UseWeightedPosition',
        not (flags.Tracking.doPixelDigitalClustering or flags.Beam.Type is BeamType.Cosmics)
    )

    kwargs.setdefault('UseBroadErrors', flags.Beam.Type is BeamType.Cosmics)

    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc


def ActsStripClusteringToolCfg(flags,
                               name: str = "ActsStripClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))

    kwargs.setdefault("conditionsTool",None)
    if "StripDetElStatus" not in kwargs :
        from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import  (
            ITkStripDetectorElementStatusAlgCfg)
        acc.merge(ITkStripDetectorElementStatusAlgCfg(flags))
        kwargs.setdefault("StripDetElStatus", "ITkStripDetectorElementStatus")

    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)

    if flags.ITk.selectStripIntimeHits and 'timeBins' not in kwargs:
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsPixelClusterizationAlgCfg(flags,
                                  name: str = 'ActsPixelClusterizationAlg',
                                  *,
                                  useCache: bool = False,
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("expectedClustersPerRDO", 32)
    kwargs.setdefault("IDHelper", "PixelID")
    kwargs.setdefault("RDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("ClustersKey", "ITkPixelClusters")
    # Regional selection
    kwargs.setdefault('RoIs', 'ActsRegionOfInterest')

    kwargs.setdefault('ClusterCacheBackend', 'ActsPixelClusterCache_Back')
    kwargs.setdefault('ClusterCache', 'ActsPixelClustersCache')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsPixelClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkPixelClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkPixelClusterizationMonitoringToolCfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelCacheClusterizationAlg(name, **kwargs))
    return acc

def ActsStripClusterizationAlgCfg(flags, 
                                  name: str = 'ActsStripClusterizationAlg',
                                  useCache: bool = False,
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("RDOContainerKey", "ITkStripRDOs")
    kwargs.setdefault("ClustersKey", "ITkStripClusters")
    kwargs.setdefault("expectedClustersPerRDO", 6)
    kwargs.setdefault("IDHelper", "SCT_ID")
    # Regional selection
    kwargs.setdefault('RoIs', 'ActsRegionOfInterest')

    kwargs.setdefault('ClusterCacheBackend', 'ActsStripClusterCache_Back')
    kwargs.setdefault('ClusterCache', 'ActsStripClustersCache')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))

    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsStripClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkStripClusterizationMonitoringToolCfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripCacheClusterizationAlg(name, **kwargs))
    return acc

def ActsClusterCacheCreatorAlgCfg(flags,
                                  name: str = "ActsClusterCacheCreatorAlg",
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelClustersCacheKey", "ActsPixelClusterCache_Back")
    kwargs.setdefault("StripClustersCacheKey", "ActsStripClusterCache_Back")
    acc.addEventAlgo(CompFactory.ActsTrk.Cache.CreatorAlg(name, **kwargs))
    return acc

def ActsPixelClusterPreparationAlgCfg(flags,
                                      name: str = "ActsPixelClusterPreparationAlg",
                                      useCache: bool = False,
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'ITkPixelClusters')
    kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')
    
    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsPixelClusterPreparationMonitoringTool")))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsStripClusterPreparationAlgCfg(flags,
                                      name: str = "ActsStripClusterPreparationAlg",
                                      useCache: bool = False,
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'ITkStripClusters')
    kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))
        
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsStripClusterPreparationMonitoringTool")))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterCacheDataPreparationAlg(name, **kwargs))
    return acc


def ActsHgtdClusterPreparationAlgCfg(flags,
                                     name: str = 'ActsHgtdClusterPreparationAlg',
                                     *,
                                     useCache: bool = False,
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('InputCollection', 'HGTDClusters')
    kwargs.setdefault('DetectorElements', 'HGTD_DetectorElementCollection')

    # For the time being the HGTD Regional Selector tool does not exist
    
    if flags.Acts.doMonitoring and 'MonTool' not in kwargs:
        from ActsConfig.ActsMonitoringConfig import ActsDataPreparationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsDataPreparationMonitoringToolCfg(flags,
                                                                                               name = "ActsHgtdClusterPreparationMonitoringTool")))

    acc.addEventAlgo(CompFactory.ActsTrk.HgtdClusterDataPreparationAlg(name, **kwargs))
    return acc


def ActsMainClusterizationCfg(flags,
                              *,
                              RoIs: str = "ActsRegionOfInterest",
                              **kwargs: dict) -> ComponentAccumulator:
    assert isinstance(RoIs, str)
    assert isinstance(kwargs, dict)
    
    acc = ComponentAccumulator()

    # Clusterization is a three step process at maximum:
    #   (1) Cache Creation
    #   (2) Clusterization algorithm (reconstruction of clusters)
    #   (3) Preparation of collection for downstream algorithms
    # What step is scheduled depends on the tracking pass and the activation
    # or de-activation of caching mechanism
    
    kwargs.setdefault('processPixels', flags.Detector.EnableITkPixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableITkStrip)
    kwargs.setdefault('processHGTD', flags.Acts.useHGTDClusterInTrackFinding and flags.Detector.EnableHGTD)
    kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
    kwargs.setdefault('runReconstruction', True)
    kwargs.setdefault('runPreparation', flags.Acts.useCache)    

    # Step (1)
    if kwargs['runCacheCreation']:
        acc.merge(ActsClusterCacheCreatorAlgCfg(flags,
                                                **extractChildKwargs(prefix='ClusterCacheCreatorAlg.', **kwargs)))

    # Step (2)
    if kwargs['runReconstruction']:
        if kwargs['processPixels']:
            acc.merge(ActsPixelClusterizationAlgCfg(flags,
                                                    RoIs=RoIs,
                                                    **extractChildKwargs(prefix='PixelClusterizationAlg.', **kwargs)))
        if kwargs['processStrips']:
            acc.merge(ActsStripClusterizationAlgCfg(flags,
                                                    RoIs=RoIs,
                                                    **extractChildKwargs(prefix='StripClusterizationAlg.', **kwargs)))
        if kwargs['processHGTD']:
            acc.merge(ActsHgtdClusterizationAlgCfg(flags,
                                                   **extractChildKwargs(prefix='HgtdClusterizationAlg.', **kwargs)))
    # Step (3)
    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            acc.merge(ActsPixelClusterPreparationAlgCfg(flags,
                                                        RoIs=RoIs,
                                                        **extractChildKwargs(prefix='PixelClusterPreparationAlg.', **kwargs)))
            
        if kwargs['processStrips']:
            acc.merge(ActsStripClusterPreparationAlgCfg(flags,
                                                        RoIs=RoIs,
                                                        **extractChildKwargs(prefix='StripClusterPreparationAlg.', **kwargs)))

        if kwargs['processHGTD']:
            acc.merge(ActsHgtdClusterPreparationAlgCfg(flags,
                                                       RoIs=RoIs,
                                                       **extractChildKwargs(prefix='HgtdClusterPreparationAlg.', **kwargs)))
            
    # Analysis extensions
    if flags.Acts.doAnalysis:
        if kwargs['processPixels']:
            from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg
            acc.merge(ActsPixelClusterAnalysisAlgCfg(flags, **extractChildKwargs(prefix='PixelClusterAnalysisAlg.', **kwargs)))
            
        if kwargs['processStrips']:
            from ActsConfig.ActsAnalysisConfig import ActsStripClusterAnalysisAlgCfg
            acc.merge(ActsStripClusterAnalysisAlgCfg(flags, **extractChildKwargs(prefix='StripClusterAnalysisAlg.', **kwargs)))

    return acc

def ActsClusterizationCfg(flags,
                          *,
                          previousActsExtension: str = None) -> ComponentAccumulator:
    assert previousActsExtension is None or isinstance(previousActsExtension, str)

    acc = ComponentAccumulator()
                      
    processPixels = flags.Detector.EnableITkPixel
    processStrips = flags.Detector.EnableITkStrip
    processHGTD = flags.Acts.useHGTDClusterInTrackFinding and flags.Detector.EnableHGTD

    kwargs = dict()
    kwargs.setdefault('processPixels', processPixels)
    kwargs.setdefault('processStrips', processStrips)
    kwargs.setdefault('processHGTD', processHGTD)

    # Clusterization is a three step process at maximum:
    #   (1) Cache Creation
    #   (2) Clusterization algorithm (reconstruction of clusters)
    #   (3) Preparation of collection for downstream algorithms
    # What step is scheduled depends on the tracking pass and the activation
    # or de-activation of caching mechanism.
    
    # Secondary passes do not need cache creation, that has to be performed
    # on the primary pass, and only if the caching is enabled.
    # Reconstruction can run on secondary passes only if the caching is enabled,
    # this is because we may need to process detector elements not processed
    # on the primary pass.
    # Preparation has to be performed on secondary passes always, and on primary
    # pass only if cache is enabled. In the latter case it is useed to collect all
    # the clusters from all views before passing them to the downstream algorithms

    if flags.Tracking.ActiveConfig.isSecondaryPass:
        # Secondary passes
        kwargs.setdefault('runCacheCreation', False)
        kwargs.setdefault('runReconstruction', flags.Acts.useCache)
        kwargs.setdefault('runPreparation', True)
    else:
        # Primary pass
        kwargs.setdefault('runCacheCreation', flags.Acts.useCache)
        kwargs.setdefault('runReconstruction', True)
        kwargs.setdefault('runPreparation', flags.Acts.useCache)

    # Name of the RoI to be used
    roisName = f'{flags.Tracking.ActiveConfig.extension}RegionOfInterest'
    # Large Radius Tracking uses full scan RoI created in the primary pass
    if flags.Tracking.ActiveConfig.extension == 'ActsLargeRadius':
        roisName = 'ActsRegionOfInterest'
        
    # Name of the Cluster container -> ITk + extension without "Acts" + Pixel or Strip + Clusters
    # We also define the same collection from the main ACTS pass (primary)
    primaryPixelClustersName = 'ITkPixelClusters'
    primaryStripClustersName = 'ITkStripClusters'
    primaryHgtdClustersName = 'HGTD_Clusters'
    pixelClustersName = primaryPixelClustersName
    stripClustersName = primaryStripClustersName
    hgtdClustersName = primaryHgtdClustersName

    # If the workflow is not a primary pass, then change the name of the cluster collections adding that information
    if flags.Tracking.ActiveConfig.isSecondaryPass:
        keyPrefix = flags.Tracking.ActiveConfig.extension.replace("Acts", "")
        pixelClustersName = f'ITk{keyPrefix}PixelClusters'
        stripClustersName = f'ITk{keyPrefix}StripClusters'
        hgtdClustersName = f'{keyPrefix}HGTD_Clusters'
    
    # Configuration for (1)
    if kwargs['runCacheCreation']:
        kwargs.setdefault('ClusterCacheCreatorAlg.name', f'{flags.Tracking.ActiveConfig.extension}ClusterCacheCreatorAlg')

    # Configuration for (2)
    if kwargs['runReconstruction']:
        if kwargs['processPixels']:
            kwargs.setdefault('PixelClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelClusterizationAlg')
            kwargs.setdefault('PixelClusterizationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('PixelClusterizationAlg.ClustersKey', pixelClustersName)
            kwargs.setdefault('PixelClusterizationAlg.ClusterCache', f'{flags.Tracking.ActiveConfig.extension}PixelClustersCache')
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('PixelClusterizationAlg.ExtraInputs', {('ActsTrk::PrepRawDataAssociation', f'StoreGateSvc+{previousActsExtension}PrdMap')})

        if kwargs['processStrips']:
            kwargs.setdefault('StripClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripClusterizationAlg')
            kwargs.setdefault('StripClusterizationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripClusterizationAlg.ClustersKey', stripClustersName)
            kwargs.setdefault('StripClusterizationAlg.ClusterCache', f'{flags.Tracking.ActiveConfig.extension}StripClustersCache')
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('StripClusterizationAlg.ExtraInputs', {('ActsTrk::PrepRawDataAssociation', f'StoreGateSvc+{previousActsExtension}PrdMap')})

        if kwargs['processHGTD']:
            kwargs.setdefault('HgtdClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}HgtdClusterizationAlg')
            # HGTD algo has different convention ... need to use the same
            kwargs.setdefault('HgtdClusterizationAlg.ClusterContainerName', hgtdClustersName)
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('HgtdClusterizationAlg.ExtraInputs', {('ActsTrk::PrepRawDataAssociation', f'StoreGateSvc+{previousActsExtension}PrdMap')})


    # Configuration for (3)
    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            kwargs.setdefault('PixelClusterPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelClusterPreparationAlg')
            kwargs.setdefault('PixelClusterPreparationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('PixelClusterPreparationAlg.OutputCollection', f'{pixelClustersName}_Cached' if kwargs['runReconstruction'] else pixelClustersName)
            # The input is one between the collection (w/o cache) and the IDC (w/ cache)
            if not flags.Acts.useCache:
                # Take the collection from the reconstruction step. If not available take the collection from the primary pass
                kwargs.setdefault('PixelClusterPreparationAlg.InputCollection', pixelClustersName if kwargs['runReconstruction'] else primaryPixelClustersName)
                kwargs.setdefault('PixelClusterPreparationAlg.InputIDC', '')
            else:
                kwargs.setdefault('PixelClusterPreparationAlg.InputCollection', '')
                kwargs.setdefault('PixelClusterPreparationAlg.InputIDC', f'{flags.Tracking.ActiveConfig.extension}PixelClustersCache')
            # Prd Map for removing previously used measurements
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('PixelClusterPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')
                
        if kwargs['processStrips']:
            kwargs.setdefault('StripClusterPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripClusterPreparationAlg')
            kwargs.setdefault('StripClusterPreparationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripClusterPreparationAlg.OutputCollection', f'{stripClustersName}_Cached' if kwargs['runReconstruction'] else stripClustersName)
            if not flags.Acts.useCache:
                kwargs.setdefault('StripClusterPreparationAlg.InputCollection', stripClustersName if kwargs['runReconstruction'] else primaryStripClustersName)
                kwargs.setdefault('StripClusterPreparationAlg.InputIDC', '')
            else:
                kwargs.setdefault('StripClusterPreparationAlg.InputCollection', '')
                kwargs.setdefault('StripClusterPreparationAlg.InputIDC', f'{flags.Tracking.ActiveConfig.extension}StripClustersCache')
            # Prd Map for removing previously used measurements
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('StripClusterPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')

        if kwargs['processHGTD']:
            kwargs.setdefault('HgtdClusterPreparationAlg.name', f'{flags.Tracking.ActiveConfig.extension}HgtdClusterPreparationAlg')
            kwargs.setdefault('HgtdClusterPreparationAlg.OutputCollection', f'{hgtdClustersName}_Cached' if kwargs['runReconstruction'] else hgtdClustersName)
            kwargs.setdefault('HgtdClusterPreparationAlg.InputCollection', hgtdClustersName if kwargs['runReconstruction'] else primaryHgtdClustersName)
            kwargs.setdefault('HgtdClusterPreparationAlg.InputIDC', '')
            if flags.Tracking.ActiveConfig.isSecondaryPass and previousActsExtension is not None:
                kwargs.setdefault('HgtdClusterPreparationAlg.InputPrdMap', f'{previousActsExtension}PrdMap')

    # Analysis algo(s)
    if flags.Acts.doAnalysis:
        # Run analysis code on the resulting cluster collection produced by this tracking pass
        # This collection is the result of (3) if it ran, else the result of (2). We are sure at least one of them run
        if kwargs['processPixels']:
            kwargs.setdefault('PixelClusterAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}PixelClusterAnalysisAlg')
            kwargs.setdefault('PixelClusterAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('PixelClusterAnalysisAlg.MonGroupName', f'{flags.Tracking.ActiveConfig.extension}ClusterAnalysisAlg')
            kwargs.setdefault('PixelClusterAnalysisAlg.ClusterContainerKey', kwargs['PixelClusterPreparationAlg.OutputCollection'] if kwargs['runPreparation'] else kwargs['PixelClusterizationAlg.ClustersKey'])

        if kwargs['processStrips']:
            kwargs.setdefault('StripClusterAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripClusterAnalysisAlg')
            kwargs.setdefault('StripClusterAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('StripClusterAnalysisAlg.MonGroupName', f'{flags.Tracking.ActiveConfig.extension}ClusterAnalysisAlg')
            kwargs.setdefault('StripClusterAnalysisAlg.ClusterContainerKey', kwargs['StripClusterPreparationAlg.OutputCollection'] if kwargs['runPreparation'] else kwargs['StripClusterizationAlg.ClustersKey'])
                
        if kwargs['processHGTD']:
            kwargs.setdefault('HgtdClusterAnalysisAlg.name', f'{flags.Tracking.ActiveConfig.extension}HgtdClusterAnalysisAlg')
            kwargs.setdefault('HgtdClusterAnalysisAlg.extension', flags.Tracking.ActiveConfig.extension)
            kwargs.setdefault('HgtdClusterAnalysisAlg.MonGroupName', f'{flags.Tracking.ActiveConfig.extension}ClusterAnalysisAlg')

    acc.merge(ActsMainClusterizationCfg(flags, RoIs=roisName, **kwargs))
    return acc

