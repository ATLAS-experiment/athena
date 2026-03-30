# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG1LITE.py
# Minimal derivation for producing GN3/GN3X training samples via TDD.
#
# Built from individual augmentation modules rather than the monolithic
# PhysCommonAugmentationsCfg, so we only run what TDD actually needs.
# Skips: MET, DiTau, triggers, LRT, pixel/SCT clusters.
#
# Supports two jet collections (JET_COLLECTIONS):
#   small: AntiKt4EMPFlowJets  — full calibration + NNJvt + overlap-lepton
#   large: AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets  — as-is, no calibration
#
# See AODToFTAGTraining (TDD ca_block) for the individual augmentation
# pattern this is based on.
#====================================================================

from __future__ import annotations

import json
import os

import ROOT
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

# PassThrough JSON filenames (shipped under DerivationFrameworkFlavourTag/data
# via atlas_install_data, resolved at runtime via PathResolver on DATAPATH).
# To test a local copy without rebuilding, override via --preExec, e.g.
#   --preExec "from DerivationFrameworkFlavourTag import FTAG1LITE as m; \
#              m.PASSTHROUGH_JSON_SMALL='/abs/path/to/local.json'"
PASSTHROUGH_JSON_SMALL = "DerivationFrameworkFlavourTag/passthrough_ftag1lite_data.json"
PASSTHROUGH_JSON_SMALL_MC_EXTRA = "DerivationFrameworkFlavourTag/passthrough_ftag1lite_mc_extra.json"
PASSTHROUGH_JSON_LARGE = "DerivationFrameworkFlavourTag/passthrough_ftag1lite_largeR_data.json"
PASSTHROUGH_JSON_LARGE_MC_EXTRA = "DerivationFrameworkFlavourTag/passthrough_ftag1lite_largeR_mc_extra.json"

# Calorimeter sampling layer names in enum order (CaloSampling::CaloSample).
# Used to build the OutputNamesMap for VectorExploderAlg when exploding
# EnergyPerSampling / EnergyPerSamplingCaloBased on AntiKt4EMPFlowJets.
# These vectors are small-R only — large-R jets do not carry them.
_CALO_SAMPLING_NAMES = [
    "PreSamplerB", "EMB1", "EMB2", "EMB3",
    "PreSamplerE", "EME1", "EME2", "EME3",
    "HEC0", "HEC1", "HEC2", "HEC3",
    "TileBar0", "TileBar1", "TileBar2",
    "TileGap1", "TileGap2", "TileGap3",
    "TileExt0", "TileExt1", "TileExt2",
    "FCAL0", "FCAL1", "FCAL2",
    "MINIFCAL0", "MINIFCAL1", "MINIFCAL2", "MINIFCAL3",
]

_ptjson_cache = {}


def _load_passthrough_json(path):
    """Resolve a PassThrough JSON path and return the parsed dict.

    Accepts either a DATAPATH-relative string (e.g.
    'DerivationFrameworkFlavourTag/passthrough_ftag1lite.json') or an
    absolute path.  Result is cached per-path within the process.
    """
    if path in _ptjson_cache:
        return _ptjson_cache[path]
    resolved = path
    if not os.path.isabs(resolved):
        # DATAPATH hosts files shipped via atlas_install_data (i.e.
        # InstallArea/data/<package>/<file>).  CALIBPATH (used by
        # FindCalibFile) only resolves CVMFS calib-area paths and will
        # not find package-shipped JSONs.
        resolved = ROOT.PathResolver.find_file(resolved, "DATAPATH")
    with open(resolved) as f:
        cfg = json.load(f)
    _ptjson_cache[path] = cfg
    return cfg


# Jet collection registry — one entry per collection FTAG1LITE processes.
# `small` gets NNJvt + lepton-overlap cuts + calibration decorator.
# `large` is stored as-is; analysis-time calibration only.
#
# The `thinning` selection string lives in each PassThrough JSON under the
# top-level "thinning" key; it uses `{jet}` as a placeholder for the
# container name and is substituted at config time.
JET_COLLECTIONS = {
    "small": {
        "name": "AntiKt4EMPFlowJets",
        "calibrate": True,
        "nnjvt": True,
        "overlap_lepton": True,
        "matching": True,
        "passthrough_json": PASSTHROUGH_JSON_SMALL,
        "passthrough_json_mc_extra": PASSTHROUGH_JSON_SMALL_MC_EXTRA,
        "ghost_muons": True,
        "soft_electron_selection": True,
    },
    "large": {
        "name": "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
        "calibrate": False,
        "nnjvt": False,
        "overlap_lepton": False,
        "matching": False,
        "passthrough_json": PASSTHROUGH_JSON_LARGE,
        "passthrough_json_mc_extra": PASSTHROUGH_JSON_LARGE_MC_EXTRA,
        "ghost_muons": True,
        "soft_electron_selection": True,
    },
}

