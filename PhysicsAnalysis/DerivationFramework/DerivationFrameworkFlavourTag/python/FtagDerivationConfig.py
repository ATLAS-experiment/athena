# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations

from typing import Any, TYPE_CHECKING

import ParticleJetTools.ParentDecoratorConfig as parent_decorator_config
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
    InDetTrackTruthOriginToolCfg,
)
from ParticleJetTools.ParticleJetToolsConfig import getJetDeltaRFlavorLabelTool

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def HLTJetFTagDecorationCfg(
    flags: AthConfigFlags,
    name: str = "hltJetLabelingAlg",
    jet_container: str = "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf",
) -> ComponentAccumulator:
    """Configure HLT jet flavour-label decoration."""
    acc = ComponentAccumulator()

    jet_decoration_alg = CompFactory.JetDecorationAlg(
        name=name,
        JetContainer=jet_container,
        Decorators=[getJetDeltaRFlavorLabelTool()],
    )
    acc.addEventAlgo(jet_decoration_alg)

    return acc


def TrackTruthDecoratorCfg(flags: AthConfigFlags) -> ComponentAccumulator:
    """Decorate tracks with detailed truth information."""
    acc = ComponentAccumulator()

    if not flags.Input.isMC:
        return acc

    track_truth_origin_tool = acc.popToolsAndMerge(InDetTrackTruthOriginToolCfg(flags))

    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.TruthParticleDecoratorAlg(
            "TruthParticleDecoratorAlg",
            trackTruthOriginTool=track_truth_origin_tool,
        )
    )

    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.TrackTruthDecoratorAlg(
            "TrackTruthDecoratorAlg",
            trackContainer=_get_track_collection(flags),
            trackTruthOriginTool=track_truth_origin_tool,
            truthLeptonTool=CompFactory.TruthClassificationTool("TruthClassificationTool"),
        )
    )

    return acc


def _get_track_collection(flags: AthConfigFlags) -> str:
    """Return the track-particle container name."""
    if flags.BTagging.Pseudotrack:
        return "InDetPseudoTrackParticles"

    return "InDetTrackParticles"


def ParentDecoratorCfg(
    flags: AthConfigFlags,
    prefix: str = "",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure truth-parent decorators for the FTAG derivations."""
    cfg = ComponentAccumulator()

    cfg.merge(
        parent_decorator_config.HiggsParentDecoratorCfg(
            flags,
            name=prefix + "HiggsParentDecoratorAlg",
            **kwargs,
        )
    )
    cfg.merge(
        parent_decorator_config.ZParentDecoratorCfg(
            flags,
            name=prefix + "ZParentDecoratorAlg",
            **kwargs,
        )
    )
    cfg.merge(
        parent_decorator_config.ScalarParentDecoratorCfg(
            flags,
            name=prefix + "ScalarParentDecoratorAlg",
            **kwargs,
        )
    )
    cfg.merge(
        parent_decorator_config.TopParentDecoratorCfg(
            flags,
            name=prefix + "TopParentDecoratorAlg",
            **kwargs,
        )
    )

    return cfg
