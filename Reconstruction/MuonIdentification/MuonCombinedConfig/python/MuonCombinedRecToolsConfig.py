# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Defines the shared tools used in muon identification
# Based on :
# https://gitlab.cern.ch/atlas/athena/blob/release/22.0.8/Reconstruction/MuonIdentification/MuonCombinedRecExample/python/MuonCombinedTools.py
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg
from MuonConfig.MuonTrackBuildingConfig import MuonSegmentRegionRecoveryToolCfg

from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasEnergyLossUpdatorCfg
from AthenaCommon.SystemOfUnits import GeV, mm


def InDetCandidateToolCfg(flags, name="InDetCandidateTool", **kwargs):
    from InDetConfig.InDetTrackSelectorToolConfig import MuonCombinedInDetDetailedTrackSelectorToolCfg
    result = MuonCombinedInDetDetailedTrackSelectorToolCfg(flags)
    kwargs.setdefault("TrackSelector", result.popPrivateTools())
    result.setPrivateTools(
        CompFactory.MuonCombined.InDetCandidateTool(name, **kwargs))
    return result


def MuonInDetForwardCandidateToolCfg(flags,  name='InDetForwardCandidateTool', **kwargs):
    from InDetConfig.InDetTrackSelectorToolConfig import MuonCombinedInDetDetailedForwardTrackSelectorToolCfg
    result = MuonCombinedInDetDetailedForwardTrackSelectorToolCfg(flags)
    kwargs.setdefault("TrackSelector", result.popPrivateTools())
    kwargs.setdefault("FlagCandidatesAsSiAssociated", True)
    result.setPrivateTools(result.popToolsAndMerge(
        InDetCandidateToolCfg(flags, name, **kwargs)))
    return result  # FIXME - is this and the above, actually used?


def MuonCaloEnergyToolCfg(flags,  name="MuonCaloEnergyTool", **kwargs):
    from TrackToCalo.TrackToCaloConfig import ParticleCaloCellAssociationToolCfg, ParticleCaloExtensionToolCfg

    result = ParticleCaloExtensionToolCfg(
        flags, name='MuonParticleCaloExtensionTool')
    particle_calo_extension_tool = result.getPrimary()

    particle_calo_cell_association_tool = result.popToolsAndMerge(
        ParticleCaloCellAssociationToolCfg(flags, name='MuonCaloCellAssociationTool', ParticleCaloExtensionTool=particle_calo_extension_tool))

    from TrkConfig.TrkParticleCreatorConfig import MuonCaloParticleCreatorCfg
    track_particle_creator = result.popToolsAndMerge(
        MuonCaloParticleCreatorCfg(flags))

    muonCaloEnergyTool = CompFactory.Rec.MuonCaloEnergyTool(name, ParticleCaloExtensionTool=particle_calo_extension_tool,
                                                            ParticleCaloCellAssociationTool=particle_calo_cell_association_tool,
                                                            TrackParticleCreator=track_particle_creator)
    result.setPrivateTools(muonCaloEnergyTool)
    return result


def MuonMaterialProviderToolCfg(flags,  name="MuonTrkMaterialProviderTool", **kwargs):
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    from TrkConfig.AtlasExtrapolatorToolsConfig import AtlasMultipleScatteringUpdatorCfg

    result = AtlasExtrapolatorCfg(flags)
    atlas_extrapolator = result.popPrivateTools()
    kwargs.setdefault("Extrapolator", atlas_extrapolator)
    result.addPublicTool(atlas_extrapolator)
    kwargs.setdefault("MuonCaloEnergyTool", result.popToolsAndMerge(
        MuonCaloEnergyToolCfg(flags, name="MuonCaloEnergyTool")))

    # MuonCaloEnergyTool is actually a private tool
    calo_meas_tool = result.popToolsAndMerge(MuidCaloEnergyMeasCfg(flags))
    kwargs.setdefault("CaloMeasTool", calo_meas_tool)
    result.addPublicTool(calo_meas_tool)

    calo_param_tool = MuidCaloEnergyParam(flags)
    kwargs.setdefault("CaloParamTool", calo_param_tool)
    result.addPublicTool(calo_param_tool)

    multiple_scattering_tool = result.popToolsAndMerge(
        AtlasMultipleScatteringUpdatorCfg(flags))
    kwargs.setdefault("MultipleScatteringTool", multiple_scattering_tool)
    result.addPublicTool(multiple_scattering_tool)

    useCaloEnergyMeas = True
    if flags.Muon.MuonTrigger:
        useCaloEnergyMeas = False
    kwargs.setdefault("UseCaloEnergyMeasurement", useCaloEnergyMeas)
    acc = TrackingGeometryCondAlgCfg(flags)
    kwargs.setdefault("TrackingGeometryReadKey",
                      acc.getPrimary().TrackingGeometryWriteKey)
    result.merge(acc)

    energy_loss_updator = result.popToolsAndMerge(
        AtlasEnergyLossUpdatorCfg(flags))
    kwargs.setdefault("EnergyLossUpdator",
                      energy_loss_updator)  # PublicToolHandle
    result.addPublicTool(energy_loss_updator)

    track_isolation_tool = result.popToolsAndMerge(
        MuidTrackIsolationCfg(flags))
    kwargs.setdefault("TrackIsolationTool", track_isolation_tool)
    result.addPublicTool(track_isolation_tool)

    tool = CompFactory.Trk.TrkMaterialProviderTool(name=name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonSegmentHitSummaryToolCfg(flags, name="MuonSegmentHitSummaryTool", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = ComponentAccumulator()
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags)))
    result.merge(MuonGeoModelCfg(flags))
    kwargs.setdefault("DetectorManagerKey", "MuonDetectorManager")
    tool = CompFactory.Muon.MuonSegmentHitSummaryTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonSegmentConverterToolCfg(flags, name="MuonSegmentConverterTool", **kwargs):
    result = MuonSegmentHitSummaryToolCfg(flags)
    kwargs.setdefault("MuonSegmentHitSummaryTool", result.popPrivateTools())
    # HitTimingTool does not need specific configuration
    tool = CompFactory.Muon.MuonSegmentConverterTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonDressingToolCfg(flags, name="MuonDressingTool", **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonHitSummaryToolCfg
    result = MuonHitSummaryToolCfg(flags)
    kwargs.setdefault("MuonHitSummaryTool", result.popPrivateTools())
    # HitTimingTool does not need specific configuration
    tool = CompFactory.MuonCombined.MuonDressingTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCreatorToolCfg(flags, name="MuonCreatorTool", **kwargs):
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    result = ComponentAccumulator()
    # Not explicitly setting up MuonIdHelperSvc, nor MuonEDMHelperSvc (configured in top level reco Cfg)
    muon_edm_printer = result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault("Printer", muon_edm_printer)

    kwargs.setdefault("MuonPrinter", CompFactory.Rec.MuonPrintingTool(name='MuonPrintingTool',
                                                                      MuonStationPrinter=muon_edm_printer))

    kwargs.setdefault("ParticleCaloExtensionTool", 
                      result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags, 
                                                                           name='MuonParticleCaloExtensionTool', 
                                                                           StartFromPerigee=True)))
   
    from TrkConfig.TrkParticleCreatorConfig import MuonCombinedParticleCreatorCfg
    kwargs.setdefault("TrackParticleCreator", result.popToolsAndMerge(
        MuonCombinedParticleCreatorCfg(flags)))

    from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg
    kwargs.setdefault("AmbiguityProcessor", result.popToolsAndMerge(
        MuonAmbiProcessorCfg(flags)))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    kwargs.setdefault("Propagator", result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags)))
    kwargs.setdefault("MuonDressingTool", result.popToolsAndMerge(
        MuonDressingToolCfg(flags)))
    kwargs.setdefault("AddMDTExtrasMuGirlLowBeta", flags.MuonCombined.addMDTExtrasMuGirlLowBeta)
    # Not explicitly setting up MomentumBalanceTool nor ScatteringAngleTool
    # Not explicitly setting up MeanMDTdADCTool (but probably should FIXME)

    kwargs.setdefault("CaloMaterialProvider", result.popToolsAndMerge(
        MuonMaterialProviderToolCfg(flags)))

    kwargs.setdefault("TrackQuery",   result.popToolsAndMerge(
        MuonTrackQueryCfg(flags)))
    # runCommissioningChain
    if flags.Muon.SAMuonTrigger:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonTrackSummaryToolCfg(flags)))
    else:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonCombinedTrackSummaryToolCfg(flags)))

    if flags.Muon.MuonTrigger:
        kwargs.setdefault("MuonSelectionTool", "")
        kwargs.setdefault("UseCaloCells", False)
        kwargs.setdefault("CopyUInt8SummaryKeys", [])
    else:
        from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
        kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(
            MuonSelectionToolCfg(flags, name='MuonRecoSelTool')))
    kwargs.setdefault("RequireMSOEforSA", flags.Beam.Type is BeamType.Collisions)
    kwargs.setdefault("RequireCaloForSA", flags.Beam.Type is BeamType.Collisions)
 
    tool = CompFactory.MuonCombined.MuonCreatorTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def ExtrapolateMuonToIPToolCfg(flags, name="ExtrapolateMuonToIPTool", **kwargs):
    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("Extrapolator", result.popPrivateTools())
    from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg
    kwargs.setdefault("MuonExtrapolator", result.popToolsAndMerge(
        MuonExtrapolatorCfg(flags)))

    if flags.Muon.MuonTrigger:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonTrackSummaryToolCfg(flags)))
    else:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonCombinedTrackSummaryToolCfg(flags)))
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))
    result.setPrivateTools(CompFactory.ExtrapolateMuonToIPTool(name, **kwargs))
    return result