# Convenience alias for the small-R collection (used where JETS was referenced
# in non-looped contexts: e.g. JetLeptonDecayLabelAlg, soft-electron algs,
# ghost-muon association, truth-tau matching).
JETS = JET_COLLECTIONS["small"]["name"]


def _int_labels():
    """Truth label variable names for jet matching (from FtagBaseContent)."""
    algs = ['HadronConeExcl', 'HadronGhost']
    types = ['Extended', '']
    return [f'{a}{e}TruthLabelID' for a in algs for e in types]


def _filter_aux_vars(item_list, container, vars_to_remove, prefix_filter=None):
    """Remove specific variables from a container's Aux item list entry.

    Finds the item matching ``{container}Aux.`` and strips any variable whose
    name is in *vars_to_remove* or (if *prefix_filter* is given) starts with
    one of the supplied prefixes.  All other items pass through unchanged.
    """
    tag = f'{container}Aux.'
    cleaned = []
    for item in item_list:
        if tag in item:
            prefix, varstr = item.split('Aux.', 1)
            kept = [v for v in varstr.split('.')
                    if v not in vars_to_remove
                    and (prefix_filter is None
                         or not v.startswith(prefix_filter))]
            cleaned.append(prefix + 'Aux.' + '.'.join(kept))
        else:
            cleaned.append(item)
    return cleaned


# ── Augmentation ─────────────────────────────────────────────────────

