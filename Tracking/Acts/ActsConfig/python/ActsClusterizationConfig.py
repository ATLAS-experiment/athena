# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsPixelClusteringToolCfg(flags,
                               name: str = "ActsPixelClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'PixelRDOTool' not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkPixelRDOToolCfg
        kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))
    if 'ClusterMakerTool' not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkClusterMakerToolCfg
        kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(ITkClusterMakerToolCfg(flags)))

    kwargs.setdefault("PixelOfflineCalibData", "")

    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc


def ActsStripClusteringToolCfg(flags,
                               name: str = "ActsStripClusteringTool",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'StripConditionsTool' not in kwargs:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("StripConditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))

    if 'conditionsTool' not in kwargs:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))

    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)

    if flags.ITk.selectStripIntimeHits and 'timeBins' not in kwargs:
        from AthenaConfiguration.Enums import BeamType
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsPixelClusterizationAlgCfg(flags,
                                  name: str = 'ActsPixelClusterizationAlg', 
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("expectedClustersPerRDO", 32)
    kwargs.setdefault("IDHelper", "PixelID")
    kwargs.setdefault("RDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("ClustersKey", "ITkPixelClusters")
    # Regional selection
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

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

    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    return acc

def ActsStripClusterizationAlgCfg(flags, 
                                  name: str = 'ActsStripClusterizationAlg', 
                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault("RDOContainerKey", "ITkStripRDOs")
    kwargs.setdefault("ClustersKey", "ITkStripClusters")
    kwargs.setdefault("expectedClustersPerRDO", 6)
    kwargs.setdefault("IDHelper", "SCT_ID")
    # Regional selection
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

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

    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))
    return acc

def ActsClusterCacheCreatorCfg(flags, name="ActsClusterCacheCreator", **kwargs):
    kwargs.setdefault("PixelClustersCacheKey", "ActsPixelClusterCache_Back")
    kwargs.setdefault("StripClustersCacheKey", "ActsStripClusterCache_Back")

    acc = ComponentAccumulator()

    acc.addEventAlgo(CompFactory.ActsTrk.Cache.CreatorAlg(name, **kwargs))

    return acc

def ActsPixelClustersViewFillerCfg(flags,name="PixelClusterViewFiller", **kwargs):
    kwargs.setdefault("InputIDC", "ActsPixelClustersCache")
    kwargs.setdefault("Output", "ITKPixelClusters_InView")

    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

    acc = ComponentAccumulator()

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))
        
    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterCacheFillerAlg(name, **kwargs))

    return acc

def ActsStripClustersViewFillerCfg(flags,name="StripClusterViewFiller", **kwargs):
    kwargs.setdefault("InputIDC", "ActsStripClustersCache")
    kwargs.setdefault("Output", "ITKStripClusters_InView")

    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

    acc = ComponentAccumulator()

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))
        
    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterCacheFillerAlg(name, **kwargs))

    return acc


def ActsMainClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge(ActsClusterCacheCreatorCfg(flags))
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelClusterizationAlgCfg(flags))
        acc.merge(ActsPixelClustersViewFillerCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterizationAlgCfg(flags))
        acc.merge(ActsStripClustersViewFillerCfg(flags))

    # Analysis extensions
    if flags.Acts.doAnalysis:
        if flags.Detector.EnableITkPixel:
            from ActsConfig.ActsAnalysisConfig import ActsPixelClusterAnalysisAlgCfg
            acc.merge(ActsPixelClusterAnalysisAlgCfg(flags))
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripClusterAnalysisAlgCfg
            acc.merge(ActsStripClusterAnalysisAlgCfg(flags))

    return acc

def ActsConversionClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterizationAlgCfg(flags,
                                                name="ActsConversionStripClusterizationAlg",
                                                ClustersKey="ITkConversionStripClusters",
                                                ClusterCache="",
                                                RoIs="OfflineCaloBasedRegion"))

    # Analysis extensions
    if flags.Acts.doAnalysis:
        if flags.Detector.EnableITkStrip:
            from ActsConfig.ActsAnalysisConfig import ActsStripClusterAnalysisAlgCfg
            acc.merge(ActsStripClusterAnalysisAlgCfg(flags,
                                                     name="ActsConversionStripClusterAnalysisAlg",
                                                     extension="ActsConversion",
                                                     ClusterContainerKey="ITkConversionStripClusters",
                                                     MonGroupName="ActsConversionClusterAnalysisAlg"))

    return acc

def ActsClusterizationCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Acts Main pass
    if flags.Tracking.ActiveConfig.extension == "Acts":
        acc.merge(ActsMainClusterizationCfg(flags))
    # Acts Conversion pass
    elif flags.Tracking.ActiveConfig.extension == "ActsConversion":
        acc.merge(ActsConversionClusterizationCfg(flags))
    # Any other pass -> Validation mainly
    else:
        acc.merge(ActsMainClusterizationCfg(flags))

    return acc