def MuonCandidateToolCfg(flags, name="MuonCandidateTool", **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg
    result = ComponentAccumulator()
    muon_edm_printer = result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags))
    kwargs.setdefault("Printer", muon_edm_printer)
    if "TrackBuilder" not in kwargs:
        kwargs.setdefault("TrackBuilder", result.popToolsAndMerge(
            CombinedMuonTrackBuilderCfg(flags, name="CombinedMuonTrackBuilder")))
    #   Why was this dependent on cosmics? will now always create this
    #   if flags.Beam.Type is BeamType.Cosmics:
    if flags.Muon.MuonTrigger and flags.Beam.Type is not BeamType.Cosmics:
        # trigger definitely only uses the ExtrapolateToIPtool in cosmics mode
        kwargs.setdefault("TrackExtrapolationTool", "")
    else:
        kwargs.setdefault("TrackExtrapolationTool", result.popToolsAndMerge(
            ExtrapolateMuonToIPToolCfg(flags)))
        kwargs.setdefault("SegmentContainer", "TrackMuonSegments")
    kwargs.setdefault("AmbiguityProcessor", result.popToolsAndMerge(
        MuonAmbiProcessorCfg(flags)))

    from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
        MuonTrackSummaryToolCfg(flags)))

    # MuonIDHelperSvc already configured

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("ExtrapolationStrategy", 1)

    track_segment_association_tool = CompFactory.MuonCombined.TrackSegmentAssociationTool(
        MuonEDMPrinterTool=muon_edm_printer)
    kwargs.setdefault("TrackSegmentAssociationTool",
                      track_segment_association_tool)
    result.addPublicTool(track_segment_association_tool)

    tool = CompFactory.MuonCombined.MuonCandidateTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCombinedDebuggerToolCfg(flags, name="MuonCombinedDebuggerTool", **kwargs):
    result = ComponentAccumulator()
    match_quality = CompFactory.Rec.MuonMatchQuality(
        TrackQuery=result.popToolsAndMerge(MuonTrackQueryCfg(flags)))
    kwargs.setdefault("MuonMatchQuality", match_quality)
    tool = CompFactory.MuonCombined.MuonCombinedDebuggerTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCombinedToolCfg(flags, name="MuonCombinedTool", **kwargs):
    tools = []
    result = ComponentAccumulator()
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))

    if flags.MuonCombined.doCombinedFit:
        tool = result.popToolsAndMerge(MuonCombinedFitTagToolCfg(flags))
        tools.append(tool)
    if flags.MuonCombined.doStatisticalCombination and flags.Beam.Type is not BeamType.Cosmics:
        tool = result.popToolsAndMerge(MuonCombinedStacoTagToolCfg(flags))
        tools.append(tool)

    kwargs.setdefault("MuonCombinedTagTools", tools)
    kwargs.setdefault("MuonCombinedDebuggerTool", result.popToolsAndMerge(
        MuonCombinedDebuggerToolCfg(flags)))

    acc = MuonAlignmentUncertToolThetaCfg(flags)
    result.merge(acc)
    kwargs.setdefault("AlignmentUncertTool", result.getPublicTool(
        'MuonAlignmentUncertToolTheta'))

    kwargs.setdefault("DeltaEtaPreSelection", 0.2)
    kwargs.setdefault("DeltaPhiPreSelection", 0.2)
    tool = CompFactory.MuonCombined.MuonCombinedTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCombinedFitTagToolCfg(flags, name="MuonCombinedFitTagTool", **kwargs):
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("VertexContainer", "")

    result = ComponentAccumulator()
    kwargs.setdefault("TrackBuilder",  result.popToolsAndMerge(
        CombinedMuonTrackBuilderCfg(flags)))

    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))

    kwargs.setdefault("TrackQuery",   result.popToolsAndMerge(
        MuonTrackQueryCfg(flags)))
    kwargs.setdefault("MatchQuality",  result.popToolsAndMerge(
        MuonMatchQualityCfg(flags)))

    from MuonConfig.MuonRecToolsConfig import MuonTrackScoringToolCfg
    kwargs.setdefault("TrackScoringTool", result.popToolsAndMerge(
        MuonTrackScoringToolCfg(flags)))

    tool = CompFactory.MuonCombined.MuonCombinedFitTagTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def EMEO_MuonCombinedFitTagToolCfg(flags, name="MuonCombinedFitTagTool_EMEO", **kwargs):
    result = ComponentAccumulator()
    track_builder = result.popToolsAndMerge(
        EMEO_CombinedMuonTrackBuilderCfg(flags))
    fit_tag_tool = result.popToolsAndMerge(MuonCombinedFitTagToolCfg(flags, name=name,
                                                                     TrackBuilder=track_builder,
                                                                     **kwargs))
    result.setPrivateTools(fit_tag_tool)
    return result