def FTAG1LITEKernelCfg(flags, name='FTAG1LITEKernel', **kwargs):
    """Configure augmentations for FTAG1LITE.

    Composes individual config functions instead of PhysCommonAugmentationsCfg.
    Each block is documented with why FTAG1LITE needs it.
    """
    acc = ComponentAccumulator()

    # ── MC truth collections ──
    from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
        AddHFAndDownstreamParticlesCfg,
        AddMiniTruthCollectionLinksCfg,
        AddPVCollectionCfg,
        AddStandardTruthContentsCfg,
        TruthClassificationAugmentationsCfg,
    )
    if flags.Input.isMC:
        acc.merge(TruthClassificationAugmentationsCfg(flags))

        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthCharmCfg
        acc.merge(DFCommonTruthCharmCfg(flags))

        acc.merge(AddHFAndDownstreamParticlesCfg(flags))
        acc.merge(AddStandardTruthContentsCfg(
            flags,
            navInputCollections=[
                "TruthElectrons", "TruthMuons", "TruthPhotons",
                "TruthTaus", "TruthNeutrinos", "TruthBSM",
                "TruthBottom", "TruthTop", "TruthBoson",
                "TruthCharm", "TruthHFWithDecayParticles",
            ],
        ))
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))

    # ── Inner detector ──
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    acc.merge(InDetCommonCfg(
        flags,
        DoVertexFinding=flags.Tracking.doVertexFinding,
        AddPseudoTracks=flags.Tracking.doPseudoTracking,
        DecoLRTTTVA=False,
        DoR3LargeD0=False,
        StoreSeparateLargeD0Container=False,
        MergeLRT=False,
    ))

    # ── Muons ──
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    acc.merge(MuonsCommonCfg(flags))

    # ── Electrons/photons ──
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc.merge(EGammaCommonCfg(flags))

    # ── Jets ──
    from DerivationFrameworkJetEtMiss.JetCommonConfig import JetCommonCfg
    acc.merge(JetCommonCfg(flags))

    # ── PFlow linking ──
    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    acc.merge(PFGlobalFlowElementLinkingCfg(flags))

    # ── Flavour tagging ──
    if flags.Reco.EnableBTagging:
        from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
        for cfg in JET_COLLECTIONS.values():
            acc.merge(FlavorTaggingCfg(flags, cfg["name"]))

    # ── Ftag-specific augmentations ──
    from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
        ParentDecoratorCfg,
        TrackTruthDecoratorCfg,
    )

    # ── NearestJet matching (reco-to-reco) ──
    # Match each jet to its nearest neighbour reco jet and copy kinematic
    # variables + truth label. This replaces TDD's NearestJet JetMatcher
    # ca_block, ensuring matching is done before thinning for stability.
    acc.merge(JetMatchingCfg(
        flags, target=JETS,
        source_name="NearestJet",
        floats_to_copy=["pt", "eta", "phi"],
        ints_to_copy=["HadronGhostTruthLabelID"] if flags.Input.isMC else [],
    ))

    # ── Tau jet matching ──
    # Match each reco jet to the nearest TauJet and copy tau tagger scores
    # as jet decorations. This replaces TDD's TauJet JetMatcher ca_block,
    # allowing TauJets to be excluded from the DAOD output.
    acc.merge(JetMatchingCfg(
        flags, target=JETS,
        sources=["TauJets"],
        source_name="TauJet",
        floats_to_copy=[
            "RNNJetScore", "RNNJetScoreSigTrans",
            "GNTauScore_v0prune", "GNTauScoreSigTrans_v0prune",
            "GNTauScoreSigTrans_v1trunc", "ptFinalCalib",
        ],
        pt_priority_with_delta_r=0.3,
    ))

    # ── Truth jet matching ──
    # Match each reco jet to the nearest truth jet and copy pt.
    # This replaces TDD's AntiKt4TruthJets and AntiKt4TruthDressedWZJets
    # JetMatcher ca_blocks, allowing those containers to be excluded from
    # the DAOD output.
    if flags.Input.isMC:
        acc.merge(JetMatchingCfg(
            flags, target=JETS,
            sources=["AntiKt4TruthJets"],
            source_name="TruthJet",
            floats_to_copy=["pt"],
            pt_priority_with_delta_r=0.3,
        ))
        acc.merge(JetMatchingCfg(
            flags, target=JETS,
            sources=["AntiKt4TruthDressedWZJets"],
            source_name="TruthDressedWZJet",
            floats_to_copy=["pt"],
            pt_priority_with_delta_r=0.3,
        ))

        acc.merge(TrackTruthDecoratorCfg(flags))
        acc.merge(ParentDecoratorCfg(
            flags, targetContainer=JETS, prefix="PFlow", matchDeltaR=0.3,
        ))

        acc.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.JetLeptonDecayLabelAlg(
                f"JetLeptonDecayLabelAlg_{JETS}",
                jetContainer=JETS,
            )
        )

        acc.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.SoftElectronTruthDecoratorAlg(
                "SoftElectronTruthDecoratorAlg",
            )
        )

    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.SoftElectronDecoratorAlg(
            "SoftElectronDecoratorAlg",
        )
    )

    # ── Soft electron selection (per-collection) ──
    # This module gets added automatically if we have any taggers which use
    # electron inputs. For upgrade samples, this is not currently the case,
    # and so the module is scheduled here.
    from FlavorTagDiscriminants.FTagElectronAssociationConfig import FTagElectronAssociationCfg
    for cfg in JET_COLLECTIONS.values():
        if cfg["soft_electron_selection"]:
            acc.merge(FTagElectronAssociationCfg(flags, cfg["name"]))
            acc.addEventAlgo(
                CompFactory.FlavorTagJetDecorators.SoftElectronSelectionAlg(
                    f"SoftElectronSelectionAlg_{cfg['name']}",
                    jetContainer=cfg["name"],
                    selectedElectronsKey=(
                        f"{cfg['name']}.GhostFTagSelectedElectrons"
                    ),
                )
            )

    # ── Ghost muon association (per-collection) ──
    # Must run before PassThrough model (MuonsLoader).
    for cfg in JET_COLLECTIONS.values():
        if cfg["ghost_muons"]:
            acc.addEventAlgo(
                CompFactory.FlavorTagDiscriminants.FTagGhostMuonAssociationAlg(
                    f"FTagGhostMuonAssociationAlg_{cfg['name']}",
                    jetContainer=cfg["name"],
                    outMuons=f"{cfg['name']}.GhostFTagMuons",
                )
            )

    # ── Calo charged flow decorator ──
    # Pre-compute usedInChargedFlow flag on CaloCalTopoClusters.
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.CaloChargedFlowDecoratorAlg(
            "CaloChargedFlowDecoratorAlg",
        )
    )

    # ── Flow energy decorator ──
    if not flags.HeavyIon.isDerivation:
        from DerivationFrameworkFlavourTag.FlowEnergyDecoratorConfig import (
            FlowEnergyDecoratorCfg,
        )
        acc.merge(FlowEnergyDecoratorCfg())

    # ── Jet calibrated pT decorator ──
    # Apply TDD-compatible calibration (JetArea_Residual_EtaJES_GSC) and
    # decorate each jet with pt_calibrated. Used for the thinning selection.
    # Calibration config differs between Run 2 and Run 3 — must match
    # JetAnalysisConfig.py (JetCalibrationBlock) to share the tool.
    if flags.GeoModel.Run is LHCPeriod.Run2:
        calibConfigFile = "PreRec_R22_PFlow_ResPU_EtaJES_GSC_February23_230215.config"
        calibArea = "00-04-82"
    else:
        calibConfigFile = ("AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_"
                           "CalibConfig_ResPU_EtaJES_GSC_241208_InSitu.config")
        calibArea = "00-04-83"
    calibTool = CompFactory.JetCalibrationTool(
        "JetCalibTool_AntiKt4EMPFlow",
        JetCollection="AntiKt4EMPFlow",
        ConfigFile=calibConfigFile,
        CalibSequence=("JetArea_Residual_EtaJES_GSC" if flags.Input.isMC
                       else "JetArea_Residual_EtaJES_GSC_Insitu"),
        CalibArea=calibArea,
        IsData=not flags.Input.isMC,
    )
    acc.addPublicTool(calibTool)
    acc.addEventAlgo(
        CompFactory.JetCalibrationDecoratorAlg(
            "JetCalibrationDecoratorAlg",
            JetCalibrationTool=calibTool,
            JetContainer=JETS
        )
    )

    # ── Large-R jet truth label ──
    # New factorised truth label (Origin × Decay × Containment enum,
    # Int_t) for AntiKt10UFOCSSK...Jets.  Written by
    # FtagLargeRJetTruthLabelTool (MR !87294).  Guard is inside the
    # cfg — returns empty CA on data.
    from ParticleJetTools.FtagLargeRJetTruthLabelConfig import (
        FtagLargeRJetTruthLabelCfg,
    )
    acc.merge(FtagLargeRJetTruthLabelCfg(
        flags,
        jetCollection="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
    ))

    # ── Pass-through model (constituent variables from JSON) ──
    # Reads all constituent types (tracks, electrons, muons, calo,
    # flow, towers) via GNN loaders and outputs per-constituent
    # vectors as jet decorations.  bfloat16 compression is inline
    # via "cast": {"exp":8, "man":7} in the JSON config.
    # One PassThroughModel instance per jet collection reads the data
    # (reco-only) JSON; on MC a second instance adds the truth-only
    # variables from the mc_extra JSON.
    from FlavorTagInference.FlavorTagNNConfig import PassThroughModelCfg
    ptRemap = {
        'BTagTrackToJetAssociator': 'GhostTrack',
        'FTagElectrons': 'GhostFTagSelectedElectrons',
        'FTagMuons': 'GhostFTagMuons',
    }
    for cfg in JET_COLLECTIONS.values():
        if cfg["passthrough_json"]:
            acc.merge(PassThroughModelCfg(
                flags, cfg["name"],
                jsonPath=cfg["passthrough_json"],
                variableRemapping=ptRemap,
                muons='Muons',
            ))
        if flags.Input.isMC and cfg["passthrough_json_mc_extra"]:
            acc.merge(PassThroughModelCfg(
                flags, cfg["name"],
                jsonPath=cfg["passthrough_json_mc_extra"],
                variableRemapping=ptRemap,
                muons='Muons',
                nameSuffix='MCExtra',
            ))

    # ── bfloat16 compression of constituent vector decorations ──
    # Handled inline by PassThrough model via "cast": "bf16" in JSON config.
    # The GNN framework converts float→uint16 at decoration time using
    # FPCompressionUtils::truncateToUint16(v, 8, 7).  bfloat16 has float32's
    # exponent range, so no scale factors needed — values stored in native MeV.

    # ── VectorExploderAlg: explode EnergyPerSampling vectors into scalars ──
    # Hardcoded to AntiKt4EMPFlowJets only — large-R jets do not carry these
    # vectors.  Each entry in _CALO_SAMPLING_NAMES corresponds to one index
    # in the 28-element EnergyPerSampling / EnergyPerSamplingCaloBased aux
    # vectors.  The resulting scalar decorations (e_<layer> / e_<layer>_CaloBased)
    # are kept as float32 jet decorations (no longer cast to bf16 — the
    # JetScalarCastAlg cast layer was removed due to a thread-safety bug
    # surfacing in AthenaMP runs).  VectorExploderAlg remains because TDD's
    # energy_per_sampling.json fragment (used by GN3_dev.json) still needs it.
    _eps_scalar_names = [f"e_{s}" for s in _CALO_SAMPLING_NAMES]
    acc.addEventAlgo(
        CompFactory.FlavorTagJetDecorators.VectorExploderAlg(
            "VectorExploderAlg_AntiKt4EMPFlowJets_EnergyPerSampling",
            Collection="AntiKt4EMPFlowJets",
            InputVectorName="EnergyPerSampling",
            OutputNamesMap={i: name for i, name in enumerate(_eps_scalar_names)},
        )
    )
    _epscb_scalar_names = [f"e_{s}_CaloBased" for s in _CALO_SAMPLING_NAMES]
    acc.addEventAlgo(
        CompFactory.FlavorTagJetDecorators.VectorExploderAlg(
            "VectorExploderAlg_AntiKt4EMPFlowJets_EnergyPerSamplingCaloBased",
            Collection="AntiKt4EMPFlowJets",
            InputVectorName="EnergyPerSamplingCaloBased",
            OutputNamesMap={i: name for i, name in enumerate(_epscb_scalar_names)},
        )
    )

    # ── Primary vertex decorator ──
    # Pre-compute nPrimaryVertices (int) and primaryVertexZ (float) as
    # EventInfo decorations.  TDD reads these instead of the PrimaryVertices
    # container, which can then be dropped from the DAOD output.
    acc.addEventAlgo(
        CompFactory.FlavorTagJetDecorators.PrimaryVertexDecoratorAlg(
            "PrimaryVertexDecoratorAlg",
        )
    )

    # ── Truth tau matching ──
    # Match each jet to the nearest isolated truth tau using visible
    # 4-momentum, and decorate with tau properties (isHadronicTau,
    # decayMode, classifierParticleOutCome, pt_vis, deltaPt, matched).
    # This replaces TDD's TruthTauMatcher ca_block, allowing TruthTaus
    # to be excluded from the DAOD output.
    if flags.Input.isMC:
        acc.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.TruthTauDecoratorAlg(
                f"TruthTauDecoratorAlg_{JETS}",
                JetContainer=JETS,
                TruthTauContainer="TruthTaus",
                MaxDeltaR=0.3,
            )
        )

    # ── Overlap lepton flag ──
    # For each jet, check if any truth electron/muon from W/Z/top
    # (status==1, classifierParticleOrigin in {WBoson, ZBoson, top})
    # is within DeltaR < 0.4.  Writes char ftag_hasOverlapLepton on
    # the jet.  TDD reads this flag to skip overlapping jets, replacing
    # the truth association that required TruthElectrons/TruthMuons
    # containers in the DAOD.
    # Only applied to small-R jets (large-R has no overlap-lepton cut).
    if flags.Input.isMC:
        for cfg in JET_COLLECTIONS.values():
            if cfg["overlap_lepton"]:
                acc.addEventAlgo(
                    CompFactory.FlavorTagJetDecorators.JetOverlapLeptonDecoratorAlg(
                        f"JetOverlapLeptonDecoratorAlg_{cfg['name']}",
                        JetContainer=cfg["name"],
                        TruthElectronContainer="TruthElectrons",
                        TruthMuonContainer="TruthMuons",
                    )
                )

    # ── Thinning ──
    # Only jet thinning remains.  All constituent variables (tracks,
    # electrons, muons, calo, flow, towers) are jet decorations via
    # PassThrough — no separate constituent containers need thinning.
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        GenericObjectThinningCfg,
    )

    stream = kwargs['StreamName']
    thinningTools = []

    # Thin each jet collection independently using its own selection string.
    # The thinning expression lives in the PassThrough JSON under the
    # top-level "thinning" key and uses `{jet}` as a placeholder for the
    # container name.
    for key, cfg in JET_COLLECTIONS.items():
        jet_name = cfg["name"]
        pt_cfg = _load_passthrough_json(cfg["passthrough_json"])
        thinning_tmpl = pt_cfg.get("thinning")
        if flags.Input.isMC and cfg["passthrough_json_mc_extra"]:
            extra_cfg = _load_passthrough_json(cfg["passthrough_json_mc_extra"])
            thinning_tmpl = extra_cfg.get("thinning", thinning_tmpl)
        if not thinning_tmpl:
            raise RuntimeError(
                f"PassThrough JSON {cfg['passthrough_json']} is missing a "
                f"top-level 'thinning' selection string."
            )
        jet_sel = thinning_tmpl.format(jet=jet_name)
        thinningTools.append(acc.getPrimaryAndMerge(GenericObjectThinningCfg(
            flags,
            name=f"FTAG1LITEJetThinningTool_{key}",
            StreamName=stream,
            ContainerName=jet_name,
            SelectionString=jet_sel,
        )))

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name, AugmentationTools=[], ThinningTools=thinningTools,
    ))

    return acc


