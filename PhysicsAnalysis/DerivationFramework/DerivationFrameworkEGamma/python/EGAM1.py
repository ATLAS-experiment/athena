# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM1.py
# This defines DAOD_EGAM1, a skimmed DAOD format for Run 3.
# Z->ee reduction for central electrons - for electron ID and calibration
# It requires the flag EGAM1 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from AthenaCommon.SystemOfUnits import MeV

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)

from DerivationFrameworkEGamma.TriggerContent import (
    ExtraContainersTrigger,
    ExtraContainersElectronTrigger,
)


def EGAM1SkimmingToolCfg(flags):
    """Configure the EGAM1 skimming tool"""
    acc = ComponentAccumulator()

    expression = " || ".join(
        [
            "(count( EGAM1_DiElectronMass1 > 50.0*GeV ) >= 1)",
            "(count( EGAM1_DiElectronMass2 > 50.0*GeV ) >= 1)",
            "(count( EGAM1_DiElectronMass3 > 50.0*GeV ) >= 1)",
            "(count( EGAM1_ElectronPhotonMass > 50.0*GeV )>=1)",
        ]
    )
    print("EGAM1 skimming expression: ", expression)

    acc.setPrivateTools(
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name="EGAM1SkimmingTool", expression=expression
        )
    )

    return acc


def EGAM1ZeeMassTool1Cfg(flags):
    """Configure the EGAM1 ee invariant mass augmentation tool 1"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 1. di-electron invariant mass for events passing the Z->ee
    #    selection for the e-gamma calibration, based on single e trigger
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
            name="EGAM1_ZEEMassTool1",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM1_DiElectronMass1",
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


def EGAM1ZeeMassTool2Cfg(flags):
    """Configure the EGAM1 ee invariant mass augmentation tool 2"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 2. di-electron invariant mass for events passing the Z->e selection
    #    for the e-gamma calibration, based on di-electron triggers
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
            name="EGAM1_ZEEMassTool2",
            Object1Requirements=requirement,
            Object2Requirements=requirement,
            StoreGateEntryName="EGAM1_DiElectronMass2",
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


def EGAM1ZeeMassTool3Cfg(flags):
    """Configure the EGAM1 ee invariant mass augmentation tool 3"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 3. di-electron invariant mass for events passing the Z->ee
    #    selection for the e efficiencies with tag and probe.
    #    Based on single e trigger, for reco (central) and ID SF(central)
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
            name="EGAM1_ZEEMassTool3",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM1_DiElectronMass3",
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


def EGAM1ZegMassToolCfg(flags):
    """Configure the EGAM1 e+photon mass augmentation tool"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 4. Z->eg selection based on single e trigger, for reco SF (central)
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
            name="EGAM1_ZEGMassTool",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM1_ElectronPhotonMass",
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
def EGAM1KernelCfg(flags, name="EGAM1Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM1"""
    acc = ComponentAccumulator()

    # Schedule extra jets collections
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track
    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.JetConfigFlags import jetInternalFlags

    jetList = [AntiKt4PV0Track]
    jetInternalFlags.isRecoJob = True
    for jd in jetList:
        acc.merge(JetRecCfg(flags, jd))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM1 augmentations
    augmentationTools = []

    # ====================================================================
    # ee and egamma invariant masses
    # ====================================================================
    EGAM1ZeeMassTool1 = acc.popToolsAndMerge(EGAM1ZeeMassTool1Cfg(flags))
    acc.addPublicTool(EGAM1ZeeMassTool1)
    augmentationTools.append(EGAM1ZeeMassTool1)

    EGAM1ZeeMassTool2 = acc.popToolsAndMerge(EGAM1ZeeMassTool2Cfg(flags))
    acc.addPublicTool(EGAM1ZeeMassTool2)
    augmentationTools.append(EGAM1ZeeMassTool2)

    EGAM1ZeeMassTool3 = acc.popToolsAndMerge(EGAM1ZeeMassTool3Cfg(flags))
    acc.addPublicTool(EGAM1ZeeMassTool3)
    augmentationTools.append(EGAM1ZeeMassTool3)

    EGAM1ZegMassTool = acc.popToolsAndMerge(EGAM1ZegMassToolCfg(flags))
    acc.addPublicTool(EGAM1ZegMassTool)
    augmentationTools.append(EGAM1ZegMassTool)

    # ====================================================================
    # Common calo decoration tools
    # ====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloDecoratorKernelCfg)
    acc.merge(CaloDecoratorKernelCfg(flags))

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
            EGAM1ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM1ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM1ElectronTPThinningTool)
            thinningTools.append(EGAM1ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM1ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM1ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM1ElectronTPThinningTool2)
            thinningTools.append(EGAM1ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM1PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM1PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM1PhotonTPThinningTool)
            thinningTools.append(EGAM1PhotonTPThinningTool)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM1JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM1JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM1JetTPThinningTool)
            thinningTools.append(EGAM1JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM1MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM1MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM1MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM1TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM1TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM1TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM1TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM1TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM1TPThinningTool)

    # keep topoclusters around electrons
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloClusterThinningCfg,
    )

    EGAM1CCTCThinningTool = acc.getPrimaryAndMerge(
        CaloClusterThinningCfg(
            flags,
            name="EGAM1CCTCThinningTool",
            StreamName=streamName,
            SGKey="Electrons",
            SelectionString="Electrons.pt>4*GeV",
            TopoClCollectionSGKey="CaloCalTopoClusters",
            ConeSize=0.5,
        )
    )
    thinningTools.append(EGAM1CCTCThinningTool)

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
        print("EGAM1 truth thinning expression: ", truth_expression)

        EGAM1TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(
            name="EGAM1TruthThinningTool",
            StreamName=streamName,
            ParticleSelectionString=truth_expression,
            PreserveDescendants=False,
            PreserveGeneratorDescendants=True,
            PreserveAncestors=True,
        )
        acc.addPublicTool(EGAM1TruthThinningTool)
        thinningTools.append(EGAM1TruthThinningTool)

    # skimming
    skimmingTool = acc.popToolsAndMerge(EGAM1SkimmingToolCfg(flags))
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


