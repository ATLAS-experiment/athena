# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TgcL0BitwiseCandidateBuilderToolCfg(
    flags, name="L1Muon.TgcL0BitwiseCandidateBuilderTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.L1Muon.TgcL0BitwiseCandidateBuilderTool(
        name, **kwargs
    ))
    return result


def TgcL0BitwiseInnerCoincidenceToolCfg(
    flags, name="L1Muon.TgcL0BitwiseInnerCoincidenceTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.L1Muon.TgcL0BitwiseInnerCoincidenceTool(
        name, **kwargs
    ))
    return result


def TgcL0BitwiseTrackSelectorToolCfg(
    flags, name="L1Muon.TgcL0BitwiseTrackSelectorTool", **kwargs
):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.L1Muon.TgcL0BitwiseTrackSelectorTool(
        name, **kwargs
    ))
    return result
