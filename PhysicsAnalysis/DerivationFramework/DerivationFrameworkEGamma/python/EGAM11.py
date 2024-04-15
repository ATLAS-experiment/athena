# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM11.py
# This defines DAOD_EGAM11, a skimmed DAOD format for Run 3.
# Z->ee reduction for central electrons - for electron ID and calibration
# Adaptation of EGAM1 for heavy ions
# It requires the flag EGAM11 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from AthenaCommon.SystemOfUnits import MeV

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)


def EGAM11SkimmingToolCfg(flags):
    """Configure the EGAM11 skimming tool"""
    acc = ComponentAccumulator()

    expression = " || ".join(
        [
            "(count( EGAM11_DiElectronMass1 > 50.0*GeV ) >= 1)",
            "(count( EGAM11_DiElectronMass2 > 50.0*GeV ) >= 1)",
            "(count( EGAM11_DiElectronMass3 > 50.0*GeV ) >= 1)",
            "(count( EGAM11_ElectronPhotonMass > 50.0*GeV )>=1)",
        ]
    )
    print("EGAM11 skimming expression: ", expression)

    acc.setPrivateTools(
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name="EGAM11SkimmingTool", expression=expression, TrigDecisionTool=""
        )
    )

    return acc


def EGAM11ZeeMassTool1Cfg(flags):
    """Configure the EGAM11 ee invariant mass augmentation tool 1"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 1. di-electron invariant mass for events passing the Z->ee
    #    selection for the e-gamma calibration
    #
    #    1 tight e, central, pT>25 GeV
    #    1 medium e, pT>20 GeV
    #    opposite-sign
    #    mee>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHTight)", "(Electrons.pt > 24.5*GeV)"]
    )

    requirement_probe = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 19.5*GeV)"]
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM11_ZEEMassTool1",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM11_DiElectronMass1",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="Electrons",
            CheckCharge=True,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


def EGAM11ZeeMassTool2Cfg(flags):
    """Configure the EGAM11 ee invariant mass augmentation tool 2"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 2. di-electron invariant mass for events passing the Z->e selection
    #    for the e-gamma calibration
    #
    #    2 medium e, central, pT>20 GeV
    #    opposite-sign
    #    mee>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 19.5*GeV)"]
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM11_ZEEMassTool2",
            Object1Requirements=requirement,
            Object2Requirements=requirement,
            StoreGateEntryName="EGAM11_DiElectronMass2",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="Electrons",
            CheckCharge=True,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


def EGAM11ZeeMassTool3Cfg(flags):
    """Configure the EGAM11 ee invariant mass augmentation tool 3"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 3. di-electron invariant mass for events passing the Z->ee
    #    selection for the e efficiencies with tag and probe.
    #
    #    1 tight e, central, pT>25 GeV
    #    1 e, central, pT>4 GeV
    #    opposite-sign + same-sign
    #    mee>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 24.5*GeV)"]
    )

    requirement_probe = "Electrons.pt > 4*GeV"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM11_ZEEMassTool3",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM11_DiElectronMass3",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="Electrons",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


def EGAM11ZegMassToolCfg(flags):
    """Configure the EGAM11 e+photon mass augmentation tool"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 4. Z->eg selection for reco SF (central)
    #    for tag and probe
    #
    #    1 tight e, central, pT>25 GeV
    #      note: use medium instead of tight for early data upon electron
    #            group request
    #    1 gamma, pT>15 GeV, central
    #    opposite sign + same sign
    #    mey>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 24.5*GeV)"]
    )

    requirement_probe = "DFCommonPhotons_et > 14.5*GeV"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM11_ZEGMassTool",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM11_ElectronPhotonMass",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="Photons",
            Pt2BranchName="DFCommonPhotons_et",
            Eta2BranchName="DFCommonPhotons_eta",
            Phi2BranchName="DFCommonPhotons_phi",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )
    return acc


