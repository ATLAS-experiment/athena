# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# DAOD_FTAG2.py
#
# Dilepton-skimmed FTAG derivation for Data/MC studies
# Built from FTAG1 content plus targeted skimming and thinning.
# Requires the FTAG2 flag in Derivation_tf.py
# ====================================================================

from __future__ import annotations

from typing import Any, TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from DerivationFrameworkFlavourTag.FTAG1 import FTAG1CoreCfg, FTAG1ExtraContentCfg
from DerivationFrameworkInDet.InDetToolsConfig import (
    EgammaTrackParticleThinningCfg,
    JetConstituentThinningCfg,
    JetGhostThinningCfg,
    JetTrackParticleThinningCfg,
    MuonTrackParticleThinningCfg,
    UFOTrackParticleThinningCfg,
)
from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
    GenericObjectThinningCfg,
    xAODStringSkimmingToolCfg,
)

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def _get_two_lepton_skimming_expression() -> str:
    """Return the FTAG2 dilepton skimming expression."""
    muon_quality = "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType)"
    electron_quality = "((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))"

    return (
        f"count( (Muons.pt > 18*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 18*GeV) && {electron_quality}) >= 2 "
        "&& "
        f"count( (Muons.pt > 25*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 25*GeV) && {electron_quality}) >= 1"
    )


def _get_one_lepton_and_jets_skimming_expression() -> str:
    """Return the FTAG2 single lepton + jets skimming expression."""
    muon_quality = "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType)"
    electron_quality = "((Electrons.Loose) || (Electrons.DFCommonElectronsLHLoose))"

    return (
        f"count( (Muons.pt > 25*GeV) && {muon_quality} ) "
        f"+ count(( Electrons.pt > 25*GeV) && {electron_quality}) >= 1"
        " && "
        f"count(AntiKt4EMPFlowJets.pt > 20*GeV) >= 4"
    )


def _get_thinning_tools(
    flags: AthConfigFlags,
    acc: ComponentAccumulator,
    stream_name: str,
) -> list[Any]:
    """Configure FTAG2 thinning tools."""
    thinning_tools = []

    # Keep ID tracks associated with selected leptons.
    muon_tp_thinning_tool = acc.getPrimaryAndMerge(
        MuonTrackParticleThinningCfg(
            flags=flags,
            name="FTAG2MuonTPThinningTool",
            StreamName=stream_name,
            MuonKey="Muons",
            InDetTrackParticlesKey="InDetTrackParticles",
        )
    )
    thinning_tools.append(muon_tp_thinning_tool)

    electron_tp_thinning_tool = acc.getPrimaryAndMerge(
        EgammaTrackParticleThinningCfg(
            flags=flags,
            name="FTAG2ElectronTPThinningTool",
            StreamName=stream_name,
            SGKey="Electrons",
            InDetTrackParticlesKey="InDetTrackParticles",
        )
    )
    thinning_tools.append(electron_tp_thinning_tool)

    # Jet collections to thin, mapped to the selection defining which jets are kept.
    # Large-R jets carry no pT cut; their constituents are thinned by association.
    jet_thinning_selections = {
        "AntiKt4EMPFlowJets": "AntiKt4EMPFlowJets.pt > 15*GeV",
        "AntiKt4LCTopoJets": "AntiKt4LCTopoJets.pt > 20*GeV",
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets": "",
    }

    for jet_collection, selection in jet_thinning_selections.items():
        if selection:
            thinning_tools.append(
                acc.getPrimaryAndMerge(
                    GenericObjectThinningCfg(
                        flags=flags,
                        name=f"FTAG2{jet_collection}ThinningTool",
                        StreamName=stream_name,
                        ContainerName=jet_collection,
                        SelectionString=selection,
                    )
                )
            )
        thinning_tools.append(
            acc.getPrimaryAndMerge(
                JetTrackParticleThinningCfg(
                    flags=flags,
                    name=f"FTAG2{jet_collection}TPThinningTool",
                    StreamName=stream_name,
                    JetKey=jet_collection,
                    SelectionString=selection,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
        )

    ghost_tower_thinning_tool = acc.getPrimaryAndMerge(
        JetGhostThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4EMPFlowJetsGhostTowerThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt4EMPFlowJets",
            SelectionString=jet_thinning_selections["AntiKt4EMPFlowJets"],
            GhostName="GhostTower",
            GhostContainerName="CaloCalFwdTopoTowers",
        )
    )
    thinning_tools.append(ghost_tower_thinning_tool)

    jet_constituent_thinning_tool = acc.getPrimaryAndMerge(
        JetConstituentThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4EMPFlowJetsConstituentThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt4EMPFlowJets",
            SelectionString=jet_thinning_selections["AntiKt4EMPFlowJets"],
            JetConstituentName="CHSG",
            GlobalConstituentName="Global",
            OtherObjectsName="CaloCalTopoClusters",
        )
    )
    thinning_tools.append(jet_constituent_thinning_tool)

    large_r_constituent_thinning_tool = acc.getPrimaryAndMerge(
        UFOTrackParticleThinningCfg(
            flags=flags,
            name="FTAG2AntiKt10UFOCSSKSoftDropBeta100Zcut10JetsUFOThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
            UFOKey="UFOCSSK",
            PFOCollectionSGKey="CSSKG",
            InDetTrackParticlesKey="InDetTrackParticles",
            ThinTrackingContainer=True,
        )
    )
    thinning_tools.append(large_r_constituent_thinning_tool)

    return thinning_tools


def FTAG2KernelCfg(
    flags: AthConfigFlags,
    name: str = "FTAG2Kernel",
    **kwargs: Any,
) -> ComponentAccumulator:
    """Configure the derivation kernel for FTAG2."""
    acc = ComponentAccumulator()

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags=flags,
            TriggerListsHelper=kwargs["trigger_lists_helper"],
        )
    )

    skimming_tool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(
            flags=flags,
            name="FTAG2SkimmingTool",
            expression=(
                f"({_get_two_lepton_skimming_expression()})"
                " || "
                f"({_get_one_lepton_and_jets_skimming_expression()})"
                " || "
                "(count( AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.pt > 450*GeV ) >= 1)"
            ),
        )
    )

    # FTAG2 reuses the FTAG1 jet reconstruction additions (LCTopo)
    acc.merge(FTAG1ExtraContentCfg(flags))

    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            name=name,
            SkimmingTools=[skimming_tool],
            ThinningTools=_get_thinning_tools(
                flags=flags,
                acc=acc,
                stream_name=kwargs["stream_name"],
            ),
        )
    )

    return acc


def FTAG2Cfg(
    flags: AthConfigFlags,
    name_tag: str = "FTAG2",
) -> ComponentAccumulator:
    """Configure the full FTAG2 derivation."""
    acc = ComponentAccumulator()

    trigger_lists_helper = TriggerListsHelper(flags)
    stream_name = "StreamDAOD_" + name_tag

    acc.merge(
        FTAG2KernelCfg(
            flags=flags,
            name=name_tag + "Kernel",
            stream_name=stream_name,
            trigger_lists_helper=trigger_lists_helper,
        )
    )

    acc.merge(
        FTAG1CoreCfg(
            flags=flags,
            name_tag=name_tag,
            trigger_lists_helper=trigger_lists_helper,
            keep_truth_collections=False,
            keep_track_covariance_offdiag=True,
            tau_as_smart_collection=True,
        )
    )

    return acc
