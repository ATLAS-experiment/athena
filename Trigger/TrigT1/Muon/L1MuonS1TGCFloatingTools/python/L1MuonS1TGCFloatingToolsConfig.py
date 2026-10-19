# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TgcSLFloatingCandidateBuilderToolCfg(
    flags, name="L1Muon.L1TGC.FloatingCandidateBuilderTool", **kwargs
):
    result = ComponentAccumulator()
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg

    result.merge(TGCCablingConfigCfg(flags))

    result.setPrivateTools(
        CompFactory.L1Muon.L1TGC.FloatingCandidateBuilderTool(name, **kwargs)
    )
    return result


def TgcSLFloatingInnerCoincidenceToolCfg(
    flags, name="L1Muon.L1TGC.FloatingInnerCoincidenceTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L1Muon.L1TGC.FloatingInnerCoincidenceTool(name, **kwargs)
    )
    return result


def TgcSLFloatingTrackSelectorToolCfg(
    flags, name="L1Muon.L1TGC.FloatingTrackSelectorTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(
        CompFactory.L1Muon.L1TGC.FloatingTrackSelectorTool(name, **kwargs)
    )
    return result