# Main algorithm config
def EGAM11KernelCfg(flags, name="EGAM11Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM11"""
    acc = ComponentAccumulator()

    # Schedule extra jets collections
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMTopo
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.JetConfigFlags import jetInternalFlags

    jetList = [AntiKt4PV0Track, AntiKt4EMTopo]
    jetInternalFlags.isRecoJob = True
    for jd in jetList:
        acc.merge(JetRecCfg(flags, jd))

    # Common augmentations
    # cannot use PhysCommon sequence because
    # - no triggers
    # - no TauJets
    # so we have to use a modified version here
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg

    TrackingFlags = flags.Tracking
    acc.merge(
        InDetCommonCfg(
            flags,
            DoVertexFinding=TrackingFlags.doVertexFinding,
            AddPseudoTracks=TrackingFlags.doPseudoTracking,
            DecoLRTTTVA=False,
            DoR3LargeD0=flags.Tracking.doLargeD0,
            StoreSeparateLargeD0Container=TrackingFlags.storeSeparateLargeD0Container,
            MergeLRT=False,
        )
    )
    acc.merge(MuonsCommonCfg(flags))
    acc.merge(EGammaCommonCfg(flags))

    # jet cleaning
    # standard way in PhysCommon is
    # - calculate tau ID (needed for default jet OR)
    # - decorate jets with overlap removal
    # - do event cleaning
    # but taus are missing in HI derivations so need to do differently

    # Decorate if jet passed JVT criteria
    from JetJvtEfficiency.JetJvtEfficiencyToolConfig import getJvtEffToolCfg

    algName = "DFJet_EventCleaning_passJvtAlg"
    passJvtTool = acc.popToolsAndMerge(getJvtEffToolCfg(flags, "AntiKt4EMTopo"))
    passJvtTool.PassJVTKey = "AntiKt4EMTopoJets.DFCommonJets_passJvt"
    acc.addEventAlgo(
        CompFactory.JetDecorationAlg(
            algName, JetContainer="AntiKt4EMTopoJets", Decorators=[passJvtTool]
        )
    )

    # Decorate if jet passes OR and save decoration DFCommonJets_passOR
    # Use modified OR that does not check overlaps with tauls
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg

    outputLabel = "DFCommonJets_passOR"
    bJetLabel = ""  # default
    tauLabel = ""  # workaround for missing taus
    tauKey = ""  # workaround for missing taus
    orTool = acc.popToolsAndMerge(
        OverlapRemovalToolCfg(
            flags, outputLabel=outputLabel, bJetLabel=bJetLabel, doTaus=False
        )
    )
    algOR = CompFactory.OverlapRemovalGenUseAlg(
        "OverlapRemovalGenUseAlg",
        OverlapLabel=outputLabel,
        OverlapRemovalTool=orTool,
        TauKey=tauKey,
        TauLabel=tauLabel,
        BJetLabel=bJetLabel,
    )
    acc.addEventAlgo(algOR)

    # Do the cleaning
    from JetSelectorTools.JetSelectorToolsConfig import (
        EventCleaningToolCfg,
        JetCleaningToolCfg,
    )

    workingPoints = ["Loose"]
    prefix = "DFCommonJets_"

    for wp in workingPoints:
        cleaningLevel = wp + "Bad"
        # LLP WPs have a slightly different name format
        if "LLP" in wp:
            cleaningLevel = wp.replace("LLP", "BadLLP")

        jetCleaningTool = acc.popToolsAndMerge(
            JetCleaningToolCfg(
                flags,
                "JetCleaningTool_" + cleaningLevel,
                "AntiKt4EMTopoJets",
                cleaningLevel,
                False,
            )
        )
        acc.addPublicTool(jetCleaningTool)

        ecTool = acc.popToolsAndMerge(
            EventCleaningToolCfg(flags, "EventCleaningTool_" + wp, cleaningLevel)
        )
        ecTool.JetCleanPrefix = prefix
        ecTool.JetContainer = "AntiKt4EMTopoJets"
        ecTool.JetCleaningTool = jetCleaningTool
        acc.addPublicTool(ecTool)

        # Alg to calculate event-level and jet-level cleaning variables
        # Only store event-level flags for Loose* WPs
        eventCleanAlg = CompFactory.EventCleaningTestAlg(
            "EventCleaningTestAlg_" + wp,
            EventCleaningTool=ecTool,
            JetCollectionName="AntiKt4EMTopoJets",
            EventCleanPrefix=prefix,
            CleaningLevel=cleaningLevel,
            doEvent=("Loose" in wp),
        )
        acc.addEventAlgo(eventCleanAlg)

    # EGAM11 augmentations
    augmentationTools = []

    # ====================================================================
    # ee and egamma invariant masses
    # ====================================================================
    EGAM11ZeeMassTool1 = acc.popToolsAndMerge(EGAM11ZeeMassTool1Cfg(flags))
    acc.addPublicTool(EGAM11ZeeMassTool1)
    augmentationTools.append(EGAM11ZeeMassTool1)

    EGAM11ZeeMassTool2 = acc.popToolsAndMerge(EGAM11ZeeMassTool2Cfg(flags))
    acc.addPublicTool(EGAM11ZeeMassTool2)
    augmentationTools.append(EGAM11ZeeMassTool2)

    EGAM11ZeeMassTool3 = acc.popToolsAndMerge(EGAM11ZeeMassTool3Cfg(flags))
    acc.addPublicTool(EGAM11ZeeMassTool3)
    augmentationTools.append(EGAM11ZeeMassTool3)

    EGAM11ZegMassTool = acc.popToolsAndMerge(EGAM11ZegMassToolCfg(flags))
    acc.addPublicTool(EGAM11ZegMassTool)
    augmentationTools.append(EGAM11ZegMassTool)

    # thinning tools
    thinningTools = []
    streamName = kwargs["StreamName"]

    # Track thinning
    if flags.Derivation.Egamma.doTrackThinning:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            TrackParticleThinningCfg,
            MuonTrackParticleThinningCfg,
            TauTrackParticleThinningCfg,
        )

        TrackThinningKeepElectronTracks = True
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepAllElectronTracks = True
        TrackThinningKeepJetTracks = False
        TrackThinningKeepMuonTracks = False
        TrackThinningKeepTauTracks = False
        TrackThinningKeepPVTracks = True

        # Tracks associated with Electrons
        if TrackThinningKeepElectronTracks:
            EGAM11ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM11ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM11ElectronTPThinningTool)
            thinningTools.append(EGAM11ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM11ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM11ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM11ElectronTPThinningTool2)
            thinningTools.append(EGAM11ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM11PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM11PhotonTPThinningTool",
                    StreamName=streamName,
                    SGKey="Photons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    GSFConversionVerticesKey="GSFConversionVertices",
                    SelectionString="Photons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM11PhotonTPThinningTool)
            thinningTools.append(EGAM11PhotonTPThinningTool)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM11JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM11JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMTopoJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM11JetTPThinningTool)
            thinningTools.append(EGAM11JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM11MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM11MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM11MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM11TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM11TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM11TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM11TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM11TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM11TPThinningTool)

    # keep topoclusters around electrons
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloClusterThinningCfg,
    )

    EGAM11CCTCThinningTool = acc.getPrimaryAndMerge(
        CaloClusterThinningCfg(
            flags,
            name="EGAM11CCTCThinningTool",
            StreamName=streamName,
            SGKey="Electrons",
            SelectionString="Electrons.pt>4*GeV",
            TopoClCollectionSGKey="CaloCalTopoClusters",
            ConeSize=0.5,
        )
    )
    thinningTools.append(EGAM11CCTCThinningTool)

    # truth thinning
    if flags.Input.isMC:
        # W, Z and Higgs
        truth_cond_WZH = " && ".join(
            ["(abs(TruthParticles.pdgId) >= 23)", "(abs(TruthParticles.pdgId) <= 25)"]
        )
        # Leptons
        truth_cond_lep = " && ".join(
            ["(abs(TruthParticles.pdgId) >= 11)", "(abs(TruthParticles.pdgId) <= 16)"]
        )
        # Top quark
        truth_cond_top = "(abs(TruthParticles.pdgId) ==  6)"
        # Photon
        truth_cond_gam = " && ".join(
            ["(abs(TruthParticles.pdgId) == 22)", "(TruthParticles.pt > 1*GeV)"]
        )
        # stable particles
        truth_cond_finalState = " && ".join(
            ["(TruthParticles.status == 1)", "(TruthParticles.barcode < 200000)"]
        )
        truth_expression = (
            "( "
            + truth_cond_WZH
            + " ) || "
            + "( "
            + truth_cond_lep
            + " ) || "
            + "( "
            + truth_cond_top
            + " ) || "
            + "( "
            + truth_cond_gam
            + " ) || "
            + "( "
            + truth_cond_finalState
            + " )"
        )
        print("EGAM11 truth thinning expression: ", truth_expression)

        EGAM11TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(
            name="EGAM11TruthThinningTool",
            StreamName=streamName,
            ParticleSelectionString=truth_expression,
            PreserveDescendants=False,
            PreserveGeneratorDescendants=True,
            PreserveAncestors=True,
        )
        acc.addPublicTool(EGAM11TruthThinningTool)
        thinningTools.append(EGAM11TruthThinningTool)

    # skimming
    skimmingTool = acc.popToolsAndMerge(EGAM11SkimmingToolCfg(flags))
    acc.addPublicTool(skimmingTool)

    # setup the kernel
    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            name,
            SkimmingTools=[skimmingTool],
            AugmentationTools=augmentationTools,
            ThinningTools=thinningTools,
        )
    )

    return acc