# ── Slimming ─────────────────────────────────────────────────────────

def FTAG1LITECoreCfg(flags, name_tag='FTAG1LITE'):
    """Configure output content for FTAG1LITE."""
    acc = ComponentAccumulator()

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    helper = SlimmingHelper(
        name_tag + "SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    helper.SmartCollections = []

    helper.AllVariables = [
        "EventInfo",
    ]

    # ── Build ExtraVariables from each collection's PassThrough JSON ──
    # The JSON is the complete authoritative spec for what jet-level variables
    # are persisted for each collection. Three sources get appended:
    #   * `copy_vars`       — bare strings naming already-existing jet
    #                         decorations / AOD moments to persist.  The
    #                         PassThroughSaltModel does not touch this key.
    #   * `jet_variables`   — list of {input, output, cast} dicts for
    #                         PassThrough-computed scalar outputs (the C++
    #                         SaltModel reads this too).
    #   * `constituents[].variables[].output` — per-constituent vector
    #                         decorations written by the C++ SaltModel.
    # With SmartCollections = [], ExtraVariables IS the complete item list
    # for each jet collection — nothing is persisted that isn't named here.
    def _output_name(entry):
        return entry["output"]

    helper.ExtraVariables = []
    for cfg in JET_COLLECTIONS.values():
        json_paths = [cfg["passthrough_json"]]
        if flags.Input.isMC and cfg["passthrough_json_mc_extra"]:
            json_paths.append(cfg["passthrough_json_mc_extra"])
        extras = []
        for path in json_paths:
            pt_config = _load_passthrough_json(path)
            extras.extend(pt_config.get("copy_vars", []))
            # jet_variables: GNN-computed scalar outputs (C++ PassThrough reads this key)
            extras.extend(_output_name(v) for v in pt_config.get("jet_variables", []))
            for cnode in pt_config.get("constituents", []):
                extras.extend(_output_name(v) for v in cnode.get("variables", []))
        helper.ExtraVariables.append(
            '.'.join([cfg["name"]] + extras)
        )

    excludedVtxAux = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    static = [
        "xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices",
        f"xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux.{excludedVtxAux}",
        "xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices",
        f"xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux.{excludedVtxAux}",
        "xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices",
        f"xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux.{excludedVtxAux}",
    ]
    if flags.BTagging.GNNVertexFitter:
        static += [
            "xAOD::VertexContainer#GNNVertices",
            f"xAOD::VertexAuxContainer#GNNVerticesAux.{excludedVtxAux}",
            "xAOD::VertexContainer#InclusiveGNNVertices",
            f"xAOD::VertexAuxContainer#InclusiveGNNVerticesAux.{excludedVtxAux}",
        ]
    helper.StaticContent = static

    helper.IncludeTriggerNavigation = False
    helper.IncludeJetTriggerContent = False
    helper.IncludeMuonTriggerContent = False
    helper.IncludeEGammaTriggerContent = False
    helper.IncludeTauTriggerContent = False
    helper.IncludeEtMissTriggerContent = False
    helper.IncludeBJetTriggerContent = False
    helper.IncludeBPhysTriggerContent = False
    helper.IncludeMinBiasTriggerContent = False

    FTAG1LITEItemList = helper.GetItemList()

    # Remove trigger CompulsoryContent — not needed for training
    _trigger_items = {
        'xAOD::TrigDecision#*',
        'xAOD::TrigDecisionAuxInfo#*',
        'xAOD::TrigConfKeys#*',
        'xAOD::BunchConfKey#*',
    }
    FTAG1LITEItemList = [i for i in FTAG1LITEItemList
                         if i not in _trigger_items]

    # Remove containers not needed by TDD
    # Remove containers not needed — all constituent variables are
    # jet decorations via PassThrough, no separate containers required.
    _containers_to_remove = [
        '#Photons', '#TruthTaus', '#InDetForwardTrackParticles',
        '#MET_Truth', '#egammaClusters', '#GSFTrackParticles',
        '#PrimaryVertices',
        '#CHSGChargedParticleFlowObjects',
        '#CHSGNeutralParticleFlowObjects',
        '#CaloCalTopoClusters',
        '#TruthBottom', '#TruthTop',
        '#TruthBoson', '#TruthCharm', '#TruthEvents',
        '#TruthPrimaryVertices',
        '#Electrons',
        '#Muons',
        '#CombinedMuonTrackParticles',
        '#ExtrapolatedMuonTrackParticles',
        '#MuonSpectrometerTrackParticles',
        '#InDetTrackParticles',
        '#TruthElectrons', '#TruthMuons',
    ]
    FTAG1LITEItemList = [i for i in FTAG1LITEItemList
                          if not any(c in i for c in _containers_to_remove)]

    # NOTE: mcEventWeights vector is NOT stripped here because framework code
    # (CutFlowSvc or similar) calls EventInfo::mcEventWeight() at dump time,
    # which reads the vector.  The McEventWeightDecoratorAlg adds a single-float
    # mcEventWeight decoration alongside the vector; TDD reads the decoration
    # when available.  Stripping the vector requires finding and fixing all
    # framework readers first.

    # Strip hardScatterVertexLink from EventInfo.
    _eventinfo_vars_to_remove = {'hardScatterVertexLink'}
    FTAG1LITEItemList = _filter_aux_vars(
        FTAG1LITEItemList, 'EventInfo', _eventinfo_vars_to_remove)

    acc.merge(OutputStreamCfg(
        flags, "DAOD_" + name_tag,
        ItemList=FTAG1LITEItemList,
        AcceptAlgs=[name_tag + "Kernel"],
    ))

    # Custom metadata setup: propagate everything EXCEPT trigger menu.
    # Standard SetupMetaDataForStreamCfg propagates all input metadata
    # including TriggerMenuJson (~146 KB) which training doesn't need.
    from AthenaConfiguration.Enums import MetadataCategory
    from xAODMetaDataCnv.InfileMetaDataConfig import (
        MetaDataHelperLists, propagateMetaData, createCutFlowMetaData,
        createTruthMetaData, createEventStreamInfo,
    )
    from OutputStreamAthenaPool.OutputStreamConfig import addToMetaData
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg

    streamName = "DAOD_" + name_tag
    mdLists = MetaDataHelperLists()

    for cat in MetadataCategory:
        if cat == MetadataCategory.TriggerMenuMetaData:
            continue
        lists, ca = propagateMetaData(flags, streamName, cat)
        mdLists += lists
        acc.merge(ca)

    create_fns = [createCutFlowMetaData, createEventStreamInfo]
    if flags.Input.isMC:
        create_fns.append(createTruthMetaData)
    for create_fn in create_fns:
        lists, ca = create_fn(flags, streamName=streamName)
        mdLists += lists
        acc.merge(ca)

    acc.merge(addToMetaData(
        flags, streamName=streamName,
        itemOrList=mdLists.mdItems,
        AcceptAlgs=[name_tag + "Kernel"],
        HelperTools=mdLists.helperTools,
    ))
    acc.merge(MetaDataSvcCfg(
        flags, tools=mdLists.mdTools, toolNames=mdLists.mdToolNames,
    ))

    return acc


# ── Entry point ──────────────────────────────────────────────────────

def FTAG1LITECfg(flags, name_tag='FTAG1LITE'):
    """Top-level configuration for FTAG1LITE derivation."""
    acc = ComponentAccumulator()

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TriggerListsHelper(flags)

    acc.merge(FTAG1LITEKernelCfg(
        flags,
        name=name_tag + "Kernel",
        StreamName='StreamDAOD_' + name_tag,
    ))
    acc.merge(FTAG1LITECoreCfg(flags, name_tag))

    return acc
