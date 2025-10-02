# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration # ====================================================================
# EGAM7.py
# This defines DAOD_EGAM7, a skimmed DAOD format for Run 3.
# Keep events passing OR of electron triggers, or inclusive
#   electron selection, to retain fake electron candidates
# Cell collection is saved but thinned (keep only cells associated with
#   egammaClusters)
# It requires the flag EGAM7 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)

from DerivationFrameworkEGamma.TriggerContent import (
    BkgElectronTriggers,
    ExtraContainersTrigger,
    ExtraContainersElectronTrigger,
    ExtraContainersTriggerDataOnly,
)


thinCells = True


def EGAM7SkimmingToolCfg(flags):
    """Configure the EGAM7 skimming tool"""
    acc = ComponentAccumulator()

    # off-line based selection
    expression = "count(Electrons.pt > 4.5*GeV) >= 1"
    print("EGAM7 offline skimming expression: ", expression)
    EGAM7_OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name="EGAM7_OfflineSkimmingTool", expression=expression
    )

    # trigger-based selection
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"
    else:
        MenuType = ""
    triggers = BkgElectronTriggers[MenuType]
    print("EGAM7 trigger skimming list (OR): ", triggers)

    EGAM7_TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
        name="EGAM7_TriggerSkimmingTool", TriggerListOR=triggers
    )

    # do the AND of trigger-based and offline-based selection
    print("EGAM7 skimming is logical AND of previous selections")
    EGAM7_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(
        name="EGAM7_SkimmingTool",
        FilterList=[EGAM7_OfflineSkimmingTool, EGAM7_TriggerSkimmingTool],
    )

    acc.addPublicTool(EGAM7_OfflineSkimmingTool)
    acc.addPublicTool(EGAM7_TriggerSkimmingTool)
    acc.addPublicTool(EGAM7_SkimmingTool, primary=True)

    return acc


def EGAM7KernelCfg(flags, name="EGAM7Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM7"""
    acc = ComponentAccumulator()

    # Schedule extra jets collections
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track
    from JetRecConfig.JetRecConfig import JetRecCfg

    jetList = [AntiKt4PV0Track]
    for jd in jetList:
        acc.merge(JetRecCfg(flags, jd))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM7 augmentations
    augmentationTools = []

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
        TrackThinningKeepAllElectronTracks = False
        TrackThinningKeepJetTracks = False
        TrackThinningKeepMuonTracks = False
        TrackThinningKeepTauTracks = False
        TrackThinningKeepPVTracks = False

        # Tracks associated with Electrons
        if TrackThinningKeepElectronTracks:
            EGAM7ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM7ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM7ElectronTPThinningTool)
            thinningTools.append(EGAM7ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM7ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM7ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM7ElectronTPThinningTool2)
            thinningTools.append(EGAM7ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM7PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM7PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM7PhotonTPThinningTool)
            thinningTools.append(EGAM7PhotonTPThinningTool)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM7JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM7JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM7JetTPThinningTool)
            thinningTools.append(EGAM7JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM7MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM7MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM7MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM7TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM7TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM7TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM7TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM7TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM7TPThinningTool)

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
        truth_cond_finalState = "(TruthParticles.isGenStable)"
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
        print("EGAM7 truth thinning expression: ", truth_expression)

        EGAM7TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(
            name="EGAM7TruthThinningTool",
            StreamName=streamName,
            ParticleSelectionString=truth_expression,
            PreserveDescendants=False,
            PreserveGeneratorDescendants=True,
            PreserveAncestors=True,
        )
        acc.addPublicTool(EGAM7TruthThinningTool)
        thinningTools.append(EGAM7TruthThinningTool)

    # Keep only calo cells associated with the egammaClusters collection
    if thinCells:
        from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg

        acc.merge(
            thinCaloCellsForDFCfg(
                flags,
                inputClusterKeys=["egammaClusters"],
                streamName="StreamDAOD_EGAM7",
                inputCellKey="AllCalo",
                outputCellKey="DFEGAMCellContainer",
            )
        )

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM7SkimmingToolCfg(flags))

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


def EGAM7Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # DODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM7TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM7KernelCfg(
            flags,
            name="EGAM7Kernel",
            StreamName="StreamDAOD_EGAM7",
            TriggerListsHelper=EGAM7TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM7SlimmingHelper = SlimmingHelper(
        "EGAM7SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM7SlimmingHelper.AllVariables = [
        "Electrons",
        "GSFTrackParticles",
        "egammaClusters",
    ]

    # for trigger studies we also add:
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"
    else:
        MenuType = ""
    EGAM7SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
    EGAM7SlimmingHelper.AllVariables += ExtraContainersElectronTrigger[MenuType]
    if not flags.Input.isMC:
        EGAM7SlimmingHelper.AllVariables += ExtraContainersTriggerDataOnly[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM7SlimmingHelper.AllVariables += [
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
    # muons, tau, MET, b-tagging could be switched off if not needed
    # and use too much space
    EGAM7SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "Muons",
        "TauJets",
        "InDetTrackParticles",
        "PrimaryVertices",
        "AntiKt4EMPFlowJets",
        "MET_Baseline_AntiKt4EMPFlow",
        "BTagging_AntiKt4EMPFlow",
    ]
    if flags.Input.isMC:
        EGAM7SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # muons
    EGAM7SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM7SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM7SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # track jets
    EGAM7SlimmingHelper.ExtraVariables += [
        "AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"
    ]

    # photons: detailed shower shape variables
    EGAM7SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM7Kernel")
    print("EGAM7 gain decorations: ", gainDecorations)
    EGAM7SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM7Kernel")
    print("EGAM7 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM7SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM7SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    # truth
    if flags.Input.isMC:
        EGAM7SlimmingHelper.ExtraVariables += [
            "MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType"
        ]

        EGAM7SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM7SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM7SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM7SlimmingHelper.IncludeEGammaTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM7TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM7SlimmingHelper)

    # Add CellContainer and cluster->cell links
    if thinCells:
        EGAM7SlimmingHelper.StaticContent = [
            "CaloCellContainer#DFEGAMCellContainer",
            "CaloClusterCellLinkContainer#egammaClusters_links",
        ]
    else:
        EGAM7SlimmingHelper.StaticContent = [
            "CaloCellContainer#AllCalo",
            "CaloClusterCellLinkContainer#egammaClusters_links",
        ]

    # PLIT variables
    if flags.Derivation.Egamma.addPLITInputs:
        from LeptonTaggers.LeptonTaggersConfig import DecorateImprovedPromptLeptonAlgsCfg, GetExtraImprovedPromptVariablesForDxAOD
        acc.merge(DecorateImprovedPromptLeptonAlgsCfg(flags))
        EGAM7SlimmingHelper.ExtraVariables += [ item for item in GetExtraImprovedPromptVariablesForDxAOD() if item.startswith("Electrons")]
    if flags.Derivation.Egamma.addPLITOutputs:
        from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg, GetExtraPLITVariablesForDxAOD
        acc.merge(DecoratePLITAlgsCfg(flags))
        EGAM7SlimmingHelper.ExtraVariables += [ item for item in GetExtraPLITVariablesForDxAOD() if item.startswith("Electrons")]

    EGAM7ItemList = EGAM7SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM7",
            ItemList=EGAM7ItemList,
            AcceptAlgs=["EGAM7Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM7",
            AcceptAlgs=["EGAM7Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
