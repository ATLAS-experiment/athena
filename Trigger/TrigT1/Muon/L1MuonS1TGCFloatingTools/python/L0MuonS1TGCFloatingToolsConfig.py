# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TgcL0FloatingCandidateBuilderToolCfg(
    flags, name="L1Muon.TgcL0FloatingCandidateBuilderTool", **kwargs
):
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg

    result.merge(TGCCablingConfigCfg(flags))

    result.setPrivateTools(
        CompFactory.L1Muon.TgcL0FloatingCandidateBuilderTool(name, **kwargs)
    )
    return result


def TgcL0FloatingInnerCoincidenceToolCfg(
    flags, name="L1Muon.TgcL0FloatingInnerCoincidenceTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L1Muon.TgcL0FloatingInnerCoincidenceTool(name, **kwargs)
    )
    return result


def TgcL0FloatingTrackSelectorToolCfg(
    flags, name="L1Muon.TgcL0FloatingTrackSelectorTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L1Muon.TgcL0FloatingTrackSelectorTool(name, **kwargs)
    )
    return result
