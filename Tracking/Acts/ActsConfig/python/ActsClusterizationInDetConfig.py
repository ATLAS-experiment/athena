# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from ActsConfig.ActsUtilities import extractChildKwargs

from ActsConfig.ActsClusterizationConfig import ActsClusterCacheCreatorAlgCfg
def ActsIDPixelClusteringToolCfg(flags,
                               name: str = "ActsIDPixelClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("isITk", False)

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelChargeCalibCondAlgCfg, PixelOfflineCalibCondAlgCfg
    acc.merge(PixelChargeCalibCondAlgCfg(flags))
    acc.merge(PixelOfflineCalibCondAlgCfg(flags))
    kwargs.setdefault("PixelChargeCalibCondData", "PixelChargeCalibCondData")

    if "PixelLorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
        kwargs.setdefault("PixelLorentzAngleTool", acc.popToolsAndMerge( PixelLorentzAngleToolCfg(flags) ))

    kwargs.setdefault("CheckGanged", True)
    kwargs.setdefault('UseWeightedPosition',False) #     not (flags.Tracking.doPixelDigitalClustering or flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault('UseBroadErrors', flags.Beam.Type is BeamType.Cosmics)

    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc

def ActsIDStripClusteringToolCfg(flags,
                               name: str = "ActsIDStripClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("isITk", False)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False)))

    if "StripDetElStatus" not in kwargs :
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import  (
            SCT_DetectorElementStatusAlgWithoutFlaggedCfg)
        acc.merge(SCT_DetectorElementStatusAlgWithoutFlaggedCfg(flags))
        kwargs.setdefault("StripDetElStatus", "SCTDetectorElementStatusWithoutFlagged")

    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)
    
    if flags.InDet.selectSCTIntimeHits:
        coll_25ns = (flags.Beam.BunchSpacing <= 25 and
                     flags.Beam.Type is BeamType.Collisions)
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    kwargs.setdefault("StripDetEleCollKey", "SCT_DetectorElementCollection")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsIDPixelClusterizationAlgCfg(flags,
                                  name: str = 'ActsIDPixelClusterizationAlg',
                                  *,
                                  useCache: bool = False,
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("IDHelper", "PixelID")
    kwargs.setdefault("RDOContainerKey", "PixelRDOs")
    kwargs.setdefault("ClustersKey", "PixelClusters")
    kwargs.setdefault("DetEleCollKey", "PixelDetectorElementCollection")
    # Regional selection
    kwargs.setdefault('RoIs', 'ActsRegionOfInterest')

    kwargs.setdefault('ClusterCacheBackend', 'ActsPixelClusterCache_Back')
    kwargs.setdefault('ClusterCache', 'ActsPixelClustersCache')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags)))

    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsIDPixelClusteringToolCfg(flags)))
    
    if 'DetElStatus' not in kwargs:
        from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementStatusAlgCfg
        acc.merge(PixelDetectorElementStatusAlgCfg(flags))
        kwargs.setdefault('DetElStatus', 'PixelDetectorElementStatus')

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelCacheClusterizationAlg(name, **kwargs))
    return acc

def ActsIDStripClusterizationAlgCfg(flags, 
                                  name: str = 'ActsIDStripClusterizationAlg',
                                  useCache: bool = False,
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("RDOContainerKey", "SCT_RDOs")
    kwargs.setdefault("ClustersKey", "SCT_Clusters")
    kwargs.setdefault("IDHelper", "SCT_ID")
    kwargs.setdefault("DetEleCollKey", "SCT_DetectorElementCollection")
    # Regional selection
    kwargs.setdefault('RoIs', 'ActsRegionOfInterest')

    kwargs.setdefault('ClusterCacheBackend', 'ActsStripClusterCache_Back')
    kwargs.setdefault('ClusterCache', 'ActsStripClustersCache')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags)))

    if 'DetElStatus' not in kwargs :
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import  SCT_DetectorElementStatusAlgWithoutFlaggedCfg
        acc.merge(SCT_DetectorElementStatusAlgWithoutFlaggedCfg(flags))
        kwargs.setdefault("DetElStatus", "SCTDetectorElementStatusWithoutFlagged")

    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsIDStripClusteringToolCfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripCacheClusterizationAlg(name, **kwargs))
    return acc

