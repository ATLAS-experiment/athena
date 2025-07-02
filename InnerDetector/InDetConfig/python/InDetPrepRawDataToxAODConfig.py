# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetPrepRawDataToxAOD package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TruthParticleIndexDecoratorAlgCfg(flags, name='TruthParticleIndexDecoratorAlg', **kwargs) :
    acc = ComponentAccumulator()
    kwargs.setdefault('TruthParticleIndexDecoration', 'origTruthIndex')
    acc.addEventAlgo(CompFactory.TruthParticleIndexDecoratorAlg(name, **kwargs))
    return acc

def InDetPixelPrepDataToxAODCfg(flags, name='InDetPixelPrepDataToxAOD', **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    acc = PixelReadoutGeometryCfg(flags)

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelChargeCalibCondCfg, PixelDCSCondStateAlgCfg, PixelDCSCondStatusAlgCfg, PixelDCSCondTempAlgCfg, PixelDCSCondHVAlgCfg
    acc.merge(PixelChargeCalibCondCfg(flags))
    acc.merge(PixelDCSCondStateAlgCfg(flags))
    acc.merge(PixelDCSCondStatusAlgCfg(flags))
    acc.merge(PixelDCSCondTempAlgCfg(flags))
    acc.merge(PixelDCSCondHVAlgCfg(flags))

    from InDetConfig.PixelCalibAlgsConfig import PixelChargeToTConversionCfg
    acc.merge(PixelChargeToTConversionCfg(flags))

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.PixelConditionsSummaryConfig import PixelConditionsSummaryCfg
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(PixelConditionsSummaryCfg(flags)))

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags)))

    kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    if flags.Input.isMC:
        # need to decorate truth particles and clusters with same unique identified
        # which is the origin truth particle index
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )
    kwargs.setdefault("InputTruthParticleLinks", "xAODTruthLinks")
    kwargs.setdefault("WriteExtendedPRDinformation", True)

    acc.addEventAlgo(CompFactory.PixelPrepDataToxAOD(name, **kwargs))
    return acc


