# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM8.py
# This defines DAOD_EGAM8, a skimmed DAOD format for Run 3.
# Z->ee reduction for forward e tag-and-probe (one central e, one fwd e)
# Z->emu reduction for bkg studies (one mu, one fwd e)
# It requires the flag EGAM8 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from AthenaCommon.SystemOfUnits import MeV

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)


def EGAM8SkimmingToolCfg(flags):
    """Configure the EGAM8 skimming tool"""

    expression = " || ".join(
        [
            "(count(EGAM8_DiElectronMass >50.0*GeV) >=1)",
            "(count(EGAM8_MuonElectronMass >50.0*GeV) >=1)",
        ]
    )
    print("EGAM8 skimming expression: ", expression)

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    return xAODStringSkimmingToolCfg(flags, name = "EGAM8SkimmingTool",
                                     expression = expression)


def EGAM8ZeeMassToolCfg(flags):
    """Configure the EGAM8 ee invariant mass augmentation tool"""
    acc = ComponentAccumulator()

    # ====================================================================
    # di-electron invariant mass for events passing the Z->ee
    # selection based on single e trigger, for reco (central) and ID
    # SF(central+fwd)
    #
    # 1 medium e, central, pT>25 GeV
    # 1 forward e, pT>20 GeV
    # OS+SS
    # mee>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 24.5*GeV)"]
    )
    requirement_probe = "ForwardElectrons.pt > 19.5*GeV"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM8_ZEEMassTool",
            Object1Requirements=requirement_tag,
            Object2Requirements=requirement_probe,
            StoreGateEntryName="EGAM8_DiElectronMass",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="ForwardElectrons",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


def EGAM8ZmueMassToolCfg(flags):
    """Configure the EGAM8 mue invariant mass augmentation tool"""
    acc = ComponentAccumulator()

    # ====================================================================
    # mue invariant mass for events passing the Z->mue selection based
    # on single muon trigger, for ID SF(central+fwd) background studies
    #
    # 1 medium muon, central, pT>25 GeV
    # 1 forward e, pT>20 GeV
    # OS+SS
    # m(mue)>50 GeV (cut applied in skimming step later)
    # ====================================================================

    requirement_muon = " && ".join(
        [
            "Muons.pt>24.5*GeV",
            "abs(Muons.eta)<2.7",
            "Muons.DFCommonMuonPassPreselection",
        ]
    )
    requirement_electron = "ForwardElectrons.pt > 19.5*GeV"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM8_ZMuEMassTool",
            Object1Requirements=requirement_muon,
            Object2Requirements=requirement_electron,
            StoreGateEntryName="EGAM8_MuonElectronMass",
            Mass1Hypothesis=105 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Muons",
            Container2Name="ForwardElectrons",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


