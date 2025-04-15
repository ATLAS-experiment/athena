# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM9.py
# This defines DAOD_EGAM9, a skimmed DAOD format for Run 3.
# keep events passing or of photon triggers used for boostrap efficiency
# measurement of photon triggers
# It requires the flag EGAM9 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)
from DerivationFrameworkEGamma.ElectronsCPDetailedContent import (
    ElectronsCPDetailedContent,
    GSFTracksCPDetailedContent,
)

from DerivationFrameworkEGamma.TriggerContent import (
    BootstrapPhotonTriggers,
    noalgTriggers,
    ExtraContainersTrigger,
    ExtraContainersPhotonTrigger,
    ExtraContainersTriggerDataOnly,
)


# additional settings for this derivation
thinCells = False
keepCells = False
applyTriggerSelection = True
saveJets = False


def EGAM9SkimmingToolCfg(flags):
    """Configure the EGAM9 skimming tool"""
    acc = ComponentAccumulator()

    # criteria for off-line based selection
    photon_selection = (
        "(count(Photons.pt > 9.5*GeV && " + "Photons.DFCommonPhotonsIsEMLoose) > 0)"
    )
    electron_selection = (
        "(count(Electrons.pt > 9.5*GeV && "
        + "Electrons.DFCommonElectronsLHMedium) > 0)"
    )
    expression = photon_selection + " || " + electron_selection

    if applyTriggerSelection:
        # trigger-based selection
        MenuType = None
        if flags.Trigger.EDMVersion == 2:
            MenuType = "Run2"
        elif flags.Trigger.EDMVersion == 3:
            MenuType = "Run3"
        else:
            MenuType = ""
        triggers = BootstrapPhotonTriggers[MenuType]
        triggers += noalgTriggers[MenuType]
        print("EGAM9 trigger skimming list (OR): ", triggers)
        EGAM9_TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name="EGAM9_TriggerSkimmingTool", TriggerListOR=triggers
        )

        # off-line based selection
        print("EGAM9 offline skimming expression: ", expression)
        EGAM9_OfflineSkimmingTool = (
            CompFactory.DerivationFramework.xAODStringSkimmingTool(
                name="EGAM9_OfflineSkimmingTool", expression=expression
            )
        )

        # do the AND of trigger-based and offline-based selection
        print("EGAM9 skimming is logical AND of previous selections")
        EGAM9_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(
            name="EGAM9_SkimmingTool",
            FilterList=[EGAM9_OfflineSkimmingTool, EGAM9_TriggerSkimmingTool],
        )
        acc.addPublicTool(EGAM9_OfflineSkimmingTool)
        acc.addPublicTool(EGAM9_TriggerSkimmingTool)
        acc.addPublicTool(EGAM9_SkimmingTool, primary=True)
    else:
        # off-line based selection
        print("EGAM9 skimming expression: ", expression)
        EGAM9_SkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name="EGAM9_SkimmingTool", expression=expression
        )
        acc.addPublicTool(EGAM9_SkimmingTool, primary=True)

    return acc


def EGAM9KernelCfg(flags, name="EGAM9Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM9"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM9 augmentations
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
        TrackThinningKeepElectronTracks = False
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepPVTracks = False

        # Tracks associated with Electrons
        if TrackThinningKeepElectronTracks:
            EGAM9ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM9ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM9ElectronTPThinningTool)
            thinningTools.append(EGAM9ElectronTPThinningTool)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM9PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM9PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM9PhotonTPThinningTool)
            thinningTools.append(EGAM9PhotonTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt > 10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                TrackParticleThinningCfg,
            )

            EGAM9TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM9TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM9TPThinningTool)

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
        print("EGAM9 truth thinning expression: ", truth_expression)

        EGAM9TruthThinningTool = CompFactory.DerivationFramework.GenericTruthThinning(
            name="EGAM9TruthThinningTool",
            StreamName=streamName,
            ParticleSelectionString=truth_expression,
            PreserveDescendants=False,
            PreserveGeneratorDescendants=True,
            PreserveAncestors=True,
        )
        acc.addPublicTool(EGAM9TruthThinningTool)
        thinningTools.append(EGAM9TruthThinningTool)

    # Keep only calo cells associated with the egammaClusters collection
    if thinCells:
        from DerivationFrameworkCalo.CaloCellDFGetterConfig import thinCaloCellsForDFCfg

        acc.merge(
            thinCaloCellsForDFCfg(
                flags,
                inputClusterKeys=["egammaClusters"],
                streamName="StreamDAOD_EGAM9",
                inputCellKey="AllCalo",
                outputCellKey="DFEGAMCellContainer",
            )
        )

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM9SkimmingToolCfg(flags))

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


def EGAM9Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # TODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM9TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM9KernelCfg(
            flags,
            name="EGAM9Kernel",
            StreamName="StreamDAOD_EGAM9",
            TriggerListsHelper=EGAM9TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM9SlimmingHelper = SlimmingHelper(
        "EGAM9SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM9SlimmingHelper.AllVariables = [
        "Electrons",
        "Photons",
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
    EGAM9SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
    EGAM9SlimmingHelper.AllVariables += ExtraContainersPhotonTrigger[MenuType]
    if not flags.Input.isMC:
        EGAM9SlimmingHelper.AllVariables += ExtraContainersTriggerDataOnly[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM9SlimmingHelper.AllVariables += [
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
    EGAM9SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "InDetTrackParticles",
        "PrimaryVertices",
    ]
    if saveJets:
        EGAM9SlimmingHelper.SmartCollections += ["AntiKt4EMPFlowJets"]
        if flags.Input.isMC:
            EGAM9SlimmingHelper.SmartCollections += [
                "AntiKt4TruthJets",
                "AntiKt4TruthDressedWZJets",
            ]

    # then add extra variables:

    # photons
    EGAM9SlimmingHelper.ExtraVariables += ["Photons.DFCommonLoosePrime5"]

    # conversion vertices
    EGAM9SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM9SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # track jets
    if saveJets:
        EGAM9SlimmingHelper.ExtraVariables += [
            "AntiKt4PV0TrackJets.pt.eta.phi.e.m.btaggingLink.constituentLinks"
        ]

    # photons and electrons: detailed shower shape variables and track variables
    EGAM9SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent
    EGAM9SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    EGAM9SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM9Kernel")
    print("EGAM9 gain decorations: ", gainDecorations)
    EGAM9SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM9Kernel")
    print("EGAM9 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM9SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM9SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    # truth
    if flags.Input.isMC:
        EGAM9SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM9SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM9SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM9SlimmingHelper.IncludeEGammaTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM9SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM9TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM9SlimmingHelper)

    # Add CellContainer and cluster->cell links
    if keepCells:
        if thinCells:
            EGAM9SlimmingHelper.StaticContent = [
                "CaloCellContainer#DFEGAMCellContainer",
                "CaloClusterCellLinkContainer#egammaClusters_links",
            ]
        else:
            EGAM9SlimmingHelper.StaticContent = [
                "CaloCellContainer#AllCalo",
                "CaloClusterCellLinkContainer#egammaClusters_links",
            ]

    EGAM9ItemList = EGAM9SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM9",
            ItemList=EGAM9ItemList,
            AcceptAlgs=["EGAM9Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM9",
            AcceptAlgs=["EGAM9Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