def ActsIDPixelClusterPreparationAlgCfg(flags,
                                      name: str = "ActsIDPixelClusterPreparationAlg",
                                      useCache: bool = False,
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'PixelClusters')
    kwargs.setdefault('DetectorElements', 'PixelDetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_Pixel_Cfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsIDStripClusterPreparationAlgCfg(flags,
                                      name: str = "ActsIDStripClusterPreparationAlg",
                                      useCache: bool = False,
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputCollection', 'SCT_Clusters')
    kwargs.setdefault('DetectorElements', 'SCT_DetectorElementCollection')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_SCT_Cfg(flags)))

    if not useCache:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterDataPreparationAlg(name, **kwargs))
    else:
        acc.addEventAlgo(CompFactory.ActsTrk.StripClusterCacheDataPreparationAlg(name, **kwargs))
    return acc

def ActsIDMainClusterizationCfg(flags,
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
    
    kwargs.setdefault('processPixels', flags.Detector.EnablePixel)
    kwargs.setdefault('processStrips', flags.Detector.EnableSCT)
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
            acc.merge(ActsIDPixelClusterizationAlgCfg(flags,
                                                    RoIs=RoIs,
                                                    **extractChildKwargs(prefix='PixelClusterizationAlg.', **kwargs)))
        if kwargs['processStrips']:
            acc.merge(ActsIDStripClusterizationAlgCfg(flags,
                                                    RoIs=RoIs,
                                                    **extractChildKwargs(prefix='StripClusterizationAlg.', **kwargs)))

    # Step (3)
    if kwargs['runPreparation']:
        if kwargs['processPixels']:
            acc.merge(ActsIDPixelClusterPreparationAlgCfg(flags,
                                                        RoIs=RoIs,
                                                        **extractChildKwargs(prefix='PixelClusterPreparationAlg.', **kwargs)))

        if kwargs['processStrips']:
            acc.merge(ActsIDStripClusterPreparationAlgCfg(flags,
                                                        RoIs=RoIs,
                                                        **extractChildKwargs(prefix='StripClusterPreparationAlg.', **kwargs)))

    return acc

def ActsIDClusterizationCfg(flags,
                          *,
                          previousActsExtension: str = None) -> ComponentAccumulator:
    assert previousActsExtension is None or isinstance(previousActsExtension, str)

    acc = ComponentAccumulator()
                      
    processPixels = flags.Detector.EnablePixel
    processStrips = flags.Detector.EnableSCT

    kwargs = dict()
    kwargs.setdefault('processPixels', processPixels)
    kwargs.setdefault('processStrips', processStrips)

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


    # Only Primary pass for the Inner Detector now
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
    primaryPixelClustersName = 'PixelClusters'
    primaryStripClustersName = 'SCT_Clusters'
    pixelClustersName = primaryPixelClustersName
    stripClustersName = primaryStripClustersName
    
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

        if kwargs['processStrips']:
            kwargs.setdefault('StripClusterizationAlg.name', f'{flags.Tracking.ActiveConfig.extension}StripClusterizationAlg')
            kwargs.setdefault('StripClusterizationAlg.useCache', flags.Acts.useCache)
            kwargs.setdefault('StripClusterizationAlg.ClustersKey', stripClustersName)
            kwargs.setdefault('StripClusterizationAlg.ClusterCache', f'{flags.Tracking.ActiveConfig.extension}StripClustersCache')

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

    acc.merge(ActsIDMainClusterizationCfg(flags, RoIs=roisName, **kwargs))
    return acc
