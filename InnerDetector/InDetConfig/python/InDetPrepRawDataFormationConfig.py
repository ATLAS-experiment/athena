# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPrepRawDataFormation package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from AthenaConfiguration.Enums import ProductionStep


def clusterizationInputPrefix(flags):
    """Return clusterization input prefix based on the production step and tracking config"""
    if hasattr(flags.TrackOverlay, "ActiveConfig"):
       doTrackOverlay = getattr(flags.TrackOverlay.ActiveConfig, "doTrackOverlay", None)
    else:
       doTrackOverlay = flags.Overlay.doTrackOverlay

    if doTrackOverlay:
        prefix = flags.Overlay.SigPrefix
    elif flags.Common.ProductionStep in [ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
        prefix = flags.Overlay.BkgPrefix
    else:
        prefix = ''

    return prefix


def HGTDInDetToXAODClusterConversionCfg(flags, name="HGTDInDetToXAODClusterConversion", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault('ProcessHgtd', True)
    acc.addEventAlgo(CompFactory.InDet.InDetToXAODClusterConversion(name, **kwargs))
    return acc

def HGTDXAODToInDetClusterConversionCfg(flags, name="HGTDXAODToInDetClusterConversion", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault('ProcessHgtd', True)
    acc.addEventAlgo(CompFactory.InDet.XAODToInDetClusterConversion(name, **kwargs))
    return acc


def ITkInDetToXAODClusterConversionCfg(flags, name="ITkInDetToXAODClusterConversion", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault('ProcessPixel', flags.Detector.EnableITkPixel)
    kwargs.setdefault('ProcessStrip', flags.Detector.EnableITkStrip)
    acc.addEventAlgo(CompFactory.InDet.InDetToXAODClusterConversion(name, **kwargs))
    return acc


def ITkXAODToInDetClusterConversionCfg(flags, name="ITkXAODToInDetClusterConversion", **kwargs):
    acc = ComponentAccumulator()

    from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)) )

    kwargs.setdefault('ProcessPixel', flags.Detector.EnableITkPixel)
    kwargs.setdefault('ProcessStrip', flags.Detector.EnableITkStrip)

    acc.addEventAlgo(CompFactory.InDet.XAODToInDetClusterConversion(name, **kwargs))
    return acc


def PixelClusterizationCfg(flags, name = "InDetPixelClusterization", **kwargs):
    acc = ComponentAccumulator()
    
    prefix = clusterizationInputPrefix(flags)

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import MergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            MergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import PixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(
            PixelGangedAmbiguitiesFinderCfg(flags)))

    kwargs.setdefault("DataObjectName", prefix + "PixelRDOs")
    kwargs.setdefault("ClustersName", "PixelClusters")

    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(prefix+name, **kwargs))
    return acc


def PixelClusterizationPUCfg(flags, name="InDetPixelClusterizationPU", **kwargs):
    kwargs.setdefault("DataObjectName", "Pixel_PU_RDOs")
    kwargs.setdefault("ClustersName", "PixelPUClusters")
    kwargs.setdefault("AmbiguitiesMap", "PixelClusterAmbiguitiesMapPU")
    return PixelClusterizationCfg(flags, name, **kwargs)


def TrigPixelClusterizationCfg(flags, RoIs, name="InDetPixelClusterization", **kwargs):
    acc = ComponentAccumulator()
   
    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_Pixel_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_Pixel_Cfg(flags)))

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import TrigMergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            TrigMergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import PixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(
            PixelGangedAmbiguitiesFinderCfg(flags)))

    kwargs.setdefault("AmbiguitiesMap", flags.Trigger.InDetTracking.ClusterAmbiguitiesMap)
    kwargs.setdefault("ClustersName", "PixelTrigClusters")
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", RoIs)
    kwargs.setdefault("ClusterContainerCacheKey", flags.Trigger.InDetTracking.PixelClusterCacheKey)
    kwargs.setdefault("useDataPoolWithCache", True)
    kwargs.setdefault("name", f"{name}_{RoIs}")
    
    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(**kwargs))
    return acc


def ITkPixelClusterizationCfg(flags, name = "ITkPixelClusterization", **kwargs):
    acc = ComponentAccumulator()

    prefix = clusterizationInputPrefix(flags)

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkMergedPixelsToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            ITkMergedPixelsToolCfg(flags)))

    if "gangedAmbiguitiesFinder" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITkPixelGangedAmbiguitiesFinderCfg
        kwargs.setdefault("gangedAmbiguitiesFinder", acc.popToolsAndMerge(ITkPixelGangedAmbiguitiesFinderCfg(flags)))
    kwargs.setdefault("DataObjectName", prefix + "ITkPixelRDOs")
    kwargs.setdefault("ClustersName", "ITkPixelClusters")
    kwargs.setdefault("AmbiguitiesMap", "ITkPixelClusterAmbiguitiesMap")

    acc.addEventAlgo(CompFactory.InDet.PixelClusterization(prefix+name, **kwargs))
    return acc