def EGAM11Cfg(flags):
    acc = ComponentAccumulator()

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM11KernelCfg(
            flags,
            name="EGAM11Kernel",
            StreamName="StreamDAOD_EGAM11",
            TriggerListsHelper=None,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM11SlimmingHelper = SlimmingHelper(
        "EGAM11SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM11SlimmingHelper.AllVariables = [
        "Electrons",
        "GSFTrackParticles",
        "egammaClusters",
        "CaloCalTopoClusters",
    ]

    # on MC we also add:
    if flags.Input.isMC:
        EGAM11SlimmingHelper.AllVariables += [
            "TruthEvents",
            "TruthParticles",
            "TruthVertices",
            "egammaTruthParticles",
        ]

    # -------------------------------------------
    # containers that we slim
    # -------------------------------------------

    # first add variables from smart-slimming
    # adding only also those for which we add all variables since
    # the XXXCPContent.py files also bring in some extra variables
    # for other collections
    EGAM11SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "Muons",
        "TauJets",
        "PrimaryVertices",
        "InDetTrackParticles",
        "AntiKt4EMTopoJets",
    ]
    if flags.Input.isMC:
        EGAM11SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # muons
    EGAM11SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM11SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM11SlimmingHelper.ExtraVariables += ["PrimaryVertices.sumPt2"]

    # track jets
    EGAM11SlimmingHelper.ExtraVariables += [
        "AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"
    ]

    # energy density
    EGAM11SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
    ]

    # photons: detailed shower shape variables
    EGAM11SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # truth
    if flags.Input.isMC:
        EGAM11SlimmingHelper.ExtraVariables += [
            "MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType"
        ]

        EGAM11SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM11SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM11SlimmingHelper.AllVariables += ["EventInfo"]

    # Add HIEventShape and CaloSums variables for heavy ions
    EGAM11SlimmingHelper.AllVariables += ["HIEventShape"]
    EGAM11SlimmingHelper.AllVariables += ["CaloSums"]

    # Add full CellContainer
    EGAM11SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
    ]

    EGAM11ItemList = EGAM11SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM11",
            ItemList=EGAM11ItemList,
            AcceptAlgs=["EGAM11Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM11",
            AcceptAlgs=["EGAM11Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
