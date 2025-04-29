# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of InDetExtensionProcessor package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType


def InDetExtensionProcessorCfg(flags, name="InDetExtensionProcessor", **kwargs):
    acc = ComponentAccumulator()

    if "TrackFitter" not in kwargs:
        if flags.Tracking.ActiveConfig.extension != "LowPt":
            from TrkConfig.CommonTrackFitterConfig import (
                InDetTrackFitterHoleSearchCfg)
            InDetExtensionFitter = acc.popToolsAndMerge(
                InDetTrackFitterHoleSearchCfg(
                    flags,
                    name=('InDetTrackFitter_TRTExtension' +
                          flags.Tracking.ActiveConfig.extension)))
        else:
            from TrkConfig.CommonTrackFitterConfig import (
                InDetTrackFitterLowPtHoleSearchCfg)
            InDetExtensionFitter = acc.popToolsAndMerge(
                InDetTrackFitterLowPtHoleSearchCfg(
                    flags,
                    name=('InDetTrackFitter_TRTExtension' +
                          flags.Tracking.ActiveConfig.extension)))

        acc.addPublicTool(InDetExtensionFitter)
        kwargs.setdefault("TrackFitter", InDetExtensionFitter)

    if "ScoringTool" not in kwargs:
        if flags.Beam.Type is BeamType.Cosmics:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetCosmicExtenScoringToolCfg)
            InDetExtenScoringTool = acc.popToolsAndMerge(
                InDetCosmicExtenScoringToolCfg(flags))
        else:
            from InDetConfig.InDetTrackScoringToolsConfig import (
                InDetExtenScoringToolCfg)
            InDetExtenScoringTool = acc.popToolsAndMerge(
                InDetExtenScoringToolCfg(flags))

        acc.addPublicTool(InDetExtenScoringTool)
        kwargs.setdefault("ScoringTool", InDetExtenScoringTool)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrackSummaryToolCfg(flags)))

    kwargs.setdefault("suppressHoleSearch", False)
    kwargs.setdefault("tryBremFit", flags.Tracking.ActiveConfig.doBremRecoverySi)
    kwargs.setdefault("caloSeededBrem", flags.Tracking.doCaloSeededBrem)
    kwargs.setdefault("pTminBrem", flags.Tracking.ActiveConfig.minPTBrem)
    kwargs.setdefault("RefitPrds", False)
    kwargs.setdefault("matEffects",
                      flags.Tracking.materialInteractionsType
                      if flags.Tracking.materialInteractions else 0)
    kwargs.setdefault("Cosmics", flags.Beam.Type is BeamType.Cosmics)

    acc.addEventAlgo(CompFactory.InDet.InDetExtensionProcessor(
        name + flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def TrigInDetExtensionProcessorCfg(flags, name="InDetTrigMTExtensionProcessor", **kwargs):
    acc = ComponentAccumulator()

    if "TrackFitter" not in kwargs:
        from TrkConfig.TrkGlobalChi2FitterConfig import (
            InDetTrigGlobalChi2FitterCfg)
        InDetExtensionFitter = acc.popToolsAndMerge(
            InDetTrigGlobalChi2FitterCfg(flags))
        acc.addPublicTool(InDetExtensionFitter)
        kwargs.setdefault("TrackFitter", InDetExtensionFitter)

    if "ScoringTool" not in kwargs:
        from InDetConfig.InDetTrackScoringToolsConfig import (
            TrigAmbiScoringToolCfg)
        InDetExtenScoringTool = acc.popToolsAndMerge(
            TrigAmbiScoringToolCfg(flags,
                                        name="TrigAmbiguityScoringTool"+flags.Tracking.ActiveConfig.input_name))
        acc.addPublicTool(InDetExtenScoringTool)
        kwargs.setdefault("ScoringTool", InDetExtenScoringTool)

    if "TrackSummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import (
            InDetTrigTrackSummaryToolCfg)
        kwargs.setdefault("TrackSummaryTool", acc.popToolsAndMerge(
            InDetTrigTrackSummaryToolCfg(flags)))

    kwargs.setdefault("suppressHoleSearch", False)
    kwargs.setdefault("RefitPrds",
                      not flags.Tracking.ActiveConfig.refitROT)

    kwargs.setdefault("TrackName",
                      flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb")
    kwargs.setdefault("ExtensionMap", "ExtendedTrackMap")
    kwargs.setdefault("NewTrackName",
                      flags.Tracking.ActiveConfig.trkTracks_IDTrig)

    acc.addEventAlgo(CompFactory.InDet.InDetExtensionProcessor(
        name + flags.Tracking.ActiveConfig.input_name, **kwargs))
    return acc