def ITkTrigPixelClusterizationCfg(flags, name = "ITkTrigPixelClusterization", roisKey="", signature="", **kwargs):
    acc = ComponentAccumulator()
    from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
    acc.merge(ITkPixelClusterizationCfg(flags,
                                        name="ITkPixelClusterization_"+signature,
                                        isRoI_Seeded=True,
                                        RoIs=roisKey,
                                        ClustersName = "ITkTrigPixelClusters",
                                        ClusterContainerCacheKey=flags.Trigger.ITkTracking.PixelClusterCacheKey,
                                        AmbiguitiesMap = flags.Trigger.ITkTracking.ClusterAmbiguitiesMap,
                                        RegSelTool= acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags))))
    return acc


def SCTClusterizationCfg(flags, name="InDetSCT_Clusterization", **kwargs):
    acc = ComponentAccumulator()
    
    prefix = clusterizationInputPrefix(flags)

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False)))

    if "SCTDetElStatus" not in kwargs :
        from SCT_ConditionsAlgorithms.SCT_ConditionsAlgorithmsConfig import SCT_DetectorElementStatusAlgWithoutFlaggedCfg
        acc.merge( SCT_DetectorElementStatusAlgWithoutFlaggedCfg(flags) )
        kwargs.setdefault("SCTDetElStatus", "SCTDetectorElementStatusWithoutFlagged" )

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            SCT_ClusteringToolCfg(flags)))

    kwargs.setdefault("DataObjectName", prefix + "SCT_RDOs")
    kwargs.setdefault("ClustersName", 'SCT_Clusters')

    acc.addEventAlgo(CompFactory.InDet.SCT_Clusterization(prefix+name, **kwargs))
    return acc


def SCTClusterizationPUCfg(flags, name="InDetSCT_ClusterizationPU", **kwargs):
    kwargs.setdefault("DataObjectName", "SCT_PU_RDOs" )
    kwargs.setdefault("ClustersName", "SCT_PU_Clusters")
    return SCTClusterizationCfg(flags, name, **kwargs)


def TrigSCTClusterizationCfg(flags, RoIs, name="InDetSCT_Clusterization", **kwargs):
    acc = ComponentAccumulator()

    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_SCT_Cfg(flags)))

    if "conditionsTool" not in kwargs:
        from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
        kwargs.setdefault("conditionsTool",  acc.popToolsAndMerge(
            SCT_ConditionsSummaryToolCfg(flags, withFlaggedCondTool=False, withTdaqTool=False)))

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import Trig_SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            Trig_SCT_ClusteringToolCfg(flags)))

    kwargs.setdefault("DataObjectName", 'SCT_RDOs')
    kwargs.setdefault("ClustersName", 'SCT_TrigClusters')
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", RoIs)
    kwargs.setdefault("ClusterContainerCacheKey", flags.Trigger.InDetTracking.SCTClusterCacheKey)
    kwargs.setdefault("FlaggedCondCacheKey", "")
    kwargs.setdefault("useDataPoolWithCache", True)
    kwargs.setdefault("name", f"{name}_{RoIs}")
    
    acc.addEventAlgo(CompFactory.InDet.SCT_Clusterization(**kwargs))
    return acc


def ITkStripClusterizationCfg(flags, name="ITkStripClusterization", **kwargs):
    acc = ComponentAccumulator()
    
    prefix = clusterizationInputPrefix(flags)

    kwargs.setdefault("conditionsTool",None) # SCTDetElStatus is used instead
    if "SCTDetElStatus" not in kwargs :
        if not flags.Trigger.doHLT :
            from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import  (
                ITkStripDetectorElementStatusAlgCfg)
            acc.merge(ITkStripDetectorElementStatusAlgCfg(flags))
        kwargs.setdefault("SCTDetElStatus", "ITkStripDetectorElementStatus")

    if "clusteringTool" not in kwargs:
        from InDetConfig.SiClusterizationToolConfig import ITKStrip_SCT_ClusteringToolCfg
        kwargs.setdefault("clusteringTool", acc.popToolsAndMerge(
            ITKStrip_SCT_ClusteringToolCfg(flags)))
    kwargs.setdefault("DataObjectName", prefix + 'ITkStripRDOs')
    kwargs.setdefault("ClustersName", 'ITkStripClusters')
    kwargs.setdefault("SCT_FlaggedCondData", "ITkStripFlaggedCondData")
    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)

    acc.addEventAlgo( CompFactory.InDet.SCT_Clusterization(prefix+name, **kwargs))
    return acc


