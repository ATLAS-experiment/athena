# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Variable-R EMPFlow jets for soft flavour-tagging studies.

Defines the jet collection, its reconstruction and soft-lepton association, and the
per-jet truth augmentation a derivation needs to apply to it. Exploratory content:
the nominal small-R jets are not affected.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    get_matching_variable_names,
    get_truth_label_names,
)
from DerivationFrameworkFlavourTag.FtagDerivationConfig import ParentDecoratorCfg
from FlavorTagDiscriminants.FTagElectronAssociationConfig import FTagElectronAssociationCfg
from FlavorTagDiscriminants.FTagMuonAssociationConfig import FTagMuonAssociationCfg
from JetRecConfig.JetDefinition import (
    JetDefinition,
    JetInputConstit,
    JetInputExternal,
    JetModifier,
)
from JetRecConfig.JetRecCommon import isMC
from JetRecConfig.JetRecConfig import JetRecCfg
from JetRecConfig.StandardJetConstits import stdConstitDic as cst
from JetRecConfig.StandardJetConstits import stdInputExtDic
from JetRecConfig.StandardJetMods import stdJetModifiers
from JetRecConfig.StandardSmallRJets import flavourghosts, standardghosts
from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg
from ParticleJetTools.ParticleJetToolsConfig import (
    getJetDeltaRFlavorLabelTool,
    getJetGhostFlavorLabelTool,
    getJetQuarkChargeTool,
)
from xAODBase.xAODType import xAODType

if TYPE_CHECKING:
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper


# The standard heavy-hadron truth-label containers are built with PtMin = 5 GeV, which drops
# the hadrons that would label soft jets (roughly a quarter of all c-hadrons). Those containers
# are shared with every other jet collection, so lifting the cut in place would change the
# nominal labels. Build parallel, uncut copies instead and label the VR jets from those; the
# standard containers, ghosts and labels are untouched.
_NOPTCUT_HADRONS = ("BHadronsFinal", "CHadronsFinal", "BHadronsInitial", "CHadronsInitial")


def _build_noptcut_labelled_truth(parentjetdef: object, spec: str) -> object:
    """Copy heavy-flavour truth-label particles with no pT cut."""
    del parentjetdef
    tool = CompFactory.CopyFlavorLabelTruthParticles(
        f"truthpartcopy_{spec}NoPtCut",
        ParticleType=spec,
        PtMin=0.0,
        OutputName=f"TruthLabel{spec}NoPtCut",
    )
    return CompFactory.JetAlgorithm(f"jetalg_truthlabelcopy_{spec}NoPtCut", Tools=[tool])


for _hadron in _NOPTCUT_HADRONS:
    stdInputExtDic[f"TruthLabel{_hadron}NoPtCut"] = JetInputExternal(
        f"TruthLabel{_hadron}NoPtCut",
        xAODType.TruthParticle,
        algoBuilder=_build_noptcut_labelled_truth,
        filterfn=isMC,
        specs=_hadron,
    )
    cst[f"{_hadron}NoPtCut"] = JetInputConstit(
        f"{_hadron}NoPtCut", xAODType.TruthParticle, f"TruthLabel{_hadron}NoPtCut"
    )


def _noptcut_deltar_label_tool(jetdef: object, modspec: str) -> object:
    """Cone-matching flavour label from the uncut final-hadron containers."""
    del jetdef
    tool = getJetDeltaRFlavorLabelTool("jetdrlabeler_noptcut", float(modspec))
    tool.PartPtMin = 0.0
    tool.BParticleCollection = "TruthLabelBHadronsFinalNoPtCut"
    tool.CParticleCollection = "TruthLabelCHadronsFinalNoPtCut"
    return tool


def _noptcut_deltar_initial_label_tool(jetdef: object, modspec: str) -> object:
    """Cone-matching flavour label from the uncut initial-hadron containers."""
    del jetdef
    tool = getJetDeltaRFlavorLabelTool(
        "jetdrlabeler_noptcut", float(modspec), collection="Initial"
    )
    tool.PartPtMin = 0.0
    tool.BParticleCollection = "TruthLabelBHadronsInitialNoPtCut"
    tool.CParticleCollection = "TruthLabelCHadronsInitialNoPtCut"
    return tool


def _noptcut_ghost_label_tool(jetdef: object, modspec: str) -> object:
    """Ghost-based flavour label from the uncut final-hadron ghosts."""
    del jetdef, modspec
    tool = getJetGhostFlavorLabelTool("jetghostlabeler_noptcut")
    tool.PartPtMin = 0.0
    tool.GhostBName = "GhostBHadronsFinalNoPtCut"
    tool.GhostCName = "GhostCHadronsFinalNoPtCut"
    return tool


def _noptcut_ghost_initial_label_tool(jetdef: object, modspec: str) -> object:
    """Ghost-based flavour label from the uncut initial-hadron ghosts."""
    del jetdef, modspec
    tool = getJetGhostFlavorLabelTool("jetghostlabeler_noptcut", collection="Initial")
    tool.PartPtMin = 0.0
    tool.GhostBName = "GhostBHadronsInitialNoPtCut"
    tool.GhostCName = "GhostCHadronsInitialNoPtCut"
    return tool


_NOPTCUT_FINAL_GHOSTS = [
    "ghost:BHadronsFinalNoPtCut",
    "ghost:CHadronsFinalNoPtCut",
    "ghost:TausFinal",
]
_NOPTCUT_INITIAL_GHOSTS = [
    "ghost:BHadronsInitialNoPtCut",
    "ghost:CHadronsInitialNoPtCut",
    "ghost:TausFinal",
]

