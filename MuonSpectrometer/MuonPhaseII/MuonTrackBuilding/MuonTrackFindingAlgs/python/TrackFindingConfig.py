# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SegmentSelectorCfg(flags, name="SegmentSelectionTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonR4.SegmentSelectionTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


def MSExtrapolatorCfg(flags, name="MsExtrapolationTool", **kwargs):
    kwargs.setdefault("MaxSteps", 10000)
    kwargs.setdefault("InteractionEloss", flags.Acts.TrackingGeometry.MuonPassiveMaterial)
    kwargs.setdefault("InteractionMultiScatering", flags.Acts.TrackingGeometry.MuonPassiveMaterial)
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    return ActsExtrapolationToolCfg(flags, name, **kwargs)


def MSTrackFitterCfg(flags, name="MSTrackFitTool", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg, \
                                                          TruthSpacePointCalibratorCfg
    from ActsConfig.ActsConfigFlags import TrackFitterType
    if False and flags.Input.isMC:
        kwargs.setdefault("MuonCalibrationTool", result.popToolsAndMerge(TruthSpacePointCalibratorCfg(flags)))
    kwargs.setdefault("MuonCalibrationTool", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
   
    if flags.Muon.TrackFitterType == TrackFitterType.GlobalChiSquareFitter:
        kwargs.setdefault("doScattering", flags.Acts.TrackingGeometry.MuonPassiveMaterial)
        kwargs.setdefault("doEnergyLoss",  flags.Acts.TrackingGeometry.MuonPassiveMaterial)
        kwargs.setdefault("maxSteps", 1000000)
        kwargs.setdefault("maxSurfaces", 10000000)
        kwargs.setdefault("nIterMax", 30)
        the_tool = CompFactory.MuonR4.FitterTool(name, **kwargs)
        result.setPrivateTools(the_tool)
    elif flags.Muon.TrackFitterType == TrackFitterType.KalmanFitter:
        kwargs.setdefault("UseDirectNavigation", False)
        kwargs.setdefault("fitterKind", flags.Muon.TrackFitterType)
        fitTool = result.popToolsAndMerge(ActsFitterCfg(flags, name=name, **kwargs))
        result.setPrivateTools(fitTool)
    return result

def TrackSummaryToolCfg(flags, name="MuonTrackSummaryTool", **kwargs) :
    result = ComponentAccumulator()
    theTool = CompFactory.MuonR4.TrackSummaryTool(name, **kwargs)
    result.setPrivateTools(theTool)
    return result

def TrackSummaryLockCfg(flags,inContainer="", fillHoles = True, fillOutliers = True, **kwargs):
    
    summaryDecors = [ "innerSmallHits", "innerLargeHits", 
                     "middleSmallHits", "middleLargeHits", 
                     "outerSmallHits", "outerLargeHits", 
                     "extendedSmallHits", "extendedLargeHits",  
                     "innerTriggerEtaHits", "innerTriggerPhiHits", 
                     "middleTriggerEtaHits", "middleTriggerPhiHits", 
                     "outerTriggerEtaHits", "outerTriggerPhiHits"]
    if fillHoles: 
        summaryDecors +=["innerSmallHoles", "innerLargeHoles", 
                         "middleSmallHoles", "middleLargeHoles", 
                         "outerSmallHoles", "outerLargeHoles",
                         "extendedSmallHoles", "extendedLargeHoles",
                         "innerTriggerEtaHoles", "innerTriggerPhiHoles", 
                         "middleTriggerEtaHoles", "middleTriggerPhiHoles", 
                         "outerTriggerEtaHoles", "outerTriggerPhiHoles" ]
    if fillOutliers: 
        summaryDecors += ["innerClosePrecisionHits", "middleClosePrecisionHits", 
                           "outerClosePrecisionHits", "extendedClosePrecisionHits"]

    
    result = ComponentAccumulator()
    kwargs.setdefault("Decorations", [f"{inContainer}.{decor}" for decor in summaryDecors])                
    the_alg= CompFactory.DerivationFramework.LockDecorations(name=f"MuonTrackSummaryLockAlg_{inContainer}", **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def TruthTrackSeedingToolCfg(flags, name = "TruthTrackSeedingTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonR4.TruthTrackSeederTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MsTrackSeedingToolCfg(flags, name="MsTrackSeedingTool", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))

    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    the_tool = CompFactory.MuonR4.MsTrackSeederTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MSTrackFinderAlgCfg(flags, name="MSTrackFinderAlg", **kwargs):
    result = ComponentAccumulator()
    if not flags.Acts.TrackingGeometry.UseBlueprint:
        raise RuntimeError("Cannot setup the track finding alg with Gen 1 geometry")
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
    if False and flags.Input.isMC:
        kwargs.setdefault("SeedingTool", result.popToolsAndMerge(TruthTrackSeedingToolCfg(flags)))
    kwargs.setdefault("SeedingTool", result.popToolsAndMerge(MsTrackSeedingToolCfg(flags)))
    kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags)))       
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    kwargs.setdefault("expressAtMsEntrance",  flags.Muon.expressMsTrackAtEntrance)
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    the_alg = CompFactory.MuonR4.MsTrackFindingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def StandaloneTrackPartCnvCfg(flags, name="MuonMsTrackParticleCnvR4", **kwargs):
    from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvAlgCfg
    kwargs.setdefault("BeamSpotKey", "")
    kwargs.setdefault("VertexContainerKey", "")
    kwargs.setdefault("ACTSTracksLocation" ,["MuonMsTracks"])
    kwargs.setdefault("TrackParticlesOutKey", "MuonMsTrackParticles")
    kwargs.setdefault("PerigeeExpression", "DontRecalculate")
    return ActsTrackToTrackParticleCnvAlgCfg(flags, name=name, **kwargs)

def MuonActsToTrkConvCfg(flags, name="MuonActsToTrkConverterAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('TrackParticles', "MuonMsTrackParticles")
    kwargs.setdefault('OutTrackContainer', 'MsTracksTrkCnv')
    from ActsConfig.ActsEventCnvConfig import xAODtoTrkConverterAlgCfg
    result.merge(xAODtoTrkConverterAlgCfg(flags, name=name, 
                                          setupMuon = True, setupITk=False, **kwargs))
    return result 


