# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# FTAG3.py
# Slim FTAG3 derivation for g->bb / Xbb calibration
# It requires the flag FTAG3 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from BTagging.FlavorTaggingConfig import FlavorTaggingCfg

from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    trigger_matching,
    trigger_setup,
)
from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
    xAODStringSkimmingToolCfg,
)
from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg


def FTAG3KernelCfg(flags, name="FTAG3Kernel", **kwargs):
    """Configure the derivation kernel for FTAG3."""
    acc = ComponentAccumulator()

    # Common physics augmentations only
    acc.merge(
        PhysCommonAugmentationsCfg(
            flags=flags,
            TriggerListsHelper=kwargs["TriggerListsHelper"],
        )
    )

    # Setup the tool lists for the derivation kernel
    augmentationTools = []
    skimmingTools = []
    thinningTools = []

    # Require at least one muon
    lepton_skimming_expression = (
        "count( (Muons.pt > 5*GeV) && "
        "(0 == Muons.muonType || 1 == Muons.muonType || 4 == Muons.muonType) ) >= 1"
    )
    FTAG3LeptonSkimmingTool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(
            flags=flags,
            name="FTAG3LeptonSkimmingTool",
            expression=lepton_skimming_expression,
        )
    )

    # Require at least one large-R jet
    largeR_skimming_expression = (
        "count( AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.pt > 150*GeV ) >= 1"
    )
    FTAG3LargeRJetSkimmingTool = acc.getPrimaryAndMerge(
        xAODStringSkimmingToolCfg(
            flags=flags,
            name="FTAG3LargeRJetSkimmingTool",
            expression=largeR_skimming_expression,
        )
    )

    # Keep ID tracks associated to muons
    FTAG3MuonTPThinningTool = acc.getPrimaryAndMerge(
        MuonTrackParticleThinningCfg(
            flags=flags,
            name="FTAG3MuonTPThinningTool",
            StreamName=kwargs["StreamName"],
            MuonKey="Muons",
            InDetTrackParticlesKey="InDetTrackParticles",
        )
    )

    skimmingTools += [FTAG3LargeRJetSkimmingTool, FTAG3LeptonSkimmingTool]
    thinningTools += [FTAG3MuonTPThinningTool]

    for tool in augmentationTools:
        acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(tool.name+"Aug", AugmentationTools = [tool]))
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(
        DerivationKernel(
            name=name,
            SkimmingTools=skimmingTools,
            ThinningTools=thinningTools,
        )
    )

    return acc


def FTAG3ExtraContentCfg(flags):
    """Configure FTAG3-specific extra augmentation content."""
    acc = ComponentAccumulator()

    # Needed so FTAG decorations exist on VR track jets
    acc.merge(FlavorTaggingCfg(flags, "AntiKtVR30Rmax4Rmin02PV0TrackJets"))

    return acc


def FTAG3SlimmingCfg(flags, name_tag="FTAG3", TriggerListsHelper=None):
    """Configure the slim FTAG3 output content."""
    acc = ComponentAccumulator()

    slimming_helper = SlimmingHelper(
        inputName=f"{name_tag}SlimmingHelper",
        flags=flags,
        NamesAndTypes=flags.Input.TypedCollections,
    )

    # Add truth
    if flags.Input.isMC:
        slimming_helper.AllVariables += [
            "TruthBoson",
            "TruthBosonsWithDecayParticles",
            "TruthBosonsWithDecayVertices",
            "TruthBottom",
            "TruthBSMWithDecayParticles",
            "TruthEvents",
            "TruthMuons",
            "TruthParticles",
            "TruthVertices",
        ]

        # Add truth jet collections to smart collection
        slimming_helper.SmartCollections += [
            "AntiKt10TruthSoftDropBeta100Zcut10Jets",
            "AntiKt4TruthDressedWZJets",
        ]

    # Smart collections for reco
    slimming_helper.SmartCollections += [
        "EventInfo",
        "PrimaryVertices",
        "InDetTrackParticles",
        "Muons",
        "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "AntiKt4EMPFlowJets",
        "AntiKtVR30Rmax4Rmin02PV0TrackJets",
    ]

    # Extra variables used for the calibration
    slimming_helper.ExtraVariables += [
        (
            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets."
            "GhostBHadronsFinalCount.GhostBHadronsFinalPt."
            "GhostCHadronsFinalCount.GhostCHadronsFinalPt."
            "GhostAntiKtVR30Rmax4Rmin02PV0TrackJetsCount."
            "Tau1.Tau2.Tau3.Tau4."
            "Tau1_wta.Tau2_wta.Tau3_wta."
            "C2.D2."
            "Split12.Split23.Split34."
            "Angularity.PlanarFlow.Aplanarity.Sphericity."
            "ZCut12.ZCut23.ZCut34."
            "KtDR."
            "N2.N3.M2."
            "L1.L2.L3.L4.L5."
            "ThrustMin.ThrustMaj."
            "FoxWolfram0.FoxWolfram1.FoxWolfram2.FoxWolfram3.FoxWolfram4"
        ),
        "AntiKt4EMPFlowJets.SV1_masssvx.SV1_NGTinSvx",
        "AntiKtVR30Rmax4Rmin02PV0TrackJets.SV1_masssvx.SV1_NGTinSvx",
    ]

    # GN3XV00 scores on large-R jets
    slimming_helper.ExtraVariables += [
        (
            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets."
            "GN3XV00_phtautauhad.GN3XV00_phbb.GN3XV00_phcc."
            "GN3XV00_ptop.GN3XV00_pqcdbb.GN3XV00_pqcdbx."
            "GN3XV00_pqcdcx.GN3XV00_pqcdll.GN3XV00_pWqq"
        )
    ]

    # Trigger setup and matching
    trigger_setup(slimming_helper=slimming_helper)
    trigger_matching(
        flags=flags,
        slimming_helper=slimming_helper,
        trigger_lists_helper=TriggerListsHelper,
    )

    # Define the output streams
    item_list = slimming_helper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            f"DAOD_{name_tag}",
            ItemList=item_list,
            AcceptAlgs=[f"{name_tag}Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            f"DAOD_{name_tag}",
            AcceptAlgs=[f"{name_tag}Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc


def FTAG3Cfg(flags, trigger_lists_helper=None):
    """Configure DAOD_FTAG3."""
    acc = ComponentAccumulator()

    # Get the trigger list helper
    if trigger_lists_helper is None:
        trigger_lists_helper = TriggerListsHelper(flags)

    name_tag = "FTAG3"
    stream_name = f"StreamDAOD_{name_tag}"

    # Add the kernel to CA
    acc.merge(
        FTAG3KernelCfg(
            flags=flags,
            name=f"{name_tag}Kernel",
            StreamName=stream_name,
            TriggerListsHelper=trigger_lists_helper,
        )
    )

    # Add extra content
    acc.merge(FTAG3ExtraContentCfg(flags))

    # Add the slimming
    acc.merge(
        FTAG3SlimmingCfg(
            flags,
            name_tag=name_tag,
            TriggerListsHelper=trigger_lists_helper,
        )
    )

    return acc