def EMEO_MuonCombinedToolCfg(flags, name="MuonCombinedTool_EMEO", **kwargs):
    tools = []
    result = ComponentAccumulator()
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))

    if flags.MuonCombined.doCombinedFit:
        tool = result.popToolsAndMerge(EMEO_MuonCombinedFitTagToolCfg(flags))
        tools.append(tool)
    if flags.MuonCombined.doStatisticalCombination:
        tool = result.popToolsAndMerge(MuonCombinedStacoTagToolCfg(flags))
        tools.append(tool)

    kwargs.setdefault("MuonCombinedTagTools", tools)
    kwargs.setdefault("MuonCombinedDebuggerTool", result.popToolsAndMerge(
        MuonCombinedDebuggerToolCfg(flags)))

    acc = MuonAlignmentUncertToolThetaCfg(flags)
    result.merge(acc)
    kwargs.setdefault("AlignmentUncertTool", result.getPublicTool(
        'MuonAlignmentUncertToolTheta'))

    kwargs.setdefault("DeltaEtaPreSelection", 0.2)
    kwargs.setdefault("DeltaPhiPreSelection", 0.2)
    tool = CompFactory.MuonCombined.MuonCombinedTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCombinedStacoTagToolCfg(flags, name="MuonCombinedStacoTagTool", **kwargs):

    result = ComponentAccumulator()
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("TagTool", result.popToolsAndMerge(
        CombinedMuonTagTestToolCfg(flags)))
    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags)))

    tool = CompFactory.MuonCombined.MuonCombinedStacoTagTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result

# From Combined fit tools.py


def MuidMaterialAllocatorCfg(flags, name='MuidMaterialAllocator', **kwargs):
    from TrkConfig.TrkExSTEP_PropagatorConfig import AtlasSTEP_PropagatorCfg
    kwargs.setdefault("AggregateMaterial", True)
    kwargs.setdefault("AllowReordering", False)

    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("Extrapolator", result.popPrivateTools())
    # Intersector (a RungeKuttaIntersector) does not require explicit configuration
    kwargs.setdefault("STEP_Propagator", result.popToolsAndMerge(
        AtlasSTEP_PropagatorCfg(flags, name="AtlasSTEP_Propagator")))
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge(TrackingGeometryCondAlgCfg(flags))
    kwargs.setdefault("TrackingGeometryReadKey", "AtlasTrackingGeometry")

    tool = CompFactory.Trk.MaterialAllocator(name, **kwargs)
    result.setPrivateTools(tool)
    return result

# and the fitter