def ITkActsPrepDataToxAODCfg(flags,
                             *,
                             PixelClusterContainer: str = "ITkPixelClusters",
                             StripClusterContainer: str = "ITkStripClusters",
                             PixelMeasurementContainer: str = "ITkPixelMeasurements",
                             StripMeasurementContainer: str = "ITkStripMeasurements",
                             TrackParticles: list[str] = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # need to decorate truth particles and clusters with same unique identified
    # which is the origin truth particle index
    if not flags.Input.isMC:
        return acc

    acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

    from ActsConfig.ActsObjectDecorationConfig import ActsPixelClusterTruthDecoratorAlgCfg,ActsStripClusterTruthDecoratorAlgCfg
    acc.merge(ActsPixelClusterTruthDecoratorAlgCfg(flags,
                                                   ClusterContainer = PixelClusterContainer,
                                                   MeasurementContainer = PixelMeasurementContainer,
                                                   TrackParticles = TrackParticles))
    acc.merge(ActsStripClusterTruthDecoratorAlgCfg(flags,
                                                   ClusterContainer = StripClusterContainer,
                                                   MeasurementContainer = StripMeasurementContainer,
                                                   TrackParticles = TrackParticles))

    return acc

def InDetPixelPrepDataToxAOD_ExtraTruthCfg(flags, name='InDetPixelPrepDataToxAOD_ExtraTruth', **kwargs):
    kwargs.setdefault("WriteSDOs", True)
    kwargs.setdefault("WriteSiHits", True)
    return InDetPixelPrepDataToxAODCfg(flags, name, **kwargs)

def ITkPixelPrepDataToxAODCfg(flags, name='ITkPixelPrepDataToxAOD', **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc = ITkPixelReadoutGeometryCfg(flags)

    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelChargeCalibCondAlgCfg, ITkPixelDCSCondStateAlgCfg, ITkPixelDCSCondStatusAlgCfg, ITkPixelDCSCondTempAlgCfg, ITkPixelDCSCondHVAlgCfg
    acc.merge(ITkPixelChargeCalibCondAlgCfg(flags))
    acc.merge(ITkPixelDCSCondStateAlgCfg(flags))
    acc.merge(ITkPixelDCSCondStatusAlgCfg(flags))
    acc.merge(ITkPixelDCSCondTempAlgCfg(flags))
    acc.merge(ITkPixelDCSCondHVAlgCfg(flags))
    
    from TrkConfig.TrkConfigFlags import TrackingComponent
    if TrackingComponent.AthenaChain in flags.Tracking.recoChain:
        from InDetConfig.PixelCalibAlgsConfig import ITkPixelChargeToTConversionCfg
        acc.merge(ITkPixelChargeToTConversionCfg(flags))

    if "PixelConditionsSummaryTool" not in kwargs:
        from PixelConditionsTools.ITkPixelConditionsSummaryConfig import ITkPixelConditionsSummaryCfg
        kwargs.setdefault("PixelConditionsSummaryTool", acc.popToolsAndMerge(ITkPixelConditionsSummaryCfg(flags)))

    if "LorentzAngleTool" not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import ITkPixelLorentzAngleToolCfg
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags)))

    kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    if flags.Input.isMC:
        # need to decorate truth particles and clusters with same unique identified
        # which is the origin truth particle index
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

    from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
    kwargs.setdefault("PixelReadoutManager", acc.getPrimaryAndMerge(ITkPixelReadoutManagerCfg(flags)))

    kwargs.setdefault("WriteExtendedPRDinformation", True)
    kwargs.setdefault("PixelChargeCalibCondData", "ITkPixelChargeCalibCondData")
    kwargs.setdefault("PixelDCSStateCondData", "ITkPixelDCSStateCondData")
    kwargs.setdefault("PixelDCSStatusCondData", "ITkPixelDCSStatusCondData")
    kwargs.setdefault("ReadKeyTemp", "ITkPixelDCSTempCondData")
    kwargs.setdefault("ReadKeyHV", "ITkPixelDCSHVCondData")    
    kwargs.setdefault("SiClusterContainer", "ITkPixelClusters")
    kwargs.setdefault("MC_SDOs", "ITkPixelSDO_Map")
    kwargs.setdefault("MC_Hits", "ITkPixelHits")
    kwargs.setdefault("PRD_MultiTruth", "PRD_MultiTruthITkPixel")
    kwargs.setdefault("InputTruthParticleLinks", "xAODTruthLinks")
    kwargs.setdefault("OutputClusterContainer", "ITkPixelMeasurements")

    acc.addEventAlgo(CompFactory.PixelPrepDataToxAOD(name, **kwargs))
    return acc

def ITkPixelPrepDataToxAOD_ExtraTruthCfg(flags, name='ITkPixelPrepDataToxAOD_ExtraTruth', **kwargs):
    kwargs.setdefault("WriteSDOs", True)
    kwargs.setdefault("WriteSiHits", True)
    return ITkPixelPrepDataToxAODCfg(flags, name, **kwargs)
    
def InDetSCT_PrepDataToxAODCfg(flags, name='InDetSCTPrepDataToxAOD', **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)
    kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    if flags.Input.isMC:
        # need to decorate truth particles and clusters with same unique identified
        # which is the origin truth particle index
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )
    kwargs.setdefault("InputTruthParticleLinks", "xAODTruthLinks")
    acc.addEventAlgo(CompFactory.SCT_PrepDataToxAOD(name, **kwargs))
    return acc

def InDetSCT_RawDataToxAODCfg(flags, name='InDetSCTRawDataToxAOD', **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)
    acc.addEventAlgo(CompFactory.SCT_RawDataToxAOD(name, **kwargs))
    return acc

def InDetSCT_PrepDataToxAOD_ExtraTruthCfg(flags, name='InDetSCTPrepDataToxAOD_ExtraTruth', **kwargs):
    kwargs.setdefault("WriteSDOs", True)
    kwargs.setdefault("WriteSiHits", True)
    return InDetSCT_PrepDataToxAODCfg(flags, name, **kwargs)


