# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TgcL0FloatingCandidateBuilderToolCfg(
    flags, name="L0Muon.TgcL0FloatingCandidateBuilderTool", **kwargs
):
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg

    result.merge(TGCCablingConfigCfg(flags))

    if kwargs.get("EnableTruthValidation", False):
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
            TrackingGeometryCondAlgCfg,
        )
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

        result.merge(TrackingGeometryCondAlgCfg(flags))
        extrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
        # Truth extrapolation is diagnostic only.  Keep its verbose internal
        # messages out of the candidate-builder DEBUG log while preserving
        # warnings and errors.
        from AthenaCommon.Constants import WARNING

        extrapolator.OutputLevel = WARNING
        kwargs.setdefault("TrackExtrapolator", extrapolator)

    result.setPrivateTools(
        CompFactory.L0Muon.TgcL0FloatingCandidateBuilderTool(name, **kwargs)
    )
    return result


def TgcL0FloatingInnerCoincidenceToolCfg(
    flags, name="L0Muon.TgcL0FloatingInnerCoincidenceTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L0Muon.TgcL0FloatingInnerCoincidenceTool(name, **kwargs)
    )
    return result


def TgcL0FloatingTrackSelectorToolCfg(
    flags, name="L0Muon.TgcL0FloatingTrackSelectorTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L0Muon.TgcL0FloatingTrackSelectorTool(name, **kwargs)
    )
    return result