def iPatFitterCfg(flags, name='iPatFitter', **kwargs):
    from TrkConfig.SolenoidalIntersectorConfig import SolenoidalIntersectorCfg
    from TrkConfig.TrkExSTEP_PropagatorConfig import AtlasSTEP_PropagatorCfg

    kwargs.setdefault("AggregateMaterial", True)
    kwargs.setdefault("FullCombinedFit", True)
    result = MuidMaterialAllocatorCfg(flags)
    kwargs.setdefault("MaterialAllocator", result.popPrivateTools())
    # RungeKuttaIntersector needs a AtlasFieldCacheCondObj, but it's impossible to get here without that being configured already so let's be lazy
    # It does not otherwise require explicit configuration
    kwargs.setdefault('SolenoidalIntersector', result.popToolsAndMerge(
        SolenoidalIntersectorCfg(flags)))
    kwargs.setdefault('Propagator', result.popToolsAndMerge(
        AtlasSTEP_PropagatorCfg(flags)))
    # StraightLineIntersector does not need explicit configuration
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("MaxIterations", 15)
    if flags.Muon.SAMuonTrigger:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonTrackSummaryToolCfg(flags)))
    else:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonCombinedTrackSummaryToolCfg(flags)))
    # This is only to match old-style config. Does nothing.
    kwargs.setdefault("TrackingVolumesSvc", "TrackingVolumesSvc")

    tool = CompFactory.Trk.iPatFitter(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def iPatSLFitterCfg(flags, name='iPatSLFitter', **kwargs):
    kwargs.setdefault("LineFit", True)
    kwargs.setdefault("LineMomentum", flags.Muon.straightLineFitMomentum)
    return iPatFitterCfg(flags, name, **kwargs)

# track cleaner configured to use the same fitter


def MuidTrackCleanerCfg(flags, name='MuidTrackCleaner', **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonTrackCleanerCfg
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("PullCut", 5.0)
        kwargs.setdefault("PullCutPhi", 10.0)
    else:
        kwargs.setdefault("PullCut", 4.0)
        kwargs.setdefault("PullCutPhi", 4.0)
    result = ComponentAccumulator()
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("Iterate", False)
        kwargs.setdefault("RecoverOutliers", False)
        kwargs.setdefault("Fitter", result.popToolsAndMerge(
            iPatFitterCfg(flags, 'iPatFitterClean', MaxIterations=4)))
    else:
        kwargs.setdefault(
            "Fitter", result.popToolsAndMerge(iPatFitterCfg(flags)))
        kwargs.setdefault(
            "SLFitter", result.popToolsAndMerge(iPatSLFitterCfg(flags)))

    # For these following items, set back to default, because overridden in MuonTrackCleaner and we don't want overrides.
    # ALL properties that are set in old-style are: PullCut, PullCutPhi, Iterate, RecoverOutliers, Fitter and iPatSLFitter
    # However since there are defaults we still DO need to explicitly set, it's still probably easier to use MuonTrackCleanerCfg
    kwargs.setdefault("MaxAvePullSumPerChamber", 3.5)
    kwargs.setdefault("Chi2Cut", 100.0)
    kwargs.setdefault("Extrapolator", result.getPrimaryAndMerge(
        AtlasExtrapolatorCfg(flags)))
    result.setPrivateTools(result.popToolsAndMerge(
        MuonTrackCleanerCfg(flags, name, **kwargs)))
    return result


def MuidCaloEnergyParam(flags, name='MuidCaloEnergyParam', **kwargs):
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    return CompFactory.Rec.MuidCaloEnergyParam(name, **kwargs)


def MuidCaloEnergyMeasCfg(flags, name='MuidCaloEnergyMeas', **kwargs):
    result = ComponentAccumulator()
    muidcaloenergyparam = MuidCaloEnergyParam(flags)
    kwargs.setdefault("CaloParamTool",  muidcaloenergyparam)
    # FIXME! Need to setup the folders for CaloNoiseKey (which is why this needs a CA)
    # Not sure how to do : if flags.haveRIO.Calo_on() but TBH, if the cells aren't there it will abort anyway
    kwargs.setdefault("CellContainerLocation", "AllCalo")
    kwargs.setdefault("NoiseThresInSigmas", 4.)
    tool = CompFactory.Rec.MuidCaloEnergyMeas(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidCaloEnergyToolParamCfg(flags, name='MuidCaloEnergyToolParam', **kwargs):
    # Some duplication with MuidCaloEnergyToolCfg but probably safer like this, since
    # we don't want to set e.g. MinFinalEnergy here
    result = MuidCaloEnergyMeasCfg(flags)
    kwargs.setdefault("CaloMeasTool", result.popPrivateTools())
    kwargs.setdefault("EnergyLossMeasurement", False)

    kwargs.setdefault("CaloParamTool", MuidCaloEnergyParam(flags))
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("Cosmics", True)
    kwargs.setdefault("TrackIsolationTool", result.popToolsAndMerge(
        MuidTrackIsolationCfg(flags)))

    result.setPrivateTools(CompFactory.Rec.MuidCaloEnergyTool(name, **kwargs))
    return result


def MuidTrackIsolationCfg(flags, name='MuidTrackIsolation', **kwargs):
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    kwargs.setdefault("InDetTracksLocation",
                      "CombinedInDetTracks" if flags.Detector.GeometryID else "CombinedITkTracks")
    # RungeKuttaIntersector requires the magnetic field conditions
    result = AtlasFieldCacheCondAlgCfg(flags)
    tool = CompFactory.Rec.MuidTrackIsolation(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidCaloEnergyToolCfg(flags, name='MuidCaloEnergyTool', **kwargs):
    result = MuidCaloEnergyMeasCfg(flags)
    kwargs.setdefault("CaloMeasTool", result.popPrivateTools())
    kwargs.setdefault("CaloParamTool", MuidCaloEnergyParam(flags))
    kwargs.setdefault("MinFinalEnergy", 1.0*GeV)
    kwargs.setdefault("MinMuonPt", 10.0*GeV)
    kwargs.setdefault("MopParametrization", True)
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("EnergyLossMeasurement", False)
        kwargs.setdefault("TrackIsolation", False)
    else:
        kwargs.setdefault("EnergyLossMeasurement", True)
        kwargs.setdefault("TrackIsolation", True)

    kwargs.setdefault("TrackIsolationTool", result.popToolsAndMerge(
        MuidTrackIsolationCfg(flags)))
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    result.setPrivateTools(CompFactory.Rec.MuidCaloEnergyTool(name, **kwargs))
    return result


def MuidCaloTrackStateOnSurfaceCfg(flags, name='MuidCaloTrackStateOnSurface', **kwargs):
    result = ComponentAccumulator()
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    kwargs.setdefault("Propagator", result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags)))
    kwargs.setdefault("MinRemainingEnergy", 0.2*GeV)
    kwargs.setdefault("ParamPtCut", 3.0*GeV)
    kwargs.setdefault("CaloEnergyDeposit", result.popToolsAndMerge(
        MuidCaloEnergyToolCfg(flags)))
    kwargs.setdefault("CaloEnergyParam", result.popToolsAndMerge(
        MuidCaloEnergyToolParamCfg(flags)))
    # I don't think CaloMaterialParam i.e. MuidCaloMaterialParam needs explicit configuration
    # Ditto for IntersectorWrapper, since it just uses RKIntersector which doesn't
    tool = CompFactory.Rec.MuidCaloTrackStateOnSurface(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidCaloTrackStateOnSurfaceParamCfg(flags, name='MuidCaloTrackStateOnSurfaceParam', **kwargs):
    result = ComponentAccumulator()
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    kwargs.setdefault("Propagator", result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags)))
    kwargs.setdefault("MinRemainingEnergy", 0.2*GeV)
    kwargs.setdefault("ParamPtCut", 3.0*GeV)
    kwargs.setdefault("CaloEnergyDeposit", MuidCaloEnergyParam(flags))
    kwargs.setdefault("CaloEnergyParam",   MuidCaloEnergyParam(flags))
    tool = CompFactory.Rec.MuidCaloTrackStateOnSurface(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidMaterialEffectsOnTrackProviderCfg(flags, name='MuidMaterialEffectsOnTrackProvider', **kwargs):
    result = MuidCaloTrackStateOnSurfaceCfg(flags)
    kwargs.setdefault("TSOSTool",      result.popPrivateTools())
    acc = MuidCaloTrackStateOnSurfaceParamCfg(flags)
    kwargs.setdefault("TSOSToolParam", acc.popPrivateTools())
    result.merge(acc)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    tool = CompFactory.Rec.MuidMaterialEffectsOnTrackProvider(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidMaterialEffectsOnTrackProviderParamCfg(flags, name='MuidMaterialEffectsOnTrackProviderParam', **kwargs):
    result = MuidCaloTrackStateOnSurfaceParamCfg(flags)
    muidtsosparam = result.popPrivateTools()
    kwargs.setdefault("TSOSTool",      muidtsosparam)
    kwargs.setdefault("TSOSToolParam", muidtsosparam)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)
    tool = CompFactory.Rec.MuidMaterialEffectsOnTrackProvider(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonTrackQueryCfg(flags, name="MuonTrackQuery", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg
    result = MdtDriftCircleOnTrackCreatorCfg(flags)
    kwargs.setdefault("MdtRotCreator",   result.popPrivateTools())
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge(TrackingGeometryCondAlgCfg(flags))
    kwargs.setdefault("TrackingGeometryReadKey", "AtlasTrackingGeometry")

    acc = iPatFitterCfg(flags)
    kwargs.setdefault("Fitter", acc.popPrivateTools())
    result.merge(acc)

    tool = CompFactory.Rec.MuonTrackQuery(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def EMEO_MuonSegmentRegionRecoveryToolCfg(flags, name="MuonSegmentRegionRecoveryTool_EMEO"):
    result = ComponentAccumulator()
    from MuonConfig.MuonTrackBuildingConfig import EMEO_MuonChamberHoleRecoveryToolCfg
    chamber_recovery = result.popToolsAndMerge(
        EMEO_MuonChamberHoleRecoveryToolCfg(flags))
    trk_builder = result.popToolsAndMerge(
        EMEO_CombinedTrackBuilderFitCfg(flags))
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
    muon_combined_track_summary = result.popToolsAndMerge(
        MuonCombinedTrackSummaryToolCfg(flags))
    tool = result.popToolsAndMerge(MuonSegmentRegionRecoveryToolCfg(flags,
                                                                    name=name,
                                                                    ChamberHoleRecoveryTool=chamber_recovery,
                                                                    Builder=trk_builder,
                                                                    TrackSummaryTool=muon_combined_track_summary,
                                                                    STGCRegionSelector="",
                                                                    MMRegionSelector="",
                                                                    RecoverMM=False,
                                                                    RecoverSTGC=False))
    result.setPrivateTools(tool)
    return result


def EMEO_CombinedMuonTrackBuilderCfg(flags, name="MuonCombinedTrackBuilder_EMEO"):
    result = ComponentAccumulator()
    recovery_tool = result.popToolsAndMerge(
        EMEO_MuonSegmentRegionRecoveryToolCfg(flags))
    acc = CombinedMuonTrackBuilderCfg(flags, name,
                                      MuonRotCreator="",
                                      MuonHoleRecovery=recovery_tool)
    # Need to reset this to be the primary tool
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def MuidErrorOptimisationToolCfg(flags, name='MuidErrorOptimisationTool', **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonTrackSummaryHelperToolCfg, MuonRefitToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("TrackSummaryTool",  result.popToolsAndMerge(
        MuonTrackSummaryHelperToolCfg(flags)))
    useAlignErrs = True
    if flags.IOVDb.DatabaseInstance == 'COMP200' or \
            'HLT' in flags.IOVDb.GlobalTag or flags.Common.isOnline or flags.Muon.MuonTrigger:
        useAlignErrs = False
    kwargs.setdefault("RefitTool", result.popToolsAndMerge(MuonRefitToolCfg(
        flags, name="MuidRefitTool", AlignmentErrors=useAlignErrs,
        Fitter=result.popToolsAndMerge(iPatFitterCfg(flags)))))
    tool = CompFactory.Muon.MuonErrorOptimisationTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonAlignmentUncertToolThetaCfg(flags, name="MuonAlignmentUncertToolTheta", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonGeometryConfig import TrackingVolumesSvcCfg
    kwargs.setdefault("TrackingVolumesSvc", result.getPrimaryAndMerge(TrackingVolumesSvcCfg(flags)))
    kwargs.setdefault("HistoName", "ThetaScattering")
    kwargs.setdefault(
        "InFile", "MuonCombinedBaseTools/AlignmentUncertainties/201029_initial/ID_MS_Uncertainties.root")
    tool = CompFactory.Muon.MuonAlignmentUncertTool(name, **kwargs)
    result.addPublicTool(tool)
    return result


def MuonAlignmentUncertToolPhiCfg(flags, name="MuonAlignmentUncertToolPhi", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("HistoName", "PhiScattering")
    from MuonConfig.MuonGeometryConfig import TrackingVolumesSvcCfg
    kwargs.setdefault("TrackingVolumesSvc", result.getPrimaryAndMerge(TrackingVolumesSvcCfg(flags)))
    kwargs.setdefault(
        "InFile", "MuonCombinedBaseTools/AlignmentUncertainties/201029_initial/ID_MS_Uncertainties.root")
    tool = CompFactory.Muon.MuonAlignmentUncertTool(name, **kwargs)
    result.addPublicTool(tool)
    return result


@AccumulatorCache
def CombinedMuonTrackBuilderCfg(flags, name='CombinedMuonTrackBuilder', **kwargs):
    from AthenaCommon.SystemOfUnits import meter
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import CscClusterOnTrackCreatorCfg, MdtDriftCircleOnTrackCreatorCfg, MuonClusterOnTrackCreatorCfg
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg

    result = ComponentAccumulator()

    from MuonConfig.MuonGeometryConfig import TrackingVolumesSvcCfg
    kwargs.setdefault("TrackingVolumesSvc", result.getPrimaryAndMerge(TrackingVolumesSvcCfg(flags)))

    kwargs.setdefault("CaloEnergyParam", result.popToolsAndMerge(
        MuidCaloEnergyToolParamCfg(flags)))
    kwargs.setdefault("CaloTSOS",        result.popToolsAndMerge(
        MuidCaloTrackStateOnSurfaceCfg(flags)))
    kwargs.setdefault("Cleaner",         result.popToolsAndMerge(
        MuidTrackCleanerCfg(flags)))
    result.merge(MuonAlignmentUncertToolPhiCfg(flags))
    kwargs.setdefault("AlignmentUncertToolPhi",
                      result.getPublicTool('MuonAlignmentUncertToolPhi'))
    result.merge(MuonAlignmentUncertToolThetaCfg(flags))
    kwargs.setdefault("AlignmentUncertToolTheta",
                      result.getPublicTool('MuonAlignmentUncertToolTheta'))

    if flags.Detector.GeometryCSC and not flags.Muon.MuonTrigger:
        kwargs.setdefault("CscRotCreator", result.popToolsAndMerge(
            CscClusterOnTrackCreatorCfg(flags)))
    else:
        kwargs.setdefault("CscRotCreator", "")

    kwargs.setdefault("Extrapolator",      result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags)))
    kwargs.setdefault(
        "Fitter",            result.popToolsAndMerge(iPatFitterCfg(flags)))
    kwargs.setdefault(
        "SLFitter",          result.popToolsAndMerge(iPatSLFitterCfg(flags)))
    kwargs.setdefault("MaterialAllocator", result.popToolsAndMerge(
        MuidMaterialAllocatorCfg(flags)))
    kwargs.setdefault("MdtRotCreator",     result.popToolsAndMerge(
        MdtDriftCircleOnTrackCreatorCfg(flags)))
    kwargs.setdefault("MuonRotCreator",    result.popToolsAndMerge(
        MuonClusterOnTrackCreatorCfg(flags)))

    # Tracking Geometry
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
        TrackingGeometryCondAlgCfg)

    geom_cond_key = result.getPrimaryAndMerge(
        TrackingGeometryCondAlgCfg(flags)).TrackingGeometryWriteKey
    kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)

    kwargs.setdefault("CleanCombined", True)
    kwargs.setdefault("CleanStandalone", True)
    kwargs.setdefault("BadFitChi2", 2.5)
    kwargs.setdefault("LargeMomentumError", 0.5)
    kwargs.setdefault("LineMomentum", flags.Muon.straightLineFitMomentum)
    kwargs.setdefault("LowMomentum", 10.*GeV)
    kwargs.setdefault("MinEnergy", 0.3*GeV)
    kwargs.setdefault("PerigeeAtSpectrometerEntrance", False)
    kwargs.setdefault("ReallocateMaterial", False)
    kwargs.setdefault("Vertex2DSigmaRPhi", 100.*mm)
    kwargs.setdefault("Vertex3DSigmaRPhi", 6.*mm)
    kwargs.setdefault("Vertex3DSigmaZ", 60.*mm)
    kwargs.setdefault("UseCaloTG", True)

    kwargs.setdefault("CaloMaterialProvider", result.popToolsAndMerge(
        MuonMaterialProviderToolCfg(flags)))
    kwargs.setdefault("TrackQuery", result.popToolsAndMerge(
        MuonTrackQueryCfg(flags)))

    if flags.Muon.SAMuonTrigger:
        from TrkConfig.TrkTrackSummaryToolConfig import MuonTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonTrackSummaryToolCfg(flags)))
    else:
        kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
            MuonCombinedTrackSummaryToolCfg(flags)))

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import MuonCombinedPropagatorCfg, RungeKuttaPropagatorCfg
    propagator = None
    if flags.Muon.MuonTrigger:
        propagator = result.popToolsAndMerge(RungeKuttaPropagatorCfg(flags))
    else:
        propagator = result.popToolsAndMerge(MuonCombinedPropagatorCfg(flags))
    kwargs.setdefault("Propagator",   propagator)
    kwargs.setdefault("SLPropagator", propagator)
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))

    if flags.Beam.Type is BeamType.Cosmics:
        kwargs.setdefault("MdtRotCreator",  "")
        kwargs.setdefault("MuonRotCreator",   "")
        kwargs.setdefault("LowMomentum",  1.5*GeV)
        kwargs.setdefault("ReallocateMaterial", False)
        kwargs.setdefault("Vertex2DSigmaRPhi", 100.*mm)
        kwargs.setdefault("Vertex3DSigmaRPhi", 100.*mm)
        kwargs.setdefault("Vertex3DSigmaZ",  1.*meter)

    # configure tools for data reprocessing
    if flags.Muon.enableErrorTuning and 'MuonErrorOptimizer' not in kwargs:
        erroropt = result.popToolsAndMerge(MuidErrorOptimisationToolCfg(
            flags, name="MuidErrorOptimisationTool", PrepareForFit=False, RecreateStartingParameters=False))
        kwargs.setdefault("MuonErrorOptimizer", erroropt)
    else:
        kwargs.setdefault("MuonErrorOptimizer", "")

    if flags.Muon.MuonTrigger:
        kwargs.setdefault("MuonHoleRecovery", "")
    else:
        if "MuonHoleRecovery" not in kwargs:
            # Meeded to resolve circular dependency since MuonSegmentRegionRecoveryToolCfg calls CombinedMuonTrackBuilderCfg (i.e. this)!
            muon_combined_track_summary = result.popToolsAndMerge(
                MuonCombinedTrackSummaryToolCfg(flags))
            acc = MuonSegmentRegionRecoveryToolCfg(
                flags, name="MuidSegmentRegionRecoveryTool", TrackSummaryTool=muon_combined_track_summary)
            kwargs.setdefault("MuonHoleRecovery", acc.popPrivateTools())
            result.merge(acc)

    if flags.Muon.doSegmentT0Fit:
        kwargs.setdefault("MdtRotCreator", "")
    tool = CompFactory.Rec.CombinedMuonTrackBuilder(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def CombinedMuonTrackBuilderFitCfg(flags, name='CombinedMuonTrackBuilderFit', **kwargs):
    # In the old configuration we had duplication between CombinedMuonTrackBuilder and CombinedMuonTrackBuilderFit
    # Here we just call the Combined
    from MuonConfig.MuonTrackBuildingConfig import MuonChamberHoleRecoveryToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("PerigeeAtSpectrometerEntrance", True)
    kwargs.setdefault("UseCaloTG", False)
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("MuonErrorOptimizer", "")
    else:
        kwargs.setdefault("MuonErrorOptimizer", result.popToolsAndMerge(
            MuidErrorOptimisationToolCfg(flags, PrepareForFit=False, RecreateStartingParameters=False)))
    kwargs.setdefault("MuonHoleRecovery", result.popToolsAndMerge(
        MuonChamberHoleRecoveryToolCfg(flags)))

    tool = result.popToolsAndMerge(CombinedMuonTrackBuilderCfg(
        flags, name, **kwargs))  # Need to reset this to be the primary tool
    result.setPrivateTools(tool)
    return result


def EMEO_CombinedTrackBuilderFitCfg(flags, name="CombinedTrackBuilderFit_EMEO", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonTrackBuildingConfig import EMEO_MuonChamberHoleRecoveryToolCfg
    if not flags.Muon.MuonTrigger:
        trk_builder = result.popToolsAndMerge(
            EMEO_MuonChamberHoleRecoveryToolCfg(flags))
        kwargs.setdefault("MuonHoleRecovery", trk_builder)
    else:
        kwargs.setdefault("MuonHoleRecovery", "")
    kwargs.setdefault("MuonRotCreator", "")
    tool = result.popToolsAndMerge(CombinedMuonTrackBuilderFitCfg(
        flags, name, **kwargs))  # Need to reset this to be the primary tool
    result.setPrivateTools(tool)
    return result


def MuonMatchQualityCfg(flags, name='MuonMatchQuality', **kwargs):
    result = CombinedMuonTagTestToolCfg(flags)
    kwargs.setdefault("TagTool", result.popPrivateTools())
    kwargs.setdefault("TrackQuery", result.popToolsAndMerge(
        MuonTrackQueryCfg(flags)))

    tool = CompFactory.Rec.MuonMatchQuality(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuidMuonRecoveryCfg(flags, name='MuidMuonRecovery', **kwargs):
    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("Extrapolator", result.popPrivateTools())
    kwargs.setdefault("TrackBuilder", result.popToolsAndMerge(
        CombinedMuonTrackBuilderCfg(flags)))

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))
    
    tool = CompFactory.Rec.MuidMuonRecovery(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def CombinedMuonTagTestToolCfg(flags, name='CombinedMuonTagTestTool', **kwargs):
    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("ExtrapolatorTool", result.popPrivateTools())
    kwargs.setdefault("Chi2Cut", 50000.)
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge(TrackingGeometryCondAlgCfg(flags))
    kwargs.setdefault("TrackingGeometryReadKey", "AtlasTrackingGeometry")
    tool = CompFactory.MuonCombined.MuonTrackTagTestTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result

# From MuonCaloTagTool.py


def TrackDepositInCaloToolCfg(flags, name='TrackDepositInCaloTool', **kwargs):
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg, ParticleCaloCellAssociationToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleCaloExtensionTool",
                      result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)))
    kwargs.setdefault("ExtrapolatorHandle", result.popToolsAndMerge(
        AtlasExtrapolatorCfg(flags)))
    kwargs.setdefault("ParticleCaloCellAssociationTool",
                      result.popToolsAndMerge(ParticleCaloCellAssociationToolCfg(flags, name="Rec::ParticleCaloCellAssociationTool")))
    tool = CompFactory.TrackDepositInCaloTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def CaloMuonLikelihoodToolCfg(flags, name='CaloMuonLikelihoodTool', **kwargs):
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleCaloExtensionTool",
                      result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)))
    tool = CompFactory.CaloMuonLikelihoodTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def CaloMuonScoreToolCfg(flags, name='CaloMuonScoreTool', **kwargs):
    from TrackToCalo.TrackToCaloConfig import ParticleCaloCellAssociationToolCfg
    result = ComponentAccumulator()
    kwargs.setdefault("ParticleCaloCellAssociationTool",
                      result.popToolsAndMerge(ParticleCaloCellAssociationToolCfg(flags)))
    tool = CompFactory.CaloMuonScoreTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def CaloMuonTagCfg(flags, name="CaloMuonTag", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.CaloMuonTag(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


def MuonCaloTagToolCfg(flags, name='MuonCaloTagTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("CaloMuonTagLoose",
                      result.popToolsAndMerge(CaloMuonTagCfg(flags, name="CaloMuonTagLoose", TagMode="Loose")))
    kwargs.setdefault("CaloMuonTagTight",
                      result.popToolsAndMerge(CaloMuonTagCfg(flags)))
    kwargs.setdefault("CaloMuonLikelihoodTool",
                      result.popToolsAndMerge(CaloMuonLikelihoodToolCfg(flags)))
    kwargs.setdefault("CaloMuonScoreTool",
                      result.popToolsAndMerge(CaloMuonScoreToolCfg(flags)))
    kwargs.setdefault("TrackDepositInCaloTool",
                      result.popToolsAndMerge(TrackDepositInCaloToolCfg(flags)))
    from InDetConfig.InDetTrackSelectorToolConfig import CaloTrkMuIdAlgTrackSelectorToolCfg
    kwargs.setdefault("TrackSelectorTool",
                      result.popToolsAndMerge(CaloTrkMuIdAlgTrackSelectorToolCfg(flags)))
    kwargs.setdefault("doCaloLR", False)
    the_tool = CompFactory.MuonCombined.MuonCaloTagTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

# Misc


def MuonLayerSegmentFinderToolCfg(flags, name="MuonLayerSegmentFinderTool", **kwargs):
    from MuonConfig.MuonSegmentFindingConfig import DCMathSegmentMakerCfg, MuonNSWSegmentFinderToolCfg, MuonPRDSelectionToolCfg
    result = ComponentAccumulator()

    from MuonConfig.MuonSegmentFindingConfig import Csc2dSegmentMakerCfg, Csc4dSegmentMakerCfg
    kwargs.setdefault("Csc2DSegmentMaker", result.popToolsAndMerge(
        Csc2dSegmentMakerCfg(flags)) if flags.Detector.GeometryCSC else "")
    kwargs.setdefault("Csc4DSegmentMaker", result.popToolsAndMerge(
        Csc4dSegmentMakerCfg(flags)) if flags.Detector.GeometryCSC else "")

    kwargs.setdefault("MuonPRDSelectionTool", result.popToolsAndMerge(
        MuonPRDSelectionToolCfg(flags)))
    kwargs.setdefault("SegmentMaker",  result.popToolsAndMerge(
        DCMathSegmentMakerCfg(flags, name="DCMathSegmentMaker")))
    kwargs.setdefault("NSWMuonClusterSegmentFinderTool",
                      result.popToolsAndMerge(MuonNSWSegmentFinderToolCfg(flags, name="MuonNSWSegmentFinderTool")))

    ###
    from AthenaConfiguration.Enums import LHCPeriod
    if flags.GeoModel.Run < LHCPeriod.Run3 or flags.Muon.MuonTrigger:
        kwargs.setdefault("InSegmentContainer", "")
        kwargs.setdefault("MuonLayerSegmentMatchingTool", "")
    else:
        kwargs.setdefault("MuonLayerSegmentMatchingTool", result.popToolsAndMerge(
            MuonLayerSegmentMatchingToolCfg(flags)))
    tool = CompFactory.Muon.MuonLayerSegmentFinderTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonLayerSegmentMatchingToolCfg(flags, name="MuonLayerSegmentMatchingTool", **kwargs):
    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("Extrapolator", result.popPrivateTools())
    MuTagTool = result.getPrimaryAndMerge(MuTagMatchingToolCfg(flags))
    kwargs.setdefault("MatchTool", MuTagTool)

    tool = CompFactory.Muon.MuonLayerSegmentMatchingTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonInsideOutRecoToolCfg(flags, name="MuonInsideOutRecoTool", **kwargs):
    from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg
    if flags.Muon.MuonTrigger:
        kwargs.setdefault("VertexContainer", "")

    result = MuonEDMPrinterToolCfg(flags)
    kwargs.setdefault("MuonEDMPrinterTool", result.popPrivateTools())

    layersegmentfindertool = result.popToolsAndMerge(
        MuonLayerSegmentFinderToolCfg(flags, name="MuonLayerSegmentFinderTool"))
    kwargs.setdefault("MuonLayerSegmentFinderTool", layersegmentfindertool)
    kwargs.setdefault("MuonLayerSegmentMatchingTool", result.popToolsAndMerge(
        MuonLayerSegmentMatchingToolCfg(flags)))
    kwargs.setdefault("MuonLayerAmbiguitySolverTool",  result.popToolsAndMerge(
        MuonLayerAmbiguitySolverToolCfg(flags)))
    kwargs.setdefault("MuonCandidateTrackBuilderTool", result.popToolsAndMerge(
        MuonCandidateTrackBuilderToolCfg(flags)))
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
    kwargs.setdefault("TrackSummaryTool", result.popToolsAndMerge(
        MuonCombinedTrackSummaryToolCfg(flags)))
    kwargs.setdefault("MuonTrackBuilder", result.popToolsAndMerge(
        CombinedMuonTrackBuilderCfg(flags)))
    kwargs.setdefault("TrackAmbiguityProcessor",
                      result.popToolsAndMerge(MuonAmbiProcessorCfg(flags)))

    tool = CompFactory.MuonCombined.MuonInsideOutRecoTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonCandidateTrackBuilderToolCfg(flags, name="MuonCandidateTrackBuilderTool", **kwargs):
    result = CombinedMuonTrackBuilderCfg(flags)
    kwargs.setdefault("MuonTrackBuilder", result.popPrivateTools())
    muoncandidatetrackbuilder = CompFactory.Muon.MuonCandidateTrackBuilderTool(
        name, **kwargs)
    result.setPrivateTools(muoncandidatetrackbuilder)
    return result


def MuonSegmentSelectionToolCfg(flags, name="MuonSegmentSelectionTool", **kwargs):
    if flags.Input.isMC is False:
        kwargs.setdefault("GoodADCFractionCut",  0.5)
        kwargs.setdefault("MinADCPerSegmentCut", 100)
    result = ComponentAccumulator()
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))
    muon_segment_hit_summary_tool = result.popToolsAndMerge(
        MuonSegmentHitSummaryToolCfg(flags))
    kwargs.setdefault("MuonSegmentHitSummaryTool",
                      muon_segment_hit_summary_tool)
    result.addPublicTool(muon_segment_hit_summary_tool)

    result.setPrivateTools(
        CompFactory.Muon.MuonSegmentSelectionTool(name, **kwargs))
    return result