def EGAM1Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM1TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM1KernelCfg(
            flags,
            name="EGAM1Kernel",
            StreamName="StreamDAOD_EGAM1",
            TriggerListsHelper=EGAM1TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM1SlimmingHelper = SlimmingHelper(
        "EGAM1SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM1SlimmingHelper.AllVariables = [
        "Electrons",
        "GSFTrackParticles",
        "egammaClusters",
        "CaloCalTopoClusters",
    ]

    # for trigger studies we also add trigger containers
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"
    else:
        MenuType = ""
    EGAM1SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
    EGAM1SlimmingHelper.AllVariables += ExtraContainersElectronTrigger[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM1SlimmingHelper.AllVariables += [
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
    EGAM1SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "Muons",
        "TauJets",
        "PrimaryVertices",
        "InDetTrackParticles",
        "AntiKt4EMPFlowJets",
        "BTagging_AntiKt4EMPFlow",
        "MET_Baseline_AntiKt4EMPFlow",
    ]
    if flags.Input.isMC:
        EGAM1SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # muons
    EGAM1SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM1SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM1SlimmingHelper.ExtraVariables += ["PrimaryVertices.sumPt2"]

    # track jets
    EGAM1SlimmingHelper.ExtraVariables += [
        "AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"
    ]

    # energy density
    EGAM1SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    from DerivationFrameworkEGamma import EGammaIsoConfig

    (
        pflowIsoVar,
        densityList,
        densityDict,
        acc1,
    ) = EGammaIsoConfig.makeEGammaCommonIsoCfg(flags)
    acc.merge(acc1)
    EGAM1SlimmingHelper.AppendToDictionary.update(densityDict)
    EGAM1SlimmingHelper.ExtraVariables += densityList

    # photons: detailed shower shape variables
    EGAM1SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM1Kernel")
    print("EGAM1 gain decorations: ", gainDecorations)
    EGAM1SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM1Kernel")
    print("EGAM1 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM1SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # truth
    if flags.Input.isMC:
        EGAM1SlimmingHelper.ExtraVariables += [
            "MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType"
        ]

        EGAM1SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM1SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM1SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM1SlimmingHelper.IncludeEGammaTriggerContent = True

    # Add trigger matching info
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            AddRun2TriggerMatchingToSlimmingHelper,
        )

        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=EGAM1SlimmingHelper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=EGAM1TriggerListsHelper.Run2TriggerNamesNoTau,
        )
    # Run 3
    if flags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import (
            AddRun3TrigNavSlimmingCollectionsToSlimmingHelper,
        )

        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM1SlimmingHelper)
        # Run 2 is added here temporarily to allow testing/comparison/debugging
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            AddRun2TriggerMatchingToSlimmingHelper,
        )

        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=EGAM1SlimmingHelper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=EGAM1TriggerListsHelper.Run3TriggerNamesNoTau,
        )

    # Add full CellContainer
    EGAM1SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
    ]

    EGAM1ItemList = EGAM1SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM1",
            ItemList=EGAM1ItemList,
            AcceptAlgs=["EGAM1Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM1",
            AcceptAlgs=["EGAM1Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