stdJetModifiers.update(
    JetDeltaRLabelNoPtCut=JetModifier(
        "ParticleJetDeltaRLabelTool",
        "jetdrlabeler_noptcut",
        createfn=_noptcut_deltar_label_tool,
        prereqs=_NOPTCUT_FINAL_GHOSTS,
    ),
    JetGhostLabelNoPtCut=JetModifier(
        "ParticleJetGhostLabelTool",
        "jetghostlabeler_noptcut",
        createfn=_noptcut_ghost_label_tool,
        prereqs=_NOPTCUT_FINAL_GHOSTS,
    ),
    JetDeltaRInitialLabelNoPtCut=JetModifier(
        "ParticleJetDeltaRLabelTool",
        "jetdrlabelerinitial_noptcut",
        createfn=_noptcut_deltar_initial_label_tool,
        prereqs=_NOPTCUT_INITIAL_GHOSTS,
    ),
    JetGhostInitialLabelNoPtCut=JetModifier(
        "ParticleJetGhostLabelTool",
        "jetghostlabelerinitial_noptcut",
        createfn=_noptcut_ghost_initial_label_tool,
        prereqs=_NOPTCUT_INITIAL_GHOSTS,
    ),
    # The standard JetQuarkChargeLabel would pull in the cut JetGhost{,Initial}Label as
    # prereqs, and those write the same decorations as the NoPtCut tools.
    JetQuarkChargeLabelNoPtCut=JetModifier(
        "JetQuarkChargeLabelingTool",
        "jetquarkchargetool_noptcut",
        createfn=getJetQuarkChargeTool,
        prereqs=[
            "mod:JetGhostInitialLabelNoPtCut",
            "mod:JetGhostLabelNoPtCut",
            "mod:PartonTruthLabel",
        ],
    ),
)

_VR_FTAG_TRUTH_MODS = (
    "PartonTruthLabel",
    "JetDeltaRLabelNoPtCut:5000", "JetGhostLabelNoPtCut",
    "JetDeltaRInitialLabelNoPtCut:5000", "JetGhostInitialLabelNoPtCut",
    "JetQuarkChargeLabelNoPtCut",
)

# Radius is wide at low pT (Rmax=1.0) and shrinks to the Rmin=0.4 floor at high pT
# (rho=20 GeV), so the cone only varies over 20-50 GeV. Same GPFlow constituents as
# nominal AntiKt4EMPFlow, so the neutral PFO come along as constituents. Calibration
# and JVT are dropped (no JES exists for a variable-R EMPFlow collection); the full
# FTAG flavour-labelling chain is kept.
AntiKtVR20Rmax10Rmin4EMPFlow = JetDefinition(
    "AntiKt", 1.0, cst.GPFlow,
    VRMinR=0.4,
    VRMassSc=20000,
    ptmin=5000,
    ghostdefs=standardghosts + flavourghosts + [f"{h}NoPtCut" for h in _NOPTCUT_HADRONS],
    # No JetPtAssociation: it derives its truth container from the radius, which for a VR
    # jet is Rmax (-> AntiKt10TruthJets, not scheduled here), and hard-fails the JetRecAlg.
    modifiers=("ConstitFourMom", "CaloEnergies", "Sort", "numConstit",
               "Width", "TrackMoments", "TrackSumMoments", "vr") + _VR_FTAG_TRUTH_MODS,
    lock=True,
)

VR_JETS = AntiKtVR20Rmax10Rmin4EMPFlow.fullname()

# The truth-vertex summary alg takes a single fixed dR, so summarise at Rmax.
VR_JET_TRUTH_VERTEX_DR = AntiKtVR20Rmax10Rmin4EMPFlow.radius

# Built in-job, so the slimming helper needs the types declared to write them out.
VR_JET_APPEND_TO_DICTIONARY = {
    VR_JETS: "xAOD::JetContainer",
    f"{VR_JETS}Aux": "xAOD::JetAuxContainer",
}


def VRFtagJetsCfg(
    flags: AthConfigFlags,
    jetCollection: str = VR_JETS,
) -> ComponentAccumulator:
    """Build the variable-R EMPFlow jets and associate soft muons/electrons.

    The jets carry GhostTrack/GhostTruth and the full FTAG truth labelling from jet
    reco; the two ghost-lepton algs key off GhostTrack to decorate FTagMuons/FTagElectrons.
    No in-athena GNN is run here.
    """
    acc = ComponentAccumulator()
    acc.merge(JetRecCfg(flags, AntiKtVR20Rmax10Rmin4EMPFlow))
    acc.merge(FTagMuonAssociationCfg(flags, jetCollection=jetCollection))
    acc.merge(FTagElectronAssociationCfg(flags, jetCollection=jetCollection))
    return acc


def add_vr_jet_truth_augmentation(
    flags: AthConfigFlags,
    acc: ComponentAccumulator,
    slimming_helper: SlimmingHelper,
    jet_collection: str = VR_JETS,
) -> None:
    """Apply the per-jet FTAG truth augmentation the nominal jets receive.

    Schedules the matched flavour labels and the parent-truth masks. The shared global
    truth decorators are scheduled once by ``add_common_augmentation`` and are not repeated.
    """
    acc.merge(
        JetMatchingCfg(
            flags, target=jet_collection, ints_to_copy=get_truth_label_names(flags)
        )
    )
    slimming_helper.ExtraVariables += [
        ".".join([jet_collection] + get_matching_variable_names(flags, jet_collection))
    ]

    if not flags.Input.isMC:
        return

    # A distinct prefix keeps the alg names apart from the nominal "PFlow" pass.
    acc.merge(
        ParentDecoratorCfg(
            flags, targetContainer=jet_collection, prefix="PFlowVR", matchDeltaR=0.3
        )
    )