def MuonLayerAmbiguitySolverToolCfg(flags, name="MuonLayerAmbiguitySolverTool", **kwargs):
    from MuonConfig.MuonTrackBuildingConfig import MuonSegmentMatchingToolCfg, MooTrackBuilderCfg
    result = MuonSegmentSelectionToolCfg(flags)
    kwargs.setdefault("MuonSegmentSelectionTool", result.popPrivateTools())
    kwargs.setdefault("MuonSegmentMatchingTool", result.popToolsAndMerge(
        MuonSegmentMatchingToolCfg(flags, name='MuonSegmentMatchingToolTight', TightSegmentMatching=True)))

    kwargs.setdefault("MuonSegmentTrackBuilder",
                      result.popToolsAndMerge(MooTrackBuilderCfg(flags, name='MooMuonTrackBuilder', prefix='MuSt_')))
    kwargs.setdefault("MuonEDMPrinterTool", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))

    result.setPrivateTools(
        CompFactory.Muon.MuonLayerAmbiguitySolverTool(name, **kwargs))
    return result


def MdtDriftCircleOnTrackCreatorStauCfg(flags, name="MdtDriftCircleOnTrackCreatorStau", **kwargs):
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg, MdtCalibWindowNumber, MdtCalibToolForRotsCfg
    result = ComponentAccumulator()
    kwargs.setdefault("TimingMode", 3)
    kwargs.setdefault("CalibrationTool", result.popToolsAndMerge(MdtCalibToolForRotsCfg(flags, TimeWindowSetting = MdtCalibWindowNumber('Collision_t0fit') )) )
    the_tool = result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorCfg(flags, name, **kwargs))
    result.setPrivateTools(the_tool)
    return result


