# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
from InDetConfig.SiClusterizationToolConfig import ITkClusterMakerToolCfg, ITkPixelRDOToolCfg
from SiLorentzAngleTool.ITkStripLorentzAngleConfig import ITkStripLorentzAngleToolCfg

def ActsITkPixelClusteringToolCfg(flags, name="ActsITkPixelClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("PixelRDOTool", acc.popToolsAndMerge(ITkPixelRDOToolCfg(flags)))
    kwargs.setdefault("ClusterMakerTool", acc.popToolsAndMerge(ITkClusterMakerToolCfg(flags)))
    kwargs.setdefault("AddCorners", True)
    kwargs.setdefault("ErrorStrategy", 1)
    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelOfflineCalibData", "")
    acc.setPrivateTools(CompFactory.ActsTrk.PixelClusteringTool(name, **kwargs))
    return acc


def ActsITkStripClusteringToolCfg(flags, name="ActsITkStripClusteringTool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("StripConditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkStripLorentzAngleToolCfg(flags)))
    kwargs.setdefault("conditionsTool", acc.popToolsAndMerge(ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault("checkBadModules", True)
    # Disable noisy modules suppression
    kwargs.setdefault("maxFiredStrips", 0)

    if flags.ITk.selectStripIntimeHits:
        from AthenaConfiguration.Enums import BeamType
        coll_25ns = flags.Beam.BunchSpacing<=25 and flags.Beam.Type is BeamType.Collisions
        kwargs.setdefault("timeBins", "01X" if coll_25ns else "X1X")

    acc.setPrivateTools(CompFactory.ActsTrk.StripClusteringTool(name, **kwargs))
    return acc

def ActsITkPixelClusterizationAlgCfg(flags, 
                                     name: str = 'ActsTrkITkPixelClusterizationAlg', 
                                     **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SiDetectorElementCollectionKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("expectedClustersPerRDO", 32)
    kwargs.setdefault("IDHelper", "PixelID")
    kwargs.setdefault("RDOContainerKey", "ITkPixelRDOs")
    kwargs.setdefault("ClustersKey", "ITkPixelClusters")
    # Regional selection
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkPixel_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkPixel_Cfg(flags)))

    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsITkPixelClusteringToolCfg(flags)))

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsITkPixelClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkPixelClusterizationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.PixelClusterizationAlg(name, **kwargs))
    return acc

def ActsITkStripClusterizationAlgCfg(flags, 
                                     name: str = 'ActsTrkITkStripClusterizationAlg', 
                                     **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("RDOContainerKey", "ITkStripRDOs")
    kwargs.setdefault("ClustersKey", "ITkStripClusters")
    kwargs.setdefault("SiDetectorElementCollectionKey", "ITkStripDetectorElementCollection")
    if 'ClusteringTool' not in kwargs:
        kwargs.setdefault("ClusteringTool", acc.popToolsAndMerge(ActsITkStripClusteringToolCfg(flags)))
    kwargs.setdefault("expectedClustersPerRDO", 6)
    kwargs.setdefault("IDHelper", "SCT_ID")
    # Regional selection
    kwargs.setdefault('RoIs', 'OfflineFullScanRegion')

    if 'RegSelTool' not in kwargs:
        from RegionSelector.RegSelToolConfig import regSelTool_ITkStrip_Cfg
        kwargs.setdefault('RegSelTool', acc.popToolsAndMerge(regSelTool_ITkStrip_Cfg(flags)))

    if flags.Acts.doMonitoring:
        from ActsConfig.ActsMonitoringConfig import ActsITkStripClusterizationMonitoringToolCfg
        kwargs.setdefault('MonTool', acc.popToolsAndMerge(ActsITkStripClusterizationMonitoringToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.StripClusterizationAlg(name, **kwargs))
    return acc

def ActsClusterizationCfg(flags):
    acc = ComponentAccumulator()

    if flags.Detector.EnableITkPixel:
        acc.merge(ActsITkPixelClusterizationAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsITkStripClusterizationAlgCfg(flags))

    if flags.Acts.doAnalysis:
        from ActsConfig.ActsAnalysisConfig import ActsClusterAnalysisCfg
        acc.merge(ActsClusterAnalysisCfg(flags))

    return acc