def ITkStripPrepDataToxAODCfg(flags, name='ITkStripPrepDataToxAOD', **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("SiClusterContainer", "ITkStripClusters")
    kwargs.setdefault("MC_SDOs", "ITkStripSDO_Map")
    kwargs.setdefault("MC_Hits", "ITkStripHits")
    kwargs.setdefault("PRD_MultiTruth", "PRD_MultiTruthITkStrip")
    kwargs.setdefault("InputTruthParticleLinks", "xAODTruthLinks")
    kwargs.setdefault("SctRdoContainer", "ITkStripRDOs")
    kwargs.setdefault("SctxAodContainer", "ITkStripMeasurements")
    kwargs.setdefault("SctxAodOffset", "ITkStripClustersOffsets")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")
    kwargs.setdefault("UseTruthInfo", flags.Input.isMC)
    if flags.Input.isMC:
        # need to decorate truth particles and clusters with same unique identified
        # which is the origin truth particle index
        acc.merge( TruthParticleIndexDecoratorAlgCfg(flags) )

    acc.addEventAlgo(CompFactory.SCT_PrepDataToxAOD(name, **kwargs))
    return acc

def ITkStripPrepDataToxAOD_ExtraTruthCfg(flags, name='ITkStripPrepDataToxAOD_ExtraTruth', **kwargs):
    kwargs.setdefault("WriteSDOs", True)
    kwargs.setdefault("WriteSiHits", True)
    return ITkStripPrepDataToxAODCfg(flags, name, **kwargs)

def InDetTRT_PrepDataToxAODCfg(flags, name='InDetTRTPrepDataToxAOD', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UseTruthInfo", flags.Input.isMC)

    if "TRTCalDbTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_CalDbToolCfg)
        kwargs.setdefault("TRTCalDbTool", acc.popToolsAndMerge(
            TRT_CalDbToolCfg(flags)))

    if "TRTStrawSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_StrawStatusSummaryToolCfg)
        kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(
            TRT_StrawStatusSummaryToolCfg(flags)))

    if "NeighbourSvc" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
            TRT_StrawNeighbourSvcCfg)
        kwargs.setdefault("NeighbourSvc", acc.getPrimaryAndMerge(
            TRT_StrawNeighbourSvcCfg(flags)))

    if "TRTDriftFunctionTool" not in kwargs:
        from InDetConfig.TRT_DriftFunctionToolConfig import (
            TRT_DriftFunctionToolCfg)
        kwargs.setdefault("TRTDriftFunctionTool", acc.popToolsAndMerge(
            TRT_DriftFunctionToolCfg(flags)))

    acc.addEventAlgo(CompFactory.TRT_PrepDataToxAOD(name, **kwargs))
    return acc

def InDetTRT_PrepDataToxAOD_ExtraTruthCfg(flags, name='InDetTRTPrepDataToxAOD_ExtraTruth', **kwargs):
    kwargs.setdefault("WriteSDOs", True)
    return InDetTRT_PrepDataToxAODCfg(flags, name, **kwargs)

def InDetPrepDataToxAODCfg(flags):
    acc = ComponentAccumulator()

    if flags.Detector.EnablePixel:
        from InDetConfig.TrackRecoConfig import (
            ClusterSplitProbabilityContainerName)
        acc.merge(InDetPixelPrepDataToxAODCfg(
            flags, ClusterSplitProbabilityName=(
                ClusterSplitProbabilityContainerName(flags))))

    if flags.Detector.EnableSCT:
        acc.merge(InDetSCT_PrepDataToxAODCfg(flags))

    if flags.Detector.EnableTRT:
        acc.merge(InDetTRT_PrepDataToxAODCfg(flags))

    return acc

def ITkPrepDataToxAODCfg(flags):
    from InDetConfig.ITkActsHelpers import primaryPassUsesActs
    if primaryPassUsesActs(flags):
        return ITkActsPrepDataToxAODCfg(flags)

    acc = ComponentAccumulator()
    extra_truth = flags.Tracking.doTIDE_AmbiTrackMonitoring
    if flags.Detector.EnableITkPixel:
        from InDetConfig.ITkTrackRecoConfig import (
            ITkClusterSplitProbabilityContainerName)
        if extra_truth :
           acc.merge(ITkPixelPrepDataToxAOD_ExtraTruthCfg(
              flags, ClusterSplitProbabilityName=(
                  ITkClusterSplitProbabilityContainerName(flags))))
        else :
           acc.merge(ITkPixelPrepDataToxAODCfg(
              flags, ClusterSplitProbabilityName=(
                  ITkClusterSplitProbabilityContainerName(flags))))

    if flags.Detector.EnableITkStrip:
        if extra_truth :
            acc.merge(ITkStripPrepDataToxAOD_ExtraTruthCfg(flags))
        else :
            acc.merge(ITkStripPrepDataToxAODCfg(flags))

    return acc
