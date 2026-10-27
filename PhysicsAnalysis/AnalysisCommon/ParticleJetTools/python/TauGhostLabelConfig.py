"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

Configuration for the FTAG ghost truth labels of taus.

The labels are taken from the seed jet of each tau and copied to the tau by the
TauSeedJetLabelCopyAlg. By default the seed jets are the standard AntiKt4LCTopo
jets of the full event, which carry the truth labels on MC, and are found
through the tau jet link. Alternatively, the TauSeedJetAlg builds the seed jets
again from the tau clusters only, with the truth ghosts associated.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.JetRecConfig import JetInputCfg, JetRecCfg
from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo
from ParticleJetTools.ParticleJetToolsConfig import getJetGhostFlavorLabelTool

TAU_GHOSTS = ["BHadronsFinal", "CHadronsFinal", "TausFinal", "Partons"]

TAU_INT_LABELS = [
    "HadronGhostTruthLabelID",
    "HadronGhostExtendedTruthLabelID",
    "HadronGhostTruthLabelPdgId",
    "HadronGhostTruthLabelBarcode",
    "HadronGhostTruthLabelChildPdgId",
    "PartonTruthLabelID",
    "PartonExtendedTruthLabelID",
] + [f"Ghost{ghost}Count" for ghost in TAU_GHOSTS]

TAU_FLOAT_LABELS = [
    f"HadronGhostTruthLabel{name}"
    for name in [
        "Pt", "Lxy", "DR", "PositionDPhi", "PositionDEta",
        "ChildLxy", "ChildPt", "ChildPositionDPhi", "ChildPositionDEta",
    ]
] + [
    "PartonTruthLabelPt",
    "PartonTruthLabelEnergy",
    "PartonTruthLabelDR",
] + [f"Ghost{ghost}Pt" for ghost in TAU_GHOSTS]


def _seed_jet_name(tau_collection):
    return tau_collection.removesuffix("Jets") + "SeedJets"


def TauSeedJetCfg(flags, tau_collection="TauJets", **kwargs):
    """Build the seed jets of a tau collection, with the ghost truth labels.

    Also schedules the origin-corrected LC clusters and the truth ghost
    inputs, which are shared with the jet reconstruction if it runs too.
    """
    acc = ComponentAccumulator()

    acc.merge(JetInputCfg(flags, cst.LCTopoOrigin))
    for ghost in TAU_GHOSTS:
        acc.merge(JetInputCfg(flags, cst[ghost]))

    kwargs.setdefault("SeedJets", _seed_jet_name(tau_collection))
    kwargs.setdefault("Constituents", "LCOriginTopoClusters")
    kwargs.setdefault("GhostCollections", [f"TruthLabel{ghost}" for ghost in TAU_GHOSTS])
    kwargs.setdefault("Modifiers", [
        getJetGhostFlavorLabelTool(),
        CompFactory.Analysis.JetPartonTruthLabel("partontruthlabel"),
    ])
    acc.addEventAlgo(
        CompFactory.ParticleJetTools.TauSeedJetAlg(
            f"TauSeedJetAlg_{tau_collection}",
            TauJets=tau_collection,
            **kwargs,
        )
    )
    return acc


def TauSeedJetLabelCopyCfg(flags, tau_collection="TauJets", **kwargs):
    """Copy the truth labels from the seed jets to the taus."""
    acc = ComponentAccumulator()

    kwargs.setdefault("SeedJets", _seed_jet_name(tau_collection))
    kwargs.setdefault("IntDecorations", TAU_INT_LABELS)
    kwargs.setdefault("FloatDecorations", TAU_FLOAT_LABELS)
    acc.addEventAlgo(
        CompFactory.ParticleJetTools.TauSeedJetLabelCopyAlg(
            f"TauSeedJetLabelCopyAlg_{tau_collection}",
            TauJets=tau_collection,
            **kwargs,
        )
    )
    return acc


def TauGhostLabelCfg(flags, tau_collection="TauJets", full_event=True, **kwargs):
    """Schedule the ghost truth labels for a tau collection.

    Does nothing on data.

    Parameters
    ----------
    flags : AthConfigFlags
        Configuration flags.
    tau_collection : str
        Taus to decorate.
    full_event : bool
        Copy the labels from the AntiKt4LCTopo seed jets of the full event,
        valid while these are the tau seed jets (Run 2 and 3). Otherwise build
        the seed jets from the tau clusters with the TauSeedJetAlg. This
        clusters each tau separately, so unlike the full event jet labelling
        it is not exclusive: one truth particle can label more than one tau,
        e.g. for close tau pairs.
    **kwargs
        Passed to the TauSeedJetAlg, only used without full_event.
    """
    acc = ComponentAccumulator()

    if not flags.Input.isMC:
        return acc

    if full_event:
        acc.merge(JetRecCfg(flags, AntiKt4LCTopo))
        acc.merge(TauSeedJetLabelCopyCfg(
            flags,
            tau_collection=tau_collection,
            SeedJets="AntiKt4LCTopoJets",
            SeedJetLink="jetLink",
        ))
        return acc

    seed_jets = kwargs.setdefault("SeedJets", _seed_jet_name(tau_collection))
    acc.merge(TauSeedJetCfg(flags, tau_collection=tau_collection, **kwargs))
    acc.merge(TauSeedJetLabelCopyCfg(flags, tau_collection=tau_collection, SeedJets=seed_jets))
    return acc
