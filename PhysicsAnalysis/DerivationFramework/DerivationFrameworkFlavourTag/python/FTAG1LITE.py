# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG1LITE.py
# Minimal derivation for producing GN3 training samples via TDD.
#
# Built from individual augmentation modules rather than the monolithic
# PhysCommonAugmentationsCfg, so we only run what TDD actually needs.
# Skips: MET, DiTau, large-R jets, triggers, LRT, pixel/SCT clusters.
#
# See AODToFTAGTraining (TDD ca_block) for the individual augmentation
# pattern this is based on.
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
JETS = "AntiKt4EMPFlowJets"


def _int_labels():
    """Truth label variable names for jet matching (from FtagBaseContent)."""
    algs = ['HadronConeExcl', 'HadronGhost']
    types = ['Extended', '']
    return [f'{a}{e}TruthLabelID' for a in algs for e in types]


def _match_vars(source):
    """Extra variables produced by jet matching augmentation."""
    labels = _int_labels()
    allvars = [f'{l}From{source}' for l in labels]
    allvars += [f'delta{v}To{source}' for v in ['R', 'Pt']]
    return allvars


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
    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (
        DFCommonTruthCharmToolCfg,
    )

    acc.merge(TruthClassificationAugmentationsCfg(flags))

    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    charmTool = acc.getPrimaryAndMerge(
        DFCommonTruthCharmToolCfg(flags, name="PhysCommonTruthCharmTool")
    )
    acc.addEventAlgo(CommonAugmentation(
        "PhysCommonTruthCharmKernel", AugmentationTools=[charmTool]
    ))

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
        acc.merge(FlavorTaggingCfg(flags, JETS))

    # ── Ftag-specific augmentations ──
    from JetTagDerivationUtils.JetMatchingConfig import JetMatchingCfg
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import (
        ParentDecoratorCfg,
        trackTruthDecorator,
    )

    acc.merge(JetMatchingCfg(
        flags, target=JETS, ints_to_copy=_int_labels(),
    ))

    # ── NearestJet matching (reco-to-reco) ──
    # Match each jet to its nearest neighbour reco jet and copy kinematic
    # variables + truth label. This replaces TDD's NearestJet JetMatcher
    # ca_block, ensuring matching is done before thinning for stability.
    acc.merge(JetMatchingCfg(
        flags, target=JETS,
        source_name="NearestJet",
        floats_to_copy=["pt", "eta", "phi"],
        ints_to_copy=["HadronGhostTruthLabelID"],
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

    acc.merge(trackTruthDecorator(flags))
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

    # ── Track covariance uncertainties ──
    # Pre-compute phiUncertainty, thetaUncertainty, qOverPUncertainty from
    # the track covariance matrix diagonal.  The full CovMatrix is then
    # stripped from the DAOD output (significant size savings) while TDD
    # reads these simple float decorations instead.
    acc.addEventAlgo(
        CompFactory.TrackingDecorAlgorithms.TrackCovarianceDecoratorAlg(
            "TrackCovarianceDecoratorAlg_InDetTrackParticles",
            TrackContainer="InDetTrackParticles",
        )
    )

    # ── Calo charged flow decorator ──
    # Pre-compute usedInChargedFlow flag on CaloCalTopoClusters before
    # jet constituent thinning removes PFlow objects. Without this, TDD
    # computes the flag at dump time on the thinned DAOD, missing charged
    # PFOs that were thinned out.
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
    # Register a public JetCalibrationTool with the same name as PHYSLITE's.
    # Both algorithms use PublicToolHandle, so Gaudi shares a single instance.
    # In co-production the CA deduplicates; standalone just has one copy.
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
        CalibSequence="JetArea_Residual_EtaJES_GSC",
        CalibArea=calibArea,
        IsData=False,
    )
    acc.addPublicTool(calibTool)
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.JetCalibrationDecoratorAlg(
            "JetCalibrationDecoratorAlg",
            JetCalibrationTool="JetCalibrationTool/JetCalibTool_AntiKt4EMPFlow",
            JetContainer=JETS,
            ptCalibratedKey=f"{JETS}.pt_calibrated",
        )
    )

    # ── Truth tau matching ──
    # Match each jet to the nearest isolated truth tau using visible
    # 4-momentum, and decorate with tau properties (isHadronicTau,
    # decayMode, classifierParticleOutCome, pt_vis, deltaPt, matched).
    # This replaces TDD's TruthTauMatcher ca_block, allowing TruthTaus
    # to be excluded from the DAOD output.
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.TruthTauDecoratorAlg(
            f"TruthTauDecoratorAlg_{JETS}",
            JetContainer=JETS,
            TruthTauContainer="TruthTaus",
            MaxDeltaR=0.3,
        )
    )

    # ── Thinning ──
    from DerivationFrameworkInDet.InDetToolsConfig import (
        EgammaTrackParticleThinningCfg,
        JetConstituentThinningCfg,
        JetGhostThinningCfg,
        JetTrackParticleThinningCfg,
        MuonTrackParticleThinningCfg,
    )
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        GenericObjectThinningCfg,
    )

    jet_sel = (
        f'{JETS}.pt_calibrated > 20*GeV'
        f' && abs({JETS}.eta) < 2.5'
    )
    stream = kwargs['StreamName']
    thinningTools = []

    thinningTools.append(acc.getPrimaryAndMerge(GenericObjectThinningCfg(
        flags,
        name="FTAG1LITEJetThinningTool",
        StreamName=stream,
        ContainerName=JETS,
        SelectionString=jet_sel,
    )))

    # Track quality thinning — mirrors TDD r22loose-track-cuts
    # (TDD still applies all cuts at dump time, so this is purely a DAOD size optimisation)
    track_quality_sel = (
        "InDetTrackParticles.pt > 500"
        " && abs(InDetTrackParticles.eta) < 2.5"
        " && abs(InDetTrackParticles.d0) < 5.0*mm"
        " && (InDetTrackParticles.numberOfPixelHits + InDetTrackParticles.numberOfPixelDeadSensors"
        " + InDetTrackParticles.numberOfSCTHits + InDetTrackParticles.numberOfSCTDeadSensors) >= 8"
        " && (InDetTrackParticles.numberOfPixelHoles + InDetTrackParticles.numberOfSCTHoles) <= 2"
        " && InDetTrackParticles.numberOfPixelHoles <= 1"
    )

    thinningTools.append(acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name="FTAG1LITEJetTPThinningTool",
        StreamName=stream,
        JetKey=JETS,
        SelectionString=jet_sel,
        InDetTrackParticlesKey="InDetTrackParticles",
        TrackSelectionString=track_quality_sel,
    )))

    thinningTools.append(acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name="FTAG1LITEMuonTPThinningTool",
        StreamName=stream,
        MuonKey="Muons",
        InDetTrackParticlesKey="InDetTrackParticles",
    )))

    thinningTools.append(acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name="FTAG1LITEElectronTPThinningTool",
        StreamName=stream,
        SGKey="Electrons",
        InDetTrackParticlesKey="InDetTrackParticles",
    )))

    thinningTools.append(acc.getPrimaryAndMerge(JetConstituentThinningCfg(
        flags,
        name="FTAG1LITEJetConstituentThinningTool",
        StreamName=stream,
        JetKey=JETS,
        SelectionString=jet_sel,
        JetConstituentName="CHSG",
        GlobalConstituentName="Global",
        OtherObjectsName="CaloCalTopoClusters",
    )))

    thinningTools.append(acc.getPrimaryAndMerge(JetGhostThinningCfg(
        flags,
        name="FTAG1LITEGhostTowerThinningTool",
        StreamName=stream,
        JetKey=JETS,
        SelectionString=jet_sel,
        GhostName="GhostTower",
        GhostContainerName="CaloCalFwdTopoTowers",
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

    helper.SmartCollections = [
        "AntiKt4EMPFlowJets",
        "Muons",
        "PrimaryVertices",
        "InDetTrackParticles",
    ]

    helper.AllVariables = [
        "EventInfo",
        "CHSGNeutralParticleFlowObjects",
        "CHSGChargedParticleFlowObjects",
        "CaloCalFwdTopoTowers",
    ]

    jet_match_vars = _match_vars(JETS)
    parent_labels = [
        *[f"nTopTo{p}Children" for p in "BW"],
        *[f"parent{p}ParentsMask"
          for p in ["Higgs", "Z", "Scalar", "Top"]],
    ]

    _jet_vars = [
        "isJvtPU", "isJvtHS",
        "EnergyPerSamplingCaloBased",
        "FracSamplingMaxCaloBased", "FracSamplingMaxIndexCaloBased",
        "EMFrac", "EMFracCaloBased",
        "HECFrac", "HECFracCaloBased",
        "PSFrac", "PSFracCaloBased",
        "CentroidR", "LambdaLeadingCluster",
        "MeanRadialDistanceSquared", "MeanLongitudinalDistanceSquared",
        "GhostTrackPt", "GhostTrackCount",
        "TrackSumPt", "TrackSumMass",
        "GhostBHadronsFinalCount", "GhostBHadronsFinalPt",
        "GhostCHadronsFinalCount", "GhostCHadronsFinalPt",
        "HadronConeExclTruthLabelPt",
        "HadronConeExclTruthLabelLxy",
        "HadronConeExclTruthLabelDR",
        "HadronGhostTruthLabelID",
        "HadronGhostExtendedTruthLabelID",
        "HadronGhostTruthLabelPdgId",
        "HadronGhostTruthLabelPt",
        "HadronGhostTruthLabelLxy",
        "HadronGhostTruthLabelDR",
        "PartonTruthLabelPt", "PartonTruthLabelDR",
        "HadronConeExclTruthLabelChildPdgId",
        "HadronConeExclTruthLabelChildPt",
        "HadronConeExclTruthLabelChildLxy",
        "HadronGhostTruthLabelChildPdgId",
        "HadronGhostTruthLabelChildPt",
        "HadronGhostTruthLabelChildLxy",
        "HadronGhostInitialTruthLabelID",
        "HadronGhostInitialExtendedTruthLabelID",
        "HadronGhostInitialTruthLabelPdgId",
        "HadronGhostInitialTruthLabelPt",
        "HadronGhostInitialTruthLabelLxy",
        "PartonExtendedTruthLabelID",
        "GhostFTagElectrons", "GhostFTagMuons",
        "GhostTower",
        "LeptonDecayLabel", "TauDecayLabel",
        "constituentLinks",
        # NearestJet matching (derivation-time, replaces TDD JetMatcher block)
        "ptFromNearestJet", "etaFromNearestJet", "phiFromNearestJet",
        "HadronGhostTruthLabelIDFromNearestJet",
        "matchedToNearestJet", "deltaRToNearestJet",
        "deltaEtaToNearestJet", "deltaPhiToNearestJet",
        "deltaPtToNearestJet", "numberOfMatchesToNearestJet",
        # TauJet matching (derivation-time, replaces TDD JetMatcher block)
        "RNNJetScoreFromTauJet", "RNNJetScoreSigTransFromTauJet",
        "GNTauScore_v0pruneFromTauJet", "GNTauScoreSigTrans_v0pruneFromTauJet",
        "GNTauScoreSigTrans_v1truncFromTauJet", "ptFinalCalibFromTauJet",
        "matchedToTauJet", "deltaRToTauJet", "deltaPtToTauJet",
        "numberOfMatchesToTauJet",
        # TruthTau matching (derivation-time, replaces TDD TruthTauMatcher block)
        "isHadronicTauFromTruthTaus", "decayModeFromTruthTaus",
        "classifierParticleOutComeFromTruthTaus",
        "deltaPtToTruthTaus", "pt_visFromTruthTaus", "matchedToTruthTaus",
    ]

    helper.ExtraVariables = [
        '.'.join([JETS] + _jet_vars),
        "TruthPrimaryVertices.t.x.y.z",
        "Muons.TruthLink.segmentDeltaPhi.segmentDeltaEta"
        ".ParamEnergyLoss.ParamEnergyLossSigmaPlus"
        ".ParamEnergyLossSigmaMinus.MeasEnergyLoss.MeasEnergyLossSigma",
        "GSFTrackParticles.d0.z0.phi.theta.qOverP.vz.chiSquared"
        ".definingParametersCovMatrixDiag"
        ".numberOfPixelHits.numberOfSCTHits.numberOfSCTDeadSensors"
        ".numberOfInnermostPixelLayerHits"
        ".numberOfNextToInnermostPixelLayerHits"
        ".eProbabilityHT.eProbabilityNN.eProbabilityComb"
        ".originalTrackParticle.truthParticleLink",
        "PrimaryVertices.time.covariance.chiSquared.numberDoF",
        "Electrons.pt.eta.phi.charge.author.OQ"
        ".trackParticleLinks.caloClusterLinks.truthParticleLink"
        ".ambiguityLink.ambiguityType"
        ".Rhad.Rhad1.Eratio.weta2.Rphi.Reta.wtots1.f1.f3"
        ".deltaEta1.deltaPhiRescaled2"
        ".ftag_et.ftag_z0AlongBeamspot.ftag_z0AlongBeamspotSignificance"
        ".ftag_ptVarCone30OverPt.ftag_deltaPOverP.ftag_energyOverP"
        ".ftagTruthOriginLabel.ftagTruthTypeLabel"
        ".ftagTruthSourceLabel.ftagTruthVertexIndex"
        ".ftagTruthBarcode.ftagTruthParentBarcode",
        # egammaClusters: needed by TDD's SoftElectronSelector which calls
        # el.caloCluster()->e() following Electrons.caloClusterLinks.
        # Must NOT be in AllVariables (causes segfault from missing CaloCell data).
        "egammaClusters.calE.calEta.calPhi.e_sampl"
        ".ETA2CALOFRAME.ETACALOFRAME.PHI2CALOFRAME.PHICALOFRAME"
        ".constituentClusterLinks",
        "GlobalChargedParticleFlowObjects"
        ".pt.eta.phi.m.e.chargedObjectLinks.otherObjectLinks.signalType",
        "GlobalNeutralParticleFlowObjects"
        ".pt.eta.phi.m.e.otherObjectLinks.signalType",
        "CaloCalTopoClusters"
        ".ENG_BAD_CELLS.ISOLATION.CENTER_MAG.CELL_SIGNIFICANCE"
        ".ENG_FRAC_MAX.LATERAL.SIGNIFICANCE.LONGITUDINAL"
        ".ENG_POS.EM_PROBABILITY.CENTER_LAMBDA.SECOND_LAMBDA"
        ".FIRST_ENG_DENS.SECOND_R.AVG_LAR_Q.MASS"
        ".rawPhi.calPhi.rawEta.calEta.rawE.calE.rawM.calM.e_sampl"
        ".usedInChargedFlow"
        ".altE.altEta.altM.altPhi"
        ".eta0.phi0"
        ".clusterSize.NCELL_SAMPLING"
        ".N_BAD_CELLS.BADLARQ_FRAC.AVG_TILE_Q"
        ".PTD.sigmaWidth.SECOND_TIME",
        "InDetTrackParticles"
        ".numberOfNextToInnermostPixelLayerHits"
        ".numberOfInnermostPixelLayerSharedHits"
        ".numberOfInnermostPixelLayerSplitHits"
        ".numberOfPixelSplitHits"
        ".numberOfInnermostPixelLayerOutliers"
        ".numberOfNextToInnermostPixelLayerOutliers"
        ".numberOfPixelSpoiltHits"
        ".expectInnermostPixelLayerHit"
        ".expectNextToInnermostPixelLayerHit"
        ".btagIp_d0.btagIp_z0SinTheta"
        ".btagIp_d0Uncertainty.btagIp_z0SinThetaUncertainty"
        ".btagIp_invalidIp"
        ".TTVA_AMVFVertices.TTVA_AMVFWeights"
        ".ftagTruthOriginLabel.ftagTruthTypeLabel.ftagTruthSourceLabel"
        ".ftagTruthVertexIndex.ftagTruthBarcode.ftagTruthParentBarcode"
        ".ftagTruthMuonOriginLabel"
        ".muon_qOverPratio.muon_momentumBalanceSignificance"
        ".muon_scatteringNeighbourSignificance.muon_quality.leptonID"
        ".eProbabilityHT.truthMatchProbability"
        # Pre-computed from CovMatrix diagonal by TrackCovarianceDecoratorAlg
        ".phiUncertainty.thetaUncertainty.qOverPUncertainty"
        # Track IP vectors computed by the standard btag chain (FlavorTaggingCfg)
        ".btagIp_trackMomentum.btagIp_trackDisplacement",
    ]
    helper.ExtraVariables.append(
        '.'.join([JETS] + jet_match_vars)
    )
    helper.ExtraVariables.append(
        '.'.join([JETS] + parent_labels)
    )
    helper.ExtraVariables += [
        # Truth jet matching (derivation-time, replaces TDD JetMatcher blocks)
        JETS + ".ptFromTruthJet.matchedToTruthJet.deltaRToTruthJet"
            + ".deltaEtaToTruthJet.deltaPhiToTruthJet.deltaPtToTruthJet"
            + ".numberOfMatchesToTruthJet"
            + ".ptFromTruthDressedWZJet.matchedToTruthDressedWZJet"
            + ".deltaRToTruthDressedWZJet.deltaEtaToTruthDressedWZJet"
            + ".deltaPhiToTruthDressedWZJet.deltaPtToTruthDressedWZJet"
            + ".numberOfMatchesToTruthDressedWZJet",
        "TruthEvents.Q.XF1.XF2.PDGID1.PDGID2.PDFID1.PDFID2.X1.X2.crossSection",
        "MET_Truth.mpx.mpy.sumet.name.source",
        "TruthElectrons.prodVtxLink.decayVtxLink.parentLinks.childLinks.m.px.py.pz.e.pdgId.ptcone30.etcone20.classifierParticleOrigin.Classification.barcode.status.classifierParticleType.classifierParticleOutCome.polarizationPhi.polarizationTheta.e_dressed.pt_dressed.eta_dressed.phi_dressed.nPhotons_dressed.uid",
        "TruthMuons.m.px.py.pz.e.pdgId.barcode.status.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.Classification.e_dressed.pt_dressed.eta_dressed.phi_dressed.nPhotons_dressed.ptcone30.etcone20.decayVtxLink.prodVtxLink.parentLinks.childLinks.polarizationPhi.polarizationTheta.uid",
        "TruthBottom.m.px.py.pz.e.pdgId.barcode.status.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.Classification.prodVtxLink.decayVtxLink.parentLinks.childLinks.polarizationPhi.polarizationTheta.uid",
        "TruthBoson.m.px.py.pz.e.pdgId.barcode.status.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.Classification.prodVtxLink.decayVtxLink.parentLinks.childLinks.polarizationPhi.polarizationTheta.uid",
    ]
    helper.AllVariables += [
        "TruthCharm",
    ]

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

    # Strip CovMatrix from InDetTrackParticles — TDD reads pre-computed
    # uncertainties (phiUncertainty, thetaUncertainty, qOverPUncertainty)
    # from TrackCovarianceDecoratorAlg instead.
    _track_vars_to_remove = {
        'definingParametersCovMatrixDiag',
        'definingParametersCovMatrixOffDiag',
    }
    FTAG1LITEItemList = _filter_aux_vars(
        FTAG1LITEItemList, 'InDetTrackParticles', _track_vars_to_remove)

    # Remove containers not needed by TDD
    _containers_to_remove = [
        '#Photons', '#TruthTaus', '#InDetForwardTrackParticles',
    ]
    FTAG1LITEItemList = [i for i in FTAG1LITEItemList
                          if not any(c in i for c in _containers_to_remove)]

    # Strip unused jet variables.  Most SmartCollection variables are read
    # by TDD's internal tools (JetCalibrationTool, JetCleaningTool,
    # NNJvtTagger, GSC) so only confirmed-unused variables are removed.
    _jet_vars_to_remove = {
        # Ghost hadron element links (TDD reads Count/Pt, not the links)
        'ConeExclBHadronsFinal', 'ConeExclCHadronsFinal',
        # Fold hash (from BTaggingStandardContent, not needed for training)
        'jetFoldHash', 'jetFoldHash_noHits',
    }
    # Tagger scores for disabled taggers.  Only GN3EPCLV01 (non-flip) runs;
    # BTaggingStandardContent still adds all tagger outputs to SmartCollections.
    _disabled_tagger_prefixes = (
        'GN2v01_', 'GN2v01SimpleFlip_',
        'GN3V00_', 'GN3V00SimpleFlip_',
        'GN3PflowMuonsV00_', 'GN3PflowMuonsV00SimpleFlip_',
        'GN3EPCLV01SimpleFlip_',
    )
    FTAG1LITEItemList = _filter_aux_vars(
        FTAG1LITEItemList, 'AntiKt4EMPFlowJets', _jet_vars_to_remove,
        prefix_filter=_disabled_tagger_prefixes)

    # Strip muon variables that are never decorated in our config
    # (CloseByCorr isolation, DFCommonGoodMuon).
    _muon_vars_to_remove = {
        'DFCommonGoodMuon',
        'neflowisol20_CloseByCorr',
        'ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr',
        'ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr',
        'topoetcone20_CloseByCorr',
    }
    FTAG1LITEItemList = _filter_aux_vars(
        FTAG1LITEItemList, 'Muons', _muon_vars_to_remove)

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

    for create_fn in (createCutFlowMetaData, createTruthMetaData,
                      createEventStreamInfo):
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
    if not flags.Input.isMC:
        raise RuntimeError(
            "FTAG1LITE is MC-only. Cannot run on data."
        )
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