def MuonStauRecoToolCfg(flags,  name="MuonStauRecoTool", **kwargs):
    # In the old configuration this was split over several functions. But since these Stau tools are only used here,
    # trying a new approach. We can always refactorise later if necessary.
    from MuonConfig.MuonSegmentFindingConfig import DCMathSegmentMakerCfg, MuonPRDSelectionToolCfg
    from MuonConfig.MuonTrackBuildingConfig import MuonChamberHoleRecoveryToolCfg
    from MuonConfig.MuonRecToolsConfig import MuonAmbiProcessorCfg, MuonSeededSegmentFinderCfg
    from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import MdtDriftCircleOnTrackCreatorCfg
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg
    kwargs.setdefault("DoSummary", flags.Muon.printSummary)
    kwargs.setdefault("ConsideredPDGs", [13, -13, 1000015, -1000015])
    kwargs.setdefault("DoTruth", flags.Input.isMC)
    kwargs.setdefault("AddMDTExtrasMuGirlLowBeta", flags.MuonCombined.addMDTExtrasMuGirlLowBeta)

    result = ComponentAccumulator() 
    result.merge(MuonLayerHoughAlgCfg(flags))
    # Not setting up MuonIdHelperSvc nor MuonEDMHelperSvc
    kwargs.setdefault("MuonEDMPrinterTool", result.getPrimaryAndMerge(MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("MuonPRDSelectionTool", result.popToolsAndMerge(MuonPRDSelectionToolCfg(flags)))

    # This is going to be used in a few tools below
    staurotcreator = result.popToolsAndMerge(MdtDriftCircleOnTrackCreatorStauCfg(flags))
    kwargs.setdefault("MuonPRDSelectionToolStau",
                      result.popToolsAndMerge(MuonPRDSelectionToolCfg(flags,
                                                                      "MuonPRDSelectionToolStau",
                                                                      MdtDriftCircleOnTrackCreator=staurotcreator)))

    segmentmaker = result.popToolsAndMerge(DCMathSegmentMakerCfg(
        flags, name="DCMathStauSegmentMaker", MdtCreator=staurotcreator))
    # segmentmaker also used by MuonSeededSegmentFinder below
    kwargs.setdefault("MuonSegmentMaker", segmentmaker)
    kwargs.setdefault("MuonSegmentMakerT0Fit", result.popToolsAndMerge(DCMathSegmentMakerCfg(
        flags, name="DCMathT0FitSegmentMaker", doSegmentT0Fit=True)))
    # ^ doSegmentT0Fit overrides several defaults, including MdtCreatorT0 and MdtSegmentFinder

    kwargs.setdefault("MuonLayerSegmentMatchingTool", result.popToolsAndMerge(
        MuonLayerSegmentMatchingToolCfg(flags)))

    # Not configuring MuonRecoValidationTool as it is off by default, but it would need configuring if used
    kwargs.setdefault("TrackAmbiguityProcessor",
                      result.popToolsAndMerge(MuonAmbiProcessorCfg(flags)))
    # I don't believe MuonHitTimingTool needs configuration.
    kwargs.setdefault("MuonPRDSelectionTool", result.popToolsAndMerge(
        MuonPRDSelectionToolCfg(flags)))
    kwargs.setdefault("MuonPRDSelectionToolStau", result.popToolsAndMerge(
        MuonPRDSelectionToolCfg(flags, MdtDriftCircleOnTrackCreator=staurotcreator)))
    kwargs.setdefault("MdtDriftCircleOnTrackCreator",   result.popToolsAndMerge(
        MdtDriftCircleOnTrackCreatorCfg(flags)))
    kwargs.setdefault("MdtDriftCircleOnTrackCreatorStau", staurotcreator)
    # Now setup MuonInsideOutRecoTool property of MuonStauRecoTool. Long chain here! Could split for clarity. Another option would be to have a Stau flag on
    # shared tool functions.
    chamberholerecoverytool = result.popToolsAndMerge(MuonChamberHoleRecoveryToolCfg(flags,
                                                                                     sTgcPrepDataContainer="",
                                                                                     MMPrepDataContainer=""))
    seededsegmentfinder = result.popToolsAndMerge(MuonSeededSegmentFinderCfg(flags, name="MuonStauSeededSegmentFinder", MdtRotCreator=staurotcreator,
                                                                             SegmentMaker=segmentmaker, SegmentMakerNoHoles=segmentmaker))
    fitter = result.popToolsAndMerge(CombinedMuonTrackBuilderFitCfg(
        flags, name="CombinedStauTrackBuilderFit", MdtRotCreator=staurotcreator))
    from TrkConfig.TrkTrackSummaryToolConfig import MuonCombinedTrackSummaryToolCfg
    muon_combined_track_summary = result.popToolsAndMerge(
        MuonCombinedTrackSummaryToolCfg(flags))
    muidsegmentregionrecovery = result.popToolsAndMerge(MuonSegmentRegionRecoveryToolCfg(flags, name="MuonStauSegmentRegionRecoveryTool", SeededSegmentFinder=seededsegmentfinder,
                                                                                         RecoverMM=False, RecoverSTGC=False, MMRegionSelector="", STGCRegionSelector="",
                                                                                         ChamberHoleRecoveryTool=chamberholerecoverytool, Fitter=fitter, TrackSummaryTool=muon_combined_track_summary))
    trackbuilder = result.popToolsAndMerge(CombinedMuonTrackBuilderCfg(
        flags, name="CombinedStauTrackBuilder", MdtRotCreator=staurotcreator, MuonHoleRecovery=muidsegmentregionrecovery))
    muoncandidatetrackbuilder = CompFactory.Muon.MuonCandidateTrackBuilderTool(
        name="MuonStauCandidateTrackBuilderTool", MuonTrackBuilder=trackbuilder)
    kwargs.setdefault("MuonInsideOutRecoTool", result.popToolsAndMerge(
        MuonInsideOutRecoToolCfg(flags, name='MuonStauInsideOutRecoTool', MuonCandidateTrackBuilderTool=muoncandidatetrackbuilder)))
    # Rest

    tool = CompFactory.MuonCombined.MuonStauRecoTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def MuonSystemExtensionToolCfg(flags, **kwargs):
    result = ComponentAccumulator()

    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg

    kwargs.setdefault("Extrapolator", result.popToolsAndMerge(AtlasExtrapolatorCfg(flags)))
    kwargs.setdefault("Printer", result.addPublicTool(result.popToolsAndMerge(MuonEDMPrinterToolCfg(flags))) )
    kwargs.setdefault("ParticleCaloExtensionTool",
                        result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags, 
                                                                             name='MuonParticleCaloExtensionTool')))

    muon_ext_tool = CompFactory.Muon.MuonSystemExtensionTool("MuonSystemExtensionTool", **kwargs)
    result.setPrivateTools(muon_ext_tool)
    return result