def ITkTrigStripClusterizationCfg(flags, name="ITkTrigStripClusterization", roisKey="", signature="", **kwargs):
    acc = ComponentAccumulator()
    from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
    acc.merge(ITkStripClusterizationCfg(flags,
                                        name="ITkStripClusterization_"+signature,
                                        isRoI_Seeded=True,
                                        RoIs=roisKey,
                                        ClustersName = "ITkTrigStripClusters",
                                        ClusterContainerCacheKey=flags.Trigger.ITkTracking.SCTClusterCacheKey,
                                        RegSelTool= acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags))))
    return acc


def InDetTRT_RIO_MakerCfg(flags, name = "InDetTRT_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    # track overlay always uses full input container here
    if flags.Common.ProductionStep in [ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
        prefix = flags.Overlay.BkgPrefix
    else:
        prefix = ''

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_DriftCircleToolCfg(flags)))
    kwargs.setdefault("TRTRIOLocation", 'TRT_DriftCircles')
    #Backtracking is disable during the processing of pileup tracks,
    #the standard TRT_RDOs are used for track overlay when overlaying with HS RDOs.
    kwargs.setdefault("TRTRDOLocation", prefix + 'TRT_RDOs')

    acc.addEventAlgo(CompFactory.InDet.TRT_RIO_Maker(prefix+name, **kwargs))
    return acc


def InDetTRT_NoTime_RIO_MakerCfg(flags, name = "InDetTRT_NoTime_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_NoTime_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_NoTime_DriftCircleToolCfg(flags)))

    kwargs.setdefault("TRTRIOLocation", 'TRT_DriftCirclesUncalibrated')

    acc.merge(InDetTRT_RIO_MakerCfg(flags, name, **kwargs))
    return acc


def InDetTRT_Phase_RIO_MakerCfg(flags, name = "InDetTRT_Phase_RIO_Maker", **kwargs):
    acc = ComponentAccumulator()

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_Phase_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_Phase_DriftCircleToolCfg(flags)))

    acc.merge(InDetTRT_RIO_MakerCfg(flags, name, **kwargs))
    return acc


def InDetTRT_RIO_MakerPUCfg(flags, name = "InDetTRT_RIO_MakerPU", **kwargs):
    kwargs.setdefault("TRTRDOLocation", 'TRT_PU_RDOs')    
    kwargs.setdefault("TRTRIOLocation", 'TRT_PU_DriftCircles')
    return InDetTRT_RIO_MakerCfg(flags, name, **kwargs)


def TrigTRTRIOMakerCfg(flags, RoIs, name="InDetTrigMTTRTDriftCircleMaker", **kwargs):
    acc = ComponentAccumulator()

    if "RegSelTool" not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_TRT_Cfg
        kwargs.setdefault("RegSelTool", acc.popToolsAndMerge(
            regSelTool_TRT_Cfg(flags)))

    if "TRT_DriftCircleTool" not in kwargs:
        from InDetConfig.TRT_DriftCircleToolConfig import TRT_DriftCircleToolCfg
        kwargs.setdefault("TRT_DriftCircleTool", acc.popToolsAndMerge(
            TRT_DriftCircleToolCfg(flags)))

    kwargs.setdefault("TRTRIOLocation", "TRT_TrigDriftCircles")
    kwargs.setdefault("TRTRDOLocation", "TRT_RDOs_TRIG" if flags.Input.Format is Format.BS else "TRT_RDOs")
    kwargs.setdefault("isRoI_Seeded", True)
    kwargs.setdefault("RoIs", RoIs)
    
    kwargs.setdefault("TRT_DriftCircleCache", flags.Trigger.InDetTracking.TRT_DriftCircleCacheKey)
    kwargs.setdefault("useDataPoolWithCache", True)

    kwargs.setdefault("name", f"{name}_{RoIs}")
    acc.addEventAlgo(CompFactory.InDet.TRT_RIO_Maker(**kwargs))
    return acc


def AthenaTrkClusterizationCfg(flags):
    acc = ComponentAccumulator()
    #
    # -- Pixel Clusterization
    #
    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelClusterizationCfg(flags))
    #
    # --- Strip Clusterization
    #
    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripClusterizationCfg(flags))

    return acc
