# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BeamSpotPreparatorAlgCfg(flags, name="MuonBeamSpotPreparator", **kwargs):
    result = ComponentAccumulator()
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result.merge(BeamSpotCondAlgCfg(flags))
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
    the_alg = CompFactory.MuonCombinedR4.BeamSpotPreparatorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuidSaTagMakerAlgCfg(flags, name="MuonMuidTagSaAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ExtrapolateToIP", flags.Muon.buildMETrack )
    kwargs.setdefault("RefitWithBeamSpot", flags.Muon.buildMETrack )
    kwargs.setdefault("CombinedTags", "MuonTagsMuidCo")
    from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))
    if kwargs["ExtrapolateToIP"]:
        from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFitterCfg, MSExtrapolatorCfg
        kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags)))
        kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
        from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvToolCfg
        kwargs.setdefault("TrackToTrackParticleCnvTool", 
            result.popToolsAndMerge(ActsTrackToTrackParticleCnvToolCfg(flags)))

    the_alg = CompFactory.MuonCombinedR4.StandaloneMuonTagAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonCombinedStacoAlgCfg(flags, name="MuonCombinedStacoAlgR4", **kwargs ):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonCombinedR4.CombinedStacoAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def LRT_MuonCombinedStacoAlgCfg(flags, name="MuonCombinedStacoAlgR4_LRT", **kwargs ):
    kwargs.setdefault("IdTrkKey", "MuonInDetCandidatesLRT")
    kwargs.setdefault("writeKey","MuonTagsSTACOLRT")
    kwargs.setdefault("writeTrkKey","MuonSTACOTrackParticlesLRT")
    return MuonCombinedStacoAlgCfg(flags, name, **kwargs)

def MuonLegacyCaloTagAlgCfg(flags, name = "MuonLegacyCaloTagAlgR4", **kwargs):
    result = ComponentAccumulator()
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCaloTagAlgCfg, MuonCombinedInDetCandidateAlgCfg
    result.merge(MuonCombinedInDetCandidateAlgCfg(flags))
    result.merge(MuonCaloTagAlgCfg(flags))
    the_alg = CompFactory.MuonCombinedR4.LegacyCaloTagAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def LRT_MuonLegacyCaloTagAlgCfg(flags, name = "MuonLegacyCaloTagAlgR4_LRT", **kwargs):
    result = ComponentAccumulator()
    from TrackToCalo.CaloExtensionBuilderAlgCfg  import CaloExtensionBuilderAlgLRTCfg
    from MuonCombinedConfig.MuonCombinedReconstructionConfig import LRT_MuonCaloTagAlgCfg, LRT_MuonCombinedInDetCandidateAlgCfg
    result.merge(CaloExtensionBuilderAlgLRTCfg(flags))
    result.merge(LRT_MuonCombinedInDetCandidateAlgCfg(flags))
    result.merge(LRT_MuonCaloTagAlgCfg(flags))
    kwargs.setdefault("IdTrackKey", "MuonInDetCandidatesLRT")
    kwargs.setdefault("WriteKey","LegacyCaloTagsLRT")
    kwargs.setdefault("CaloTagMap","caloTagMap_LRT")
    the_alg  = CompFactory.MuonCombinedR4.LegacyCaloTagAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonCombinedFitAlgCfg(flags, name="MuonCombinedFitAlg", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("fitMETracks", flags.Muon.buildMETrack)
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))
    from MuonTrackFindingAlgs.TrackFindingConfig import MSTrackFitterCfg, MSExtrapolatorCfg
    kwargs.setdefault("FittingTool", result.popToolsAndMerge(MSTrackFitterCfg(flags)))
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvToolCfg
    kwargs.setdefault("TrackToTrackParticleCnvTool",  result.popToolsAndMerge(ActsTrackToTrackParticleCnvToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.CombinedFitAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def LRT_MuonCombinedFitAlgCfg(flags, name = "LRT_MuonCombinedFitAlg", **kwargs):
    kwargs.setdefault("StacoKey", "MuonTagsSTACOLRT")
    kwargs.setdefault("MuonTagKey", "MuonTagsMuidCoLRT")
    kwargs.setdefault("CombinedParticleKey", "MuonCmbTrackParticlesLRT")
    kwargs.setdefault("MeParticleKey", "MuonMeTrackParticlesLRT")
    kwargs.setdefault("CombinedTrackKey", "MuonCmbTracksLRT")
    kwargs.setdefault("MeTrackKey", "MuonMeTracksLRT")
    return MuonCombinedFitAlgCfg(flags, name = name, **kwargs)

def MuonInDetTrackSelectionAlgCfg(flags, name="MuonCombinedInDetCandidateAlgR4", **kwargs):
    result = ComponentAccumulator()
    if not flags.Acts.TrackingGeometry.UseBlueprint:
        raise RuntimeError("Cannot setup the InDet Candidate selection with Gen 1 geometry")
    from MuonTrackFindingAlgs.TrackFindingConfig import MSExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))
    kwargs.setdefault("matchTracksOnSameSurface",  flags.Muon.expressMsTrackAtEntrance)
    the_alg = CompFactory.MuonCombinedR4.InDetTrackSelectionAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def LRT_MuonInDetTrackSelectionAlgCfg(flags, name="MuonCombinedInDetCandidateAlgR4_LRT", **kwargs):
    kwargs.setdefault("IdTrackKey", "InDetLargeD0TrackParticles")
    kwargs.setdefault("writeKey", "MuonInDetCandidatesLRT")
    return MuonInDetTrackSelectionAlgCfg(flags, name, **kwargs) 

def MuonSegmentTaggingAlgCfg(flags, name="MuonCombinedSegmentTaggingAlgR4", **kwargs):
    result = ComponentAccumulator()
    from MuonTrackFindingAlgs.TrackFindingConfig import MSExtrapolatorCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(MSExtrapolatorCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.SegmentTaggingAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def LRT_MuonSegmentTaggingAlgCfg(flags, name="MuonCombinedSegmentTaggingAlgR4_LRT", **kwargs):
    kwargs.setdefault("IdTrackKey", "MuonInDetCandidatesLRT")
    kwargs.setdefault("writeKey", "SegmentTagsLRT")
    return MuonSegmentTaggingAlgCfg(flags, name, **kwargs)

def MuonCreatorAlgCfg(flags, name="MuonCreatorAlgR4", **kwargs):
    result = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonLoosenedNonCalibratedSelectionToolCfg
    kwargs.setdefault("SelectionTool", result.popToolsAndMerge(MuonLoosenedNonCalibratedSelectionToolCfg(flags)))
    from MuonTrackFindingAlgs.TrackFindingConfig import TrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(TrackSummaryToolCfg(flags)))
    the_alg = CompFactory.MuonCombinedR4.MuonCreatorAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result