def MuTagMatchingToolCfg(flags, name='MuTagMatchingTool', **kwargs):
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg

    #TODO: defaults in cxx
    kwargs.setdefault("AssumeLocalErrors", True)
    kwargs.setdefault("PhiCut", 30.)
    kwargs.setdefault("GlobalPhiCut", 1.)
    kwargs.setdefault("ThetaCut", 5.)
    kwargs.setdefault("GlobalThetaCut", 0.5)
    kwargs.setdefault("ThetaAngleCut", 5.)
    kwargs.setdefault("DoDistanceCut", True)
    kwargs.setdefault("CombinedPullCut", 3.0)

    result = AtlasExtrapolatorCfg(flags)
    kwargs.setdefault("IExtrapolator", result.popPrivateTools())
    kwargs.setdefault("Propagator", result.popToolsAndMerge(
        RungeKuttaPropagatorCfg(flags)))
    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("MuonSegmentHitSummary", result.popToolsAndMerge(
        MuonSegmentHitSummaryToolCfg(flags)))
    kwargs.setdefault("MuonSegmentSelection", result.popToolsAndMerge(
        MuonSegmentSelectionToolCfg(flags)))

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", result.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))
    
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
        TrackingGeometryCondAlgCfg)
    acc = TrackingGeometryCondAlgCfg(flags)
    geom_cond_key = acc.getPrimary().TrackingGeometryWriteKey
    result.merge(acc)
    kwargs.setdefault("TrackingGeometryReadKey", geom_cond_key)

    tool = CompFactory.MuTagMatchingTool(name, **kwargs)

    result.addPublicTool(tool, primary=True)
    return result