# Main algorithm config
def EGAM8KernelCfg(flags, name="EGAM8Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM8"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM8 augmentations
    augmentationTools = []

    # ====================================================================
    # ee and mue invariant masses
    # ====================================================================
    EGAM8ZeeMassTool = acc.popToolsAndMerge(EGAM8ZeeMassToolCfg(flags))
    acc.addPublicTool(EGAM8ZeeMassTool)
    augmentationTools.append(EGAM8ZeeMassTool)

    EGAM8ZmueMassTool = acc.popToolsAndMerge(EGAM8ZmueMassToolCfg(flags))
    acc.addPublicTool(EGAM8ZmueMassTool)
    augmentationTools.append(EGAM8ZmueMassTool)

    # ====================================================================
    # Common calo decoration tools
    # ====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloDecoratorKernelCfg)
    acc.merge(CaloDecoratorKernelCfg(flags))

    from DerivationFrameworkEGamma.EGammaToolsConfig import (
        EGammaCookieCutClusterToolCfg)
    cookieCutTool = acc.popToolsAndMerge(
        EGammaCookieCutClusterToolCfg(flags,
                                      name,
                                      StoreInputMoments=True,
                                      StoreCookedMoments=True))
    acc.addPublicTool(cookieCutTool)
    augmentationTools.append(cookieCutTool)

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
        TrackThinningKeepAllElectronTracks = False
        TrackThinningKeepJetTracks = False
        TrackThinningKeepMuonTracks = True
        TrackThinningKeepTauTracks = False
        TrackThinningKeepPVTracks = False

        # Tracks associated with Electrons
        if TrackThinningKeepElectronTracks:
            EGAM8ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM8ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM8ElectronTPThinningTool)
            thinningTools.append(EGAM8ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM8ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM8ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM8ElectronTPThinningTool2)
            thinningTools.append(EGAM8ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM8PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM8PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM8PhotonTPThinningTool)
            thinningTools.append(EGAM8PhotonTPThinningTool)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM8JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM8JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM8JetTPThinningTool)
            thinningTools.append(EGAM8JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM8MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM8MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM8MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM8TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM8TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM8TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3.0*mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM8TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM8TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM8TPThinningTool)

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
            ["(TruthParticles.isPhoton)", "(TruthParticles.pt > 1*GeV)"]
        )
        # stable particles
        truth_cond_finalState = " && ".join(
            ["(TruthParticles.isGenStable)", "(!TruthParticles.isSimulationParticle)"]
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
        print("EGAM8 truth thinning expression: ", truth_expression)

        EGAM8TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(
            name="EGAM8TruthThinningTool",
            StreamName=streamName,
            ParticleSelectionString=truth_expression,
            PreserveDescendants=False,
            PreserveGeneratorDescendants=True,
            PreserveAncestors=True,
        )
        acc.addPublicTool(EGAM8TruthThinningTool)
        thinningTools.append(EGAM8TruthThinningTool)

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM8SkimmingToolCfg(flags))

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


def EGAM8Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM8TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM8KernelCfg(
            flags,
            name="EGAM8Kernel",
            StreamName="StreamDAOD_EGAM8",
            TriggerListsHelper=EGAM8TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM8SlimmingHelper = SlimmingHelper(
        "EGAM8SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM8SlimmingHelper.AllVariables = [
        "Electrons",
        "ForwardElectrons",
        "GSFTrackParticles",
        "egammaClusters",
        "ForwardElectronClusters",
    ]

    # for trigger studies we also add:
    # EGAM8SlimmingHelper.AllVariables += [ ]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM8SlimmingHelper.AllVariables += [
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
    EGAM8SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "Muons",
        "TauJets",
        "PrimaryVertices",
        "InDetTrackParticles",
        "AntiKt4EMPFlowJets",

        "MET_Baseline_AntiKt4EMPFlow",
    ]
    if flags.Input.isMC:
        EGAM8SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # muons
    EGAM8SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM8SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM8SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # track jets
    EGAM8SlimmingHelper.ExtraVariables += [
        "AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"
    ]

    # energy density
    EGAM8SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    # photons: detailed shower shape variables
    EGAM8SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM8Kernel")
    print("EGAM8 gain decorations: ", gainDecorations)
    EGAM8SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM8Kernel")
    print("EGAM8 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM8SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # truth
    if flags.Input.isMC:
        EGAM8SlimmingHelper.ExtraVariables += [
            "MuonTruthParticles.e.px.py.pz.status.pdgId.truthClassification.truthOrigin.truthType"
        ]

        EGAM8SlimmingHelper.ExtraVariables += [
            "Photons.truthClassification.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM8SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM8SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM8SlimmingHelper.IncludeEGammaTriggerContent = True
    EGAM8SlimmingHelper.IncludeMuonTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM8SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM8TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM8SlimmingHelper)

    # Add full CellContainer
    EGAM8SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
        "CaloClusterCellLinkContainer#ForwardElectronClusters_links",
        "xAOD::CaloClusterContainer#ForwardElectronCookieCutClusters",
        "xAOD::CaloClusterAuxContainer#ForwardElectronCookieCutClustersAux.",
    ]

    EGAM8ItemList = EGAM8SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM8",
            ItemList=EGAM8ItemList,
            AcceptAlgs=["EGAM8Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM8",
            AcceptAlgs=["EGAM8Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
