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

    # Define the jet pT cut
    pflow_jet_thinning_selection = "AntiKt4EMPFlowJets.pt > 15*GeV"
    lctopo_jet_thinning_selection = "AntiKt4LCTopoJets.pt > 20*GeV"

    # Keep small-R jet content above the calibration threshold.
    pflow_jet_thinning_tool = acc.getPrimaryAndMerge(
        GenericObjectThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4EMPFlowJetsThinningTool",
            StreamName=stream_name,
            ContainerName="AntiKt4EMPFlowJets",
            SelectionString=pflow_jet_thinning_selection,
        )
    )
    lctopo_jet_thinning_tool = acc.getPrimaryAndMerge(
        GenericObjectThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4LCTopoJetsThinningTool",
            StreamName=stream_name,
            ContainerName="AntiKt4LCTopoJets",
            SelectionString=lctopo_jet_thinning_selection,
        )
    )
    thinning_tools.append(pflow_jet_thinning_tool)
    thinning_tools.append(lctopo_jet_thinning_tool)

    pflow_jet_tp_thinning_tool = acc.getPrimaryAndMerge(
        JetTrackParticleThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4EMPFlowJetsTPThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt4EMPFlowJets",
            SelectionString=pflow_jet_thinning_selection,
            InDetTrackParticlesKey="InDetTrackParticles",
        )
    )
    lctopo_jet_tp_thinning_tool = acc.getPrimaryAndMerge(
        JetTrackParticleThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4LCTopoJetsTPThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt4LCTopoJets",
            SelectionString=lctopo_jet_thinning_selection,
            InDetTrackParticlesKey="InDetTrackParticles",
        )
    )
    thinning_tools.append(pflow_jet_tp_thinning_tool)
    thinning_tools.append(lctopo_jet_tp_thinning_tool)

    ghost_tower_thinning_tool = acc.getPrimaryAndMerge(
        JetGhostThinningCfg(
            flags=flags,
            name="FTAG2AntiKt4EMPFlowJetsGhostTowerThinningTool",
            StreamName=stream_name,
            JetKey="AntiKt4EMPFlowJets",
            SelectionString=pflow_jet_thinning_selection,
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
            SelectionString=pflow_jet_thinning_selection,
            JetConstituentName="CHSG",
            GlobalConstituentName="Global",
            OtherObjectsName="CaloCalTopoClusters",
        )
    )
    thinning_tools.append(jet_constituent_thinning_tool)

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
            keep_track_covariance_offdiag=False,
        )
    )

    return acc