def MuTagAmbiguitySolverToolCfg(flags, name='MuTagAmbiguitySolverTool', **kwargs):
    from MuonConfig.MuonTrackBuildingConfig import MuonSegmentMatchingToolCfg
    #TODO: defaults in cxx
    kwargs.setdefault("RejectOuterEndcap", True)
    kwargs.setdefault("RejectMatchPhi", True)
    result = MuonEDMPrinterToolCfg(flags)
    kwargs.setdefault("Printer", result.popPrivateTools())
    kwargs.setdefault("MuonSegmentMatchingTool", result.popToolsAndMerge(
        MuonSegmentMatchingToolCfg(flags, name='MuonSegmentMatchingTool', doPhiMatching=False)))
    # EJWM. Not sure where doPhiMatching is set to False in old, but this is what I see in configuration diffs

    tool = CompFactory.MuTagAmbiguitySolverTool(name, **kwargs)
    result.addPublicTool(tool, primary=True)
    return result


def MuonSegmentTagToolCfg(flags, name="MuonSegmentTagTool", **kwargs):
    result = ComponentAccumulator()
    mu_tag_matching = result.getPrimaryAndMerge(MuTagMatchingToolCfg(flags))
    kwargs.setdefault("MuTagMatchingTool", mu_tag_matching)

    kwargs.setdefault("Printer", result.getPrimaryAndMerge(
        MuonEDMPrinterToolCfg(flags)))
    kwargs.setdefault("MuTagAmbiguitySolverTool", result.getPrimaryAndMerge(
        MuTagAmbiguitySolverToolCfg(flags)))
    kwargs.setdefault("MuonSegmentSelectionTool", result.getPrimaryAndMerge(
        MuonSegmentSelectionToolCfg(flags)))
    kwargs.setdefault("MuonSegmentHitSummaryTool", result.getPrimaryAndMerge(
        MuonSegmentHitSummaryToolCfg(flags)))

    result.setPrivateTools(
        CompFactory.MuonCombined.MuonSegmentTagTool(name, **kwargs))
    return result