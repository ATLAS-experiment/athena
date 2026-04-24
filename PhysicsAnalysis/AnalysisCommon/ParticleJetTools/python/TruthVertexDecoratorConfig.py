# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthVertexDecoratorCfg(flags, name="TruthVertexDecoratorAlg", **kwargs):
    """Configure TruthVertexDecoratorAlg.

    Decorates tracks and truth particles with truth vertex labels
    (vertex ID, vertex type, PV distance, etc.) using the FatVertex
    classification.
    """
    acc = ComponentAccumulator()

    from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
        InDetTrackTruthOriginToolCfg,
    )
    kwargs.setdefault(
        "trackTruthOriginTool",
        acc.popToolsAndMerge(InDetTrackTruthOriginToolCfg(flags)),
    )

    acc.addEventAlgo(
        CompFactory.ParticleJetTools.TruthVertexDecoratorAlg(name, **kwargs)
    )
    return acc


def JetTruthVertexSummaryDecoratorCfg(
    flags, name="JetTruthVertexSummaryDecoratorAlg", **kwargs
):
    """Configure JetTruthVertexSummaryDecoratorAlg.

    Decorates jets with counts of associated truth vertices by type
    (b, c, tau, strange, pion, material interaction, other).
    Reads truth particle decorations produced by TruthVertexDecoratorAlg.
    """
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.ParticleJetTools.JetTruthVertexSummaryDecoratorAlg(
            name, **kwargs
        )
    )
    return acc


def TruthVertexDecoratorsCfg(flags, jetCollections=None, **kwargs):
    """Convenience: schedule truth vertex decorator + one summary alg per jet collection.

    TruthVertexDecoratorAlg runs first (decorates truth particles once), then
    one JetTruthVertexSummaryDecoratorAlg is scheduled per entry in
    ``jetCollections`` (reads the TP decorations, writes per-jet summary
    decorations keyed to that jet container).

    Args:
        flags: Athena ConfigFlags.
        jetCollections: list of ``(jetContainer, drThreshold)`` tuples. Each
            entry schedules a summary alg reading the given jet container and
            matching truth vertices to those jets within ``drThreshold``.
            Defaults to ``[("AntiKt4EMPFlowJets", 0.4)]``.
        **kwargs: forwarded to ``TruthVertexDecoratorCfg`` (the TP decorator).
    """
    if jetCollections is None:
        jetCollections = [("AntiKt4EMPFlowJets", 0.4)]
    acc = ComponentAccumulator()
    acc.merge(TruthVertexDecoratorCfg(flags, **kwargs))
    for (jet_container, dr) in jetCollections:
        acc.merge(JetTruthVertexSummaryDecoratorCfg(
            flags,
            name=f"JetTruthVertexSummaryDecoratorAlg_{jet_container}",
            jetContainer=jet_container,
            drThreshold=dr,
        ))
    return acc
