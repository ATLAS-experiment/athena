# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SegmentSelectorCfg(flags, name="SegmentSelectionTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonR4.SegmentSelectionTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MSTrackFitterCfg(flags, name="MSTrackFitTool", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsConfigFlags import TrackFitterType
    from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault("ATLASConverterTool", result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags, setupMuon = True)))
    kwargs.setdefault("fitterKind", TrackFitterType.GlobalChiSquareFitter)
    kwargs.setdefault("OutlierChi2Cut", 200000)
    kwargs.setdefault("DoReFitFromPRD", False)
    kwargs.setdefault("IncludeScattering", flags.Muon.trackGeometryPassiveMaterial)
    kwargs.setdefault("IncludeELoss",  flags.Muon.trackGeometryPassiveMaterial)
    
    kwargs.setdefault("MaxPropagationStep", 1000000)
    kwargs.setdefault("MaxSurfacesPerNavStep", 10000000)
    kwargs.setdefault("DoFreeToBoundCorrection", True)
    kwargs.setdefault("MaxIterations", 100)
    

    kwargs.setdefault("MuonCalibrationTool",result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
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


def MSTrackFinderAlgCfg(flags, name="MSTrackFinderAlg", **kwargs):
    result = ComponentAccumulator()
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
 
    kwargs.setdefault("SegmentSelectionTool", result.popToolsAndMerge(SegmentSelectorCfg(flags)))
    kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags)))       
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, 
                                                                                            MaxSteps=10000,
                                                                                            InteractionEloss = flags.Muon.trackGeometryPassiveMaterial,
                                                                                            InteractionMultiScatering = flags.Muon.trackGeometryPassiveMaterial  )))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))
    kwargs.setdefault("SummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))

    the_alg = CompFactory.MuonR4.MsTrackFindingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def StandaloneTrackPartCnvCfg(flags, name="MuonMsTrackParticleCnvR4", **kwargs):
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    kwargs.setdefault("BeamSpotKey", "")
    kwargs.setdefault("VertexContainerKey", "")
    kwargs.setdefault("ACTSTracksLocation" ,["MsTracks"])
    kwargs.setdefault("TrackParticlesOutKey", "MsTrackParticlesR4")
    kwargs.setdefault("PerigeeExpression", "DontRecalculate")
    return ActsTrackToTrackParticleCnvAlgCfg(flags, name=name, **kwargs)

def MuonActsToTrkConvCfg(flags, name="MuonActsToTrkConverterAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('ACTSTracksLocation', "MsTracks")
    kwargs.setdefault('TracksLocation', 'MsTracksTrkCnv')
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    kwargs.setdefault("ATLASConverterTool", result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags, setupMuon = True)))
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConvertorAlgCfg
    result.merge(ActsToTrkConvertorAlgCfg(flags, name=name, **kwargs))
    return result 

def MuidSaTagMakerAlgCfg(flags, name="MuonMuidTagSaAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.StandaloneMuonTagAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def MuonInDetTrackSelectionAlgCfg(flags, name="MuonCombinedInDetCandidateAlgR4", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000, 
                                                                                             InteractionEloss = True,
                                                                                             InteractionMultiScatering = True)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.InDetTrackSelectionAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonSegmentTaggingAlgCfg(flags, name="MuonCombinedSegmentTaggingAlgR4", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.SegmentTaggingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonCreatorAlgCfg(flags, name="MuonCreatorAlgR4", **kwargs):
    result = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("SelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags)))
    from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.MuonCreatorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
