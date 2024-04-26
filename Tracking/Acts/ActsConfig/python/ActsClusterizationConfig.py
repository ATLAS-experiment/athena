# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType

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

    if 'conditionsTool' not in kwargs:
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))

    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)

    if flags.ITk.selectStripIntimeHits and 'timeBins' not in kwargs:
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsPixelClusterizationAlgCfg(flags,
                                  name: str = 'ActsPixelClusterizationAlg',
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

def ActsMainClusterizationCfg(flags,
                              RoIs: str = "ActsRegionOfInterest") -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Acts.useCache:
        acc.merge(ActsClusterCacheCreatorAlgCfg(flags))

    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelClusterizationAlgCfg(flags,
                                                RoIs=RoIs,
                                                useCache=flags.Acts.useCache))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterizationAlgCfg(flags,
                                                RoIs=RoIs,
                                                useCache=flags.Acts.useCache))
        
    if flags.Acts.useCache:
        if flags.Detector.EnableITkPixel:
            acc.merge(ActsPixelClusterPreparationAlgCfg(flags,
                                                        name="ActsPixelClusterPreparationAlg",
                                                        RoIs=RoIs,
                                                        useCache=True,
                                                        OutputCollection="ITkPixelClusters_InView",
                                                        InputIDC="ActsPixelClustersCache"))
            
        if flags.Detector.EnableITkStrip:
            acc.merge(ActsStripClusterPreparationAlgCfg(flags,
                                                        name="ActsStripClusterCachePreparationAlg",
                                                        RoIs=RoIs,
                                                        useCache=True,
                                                        OutputCollection="ITkStripClusters_InView",
                                                        InputIDC="ActsStripClustersCache"))
            
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

    if flags.Acts.useCache:
        acc.merge(ActsStripClusterizationAlgCfg(flags,
                                                name="ActsConversionStripClusterizationAlg",
                                                ClustersKey="ITkConversionStripClusters",
                                                useCache=False,
                                                RoIs="ActsConversionRegionOfInterest"))
        
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterPreparationAlgCfg(flags,
                                                    name="ActsConversionStripClusterPreparationAlg" if not flags.Acts.useCache else "ActsConversionStripClusterCachePreparationAlg",
                                                    RoIs="ActsConversionRegionOfInterest",
                                                    useCache=flags.Acts.useCache,
                                                    InputCollection="ITkStripClusters",
                                                    OutputCollection="ITkConversionStripClusters" if not flags.Acts.useCache else "ITkConversionStripClusters_InView",
                                                    InputIDC="ActsStripClustersCache"))

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
        acc.merge(ActsMainClusterizationCfg(flags,
                                            RoIs = f"{flags.Tracking.ActiveConfig.extension}RegionOfInterest"))

    return acc
