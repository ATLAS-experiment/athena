# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM4.py
# This defines DAOD_EGAM4, a skimmed DAOD format for Run 3.
# Z->mumugamma and mumue reduction for photon (and fake photon->e) studies
# It requires the flag EGAM4 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from AthenaCommon.SystemOfUnits import MeV

from DerivationFrameworkEGamma.ElectronsCPDetailedContent import (
    ElectronsCPDetailedContent,
    GSFTracksCPDetailedContent,
)

from DerivationFrameworkEGamma.TriggerContent import (
    ExtraContainersTrigger,
    ExtraContainersPhotonTrigger,
    ExtraContainersMuonTrigger,
    ExtraContainersTriggerDataOnly,
)


def EGAM4SkimmingToolCfg(flags):
    """Configure the EGAM4 skimming tool"""
    acc = ComponentAccumulator()

    # mumugamma: one reco photon (ET>10 GeV) and OS muon pair w/ m>40 GeV
    expression1a = " && ".join(
        [
            "(count(DFCommonPhotons_et>9.5*GeV)>=1)",
            "(count(EGAM4_DiMuonMass > 40.0*GeV)>=1)",
        ]
    )

    # mumue: one reco e (ET>10 GeV) and OS muon pair w/ m>40 GeV
    expression1b = " && ".join(
        ["(count(Electrons.pt>9.5*GeV)>=1)", "(count(EGAM4_DiMuonMass > 40.0*GeV)>=1)"]
    )

    # take OR of previous selections
    expression = "( " + expression1a + " ) || ( " + expression1b + " )"
    print("EGAM4 skimming expression: ", expression)

    acc.setPrivateTools(
        CompFactory.DerivationFramework.xAODStringSkimmingTool(
            name="EGAM4SkimmingTool", expression=expression
        )
    )

    return acc


def EGAM4mumuMassToolCfg(flags):
    """Configure the EGAM4 mumu invariant mass augmentation tool"""
    acc = ComponentAccumulator()

    # ====================================================================
    # invariant mass of two OS muons with pT>10 GeV passing preselection
    # ====================================================================

    requirementMuons = " && ".join(
        ["Muons.pt>9.5*GeV", "abs(Muons.eta)<2.7", "Muons.DFCommonMuonPassPreselection"]
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM4_MuMuMassTool",
            Object1Requirements=requirementMuons,
            Object2Requirements=requirementMuons,
            StoreGateEntryName="EGAM4_DiMuonMass",
            Mass1Hypothesis=105 * MeV,
            Mass2Hypothesis=105 * MeV,
            Container1Name="Muons",
            Container2Name="Muons",
            CheckCharge=True,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


# Main algorithm config
def EGAM4KernelCfg(flags, name="EGAM4Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM4"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM4 augmentations
    augmentationTools = []

    # ====================================================================
    # ee and egamma invariant masses
    # ====================================================================
    EGAM4mumuMassTool = acc.popToolsAndMerge(EGAM4mumuMassToolCfg(flags))
    acc.addPublicTool(EGAM4mumuMassTool)
    augmentationTools.append(EGAM4mumuMassTool)

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
        TrackThinningKeepAllElectronTracks = False
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepAllPhotonTracks = True
        TrackThinningKeepJetTracks = False
        TrackThinningKeepMuonTracks = False
        TrackThinningKeepTauTracks = False
        TrackThinningKeepPVTracks = True

        # Tracks associated with Electrons
        if TrackThinningKeepElectronTracks:
            EGAM4ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM4ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM4ElectronTPThinningTool)
            thinningTools.append(EGAM4ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM4ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM4ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM4ElectronTPThinningTool2)
            thinningTools.append(EGAM4ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM4PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM4PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM4PhotonTPThinningTool)
            thinningTools.append(EGAM4PhotonTPThinningTool)

        # Tracks associated with Photons (all tracks, large cone,
        # for track isolation studies of the selected photons)
        if TrackThinningKeepAllPhotonTracks:
            EGAM4PhotonTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM4PhotonTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Photons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    GSFConversionVerticesKey="GSFConversionVertices",
                    SelectionString="Photons.pt > 9.5*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM4PhotonTPThinningTool2)
            thinningTools.append(EGAM4PhotonTPThinningTool2)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM4JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM4JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM4JetTPThinningTool)
            thinningTools.append(EGAM4JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM4MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM4MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM4MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM4TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM4TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM4TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM4TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM4TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM4TPThinningTool)

    # skimming
    skimmingTool = acc.popToolsAndMerge(EGAM4SkimmingToolCfg(flags))
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


def EGAM4Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM4TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM4KernelCfg(
            flags,
            name="EGAM4Kernel",
            StreamName="StreamDAOD_EGAM4",
            TriggerListsHelper=EGAM4TriggerListsHelper,
        )
    )

    # To have ptcone40
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(
        DerivationTrackIsoCfg(
            flags, object_types=("Photons",), ptCuts=(500, 1000), postfix="Extra"
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM4SlimmingHelper = SlimmingHelper(
        "EGAM4SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM4SlimmingHelper.AllVariables = [
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
    EGAM4SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
    EGAM4SlimmingHelper.AllVariables += ExtraContainersPhotonTrigger[MenuType]
    EGAM4SlimmingHelper.AllVariables += ExtraContainersMuonTrigger[MenuType]
    if not flags.Input.isMC:
        EGAM4SlimmingHelper.AllVariables += ExtraContainersTriggerDataOnly[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM4SlimmingHelper.AllVariables += [
            "TruthEvents",
            "TruthParticles",
            "TruthVertices",
            "egammaTruthParticles",
            "MuonTruthParticles",
        ]

    # -------------------------------------------
    # containers that we slim
    # -------------------------------------------

    # first add variables from smart-slimming
    # adding only also those for which we add all variables since
    # the XXXCPContent.py files also bring in some extra variables
    # for other collections
    EGAM4SlimmingHelper.SmartCollections = [
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
        EGAM4SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]
    if flags.Derivation.Egamma.addHLTJets:
        EGAM4SlimmingHelper.SmartCollections += [
            "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf",
        ]

    # then add extra variables:

    # electrons
    EGAM4SlimmingHelper.ExtraVariables += ["Electrons.Loose.Medium.Tight"]

    # muons
    EGAM4SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM4SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM4SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # energy density
    EGAM4SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    # electrons: detailed shower shape and track variables
    EGAM4SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    EGAM4SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    # photons and electrons: gain and cluster energy per layer
    # code would not be needed for photons since we save every photon variable
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM4Kernel")
    print("EGAM4 gain decorations: ", gainDecorations)
    EGAM4SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM4Kernel")
    print("EGAM4 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM4SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # truth
    if flags.Input.isMC:
        EGAM4SlimmingHelper.ExtraVariables += [
            "Electrons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM4SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM4SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM4SlimmingHelper.IncludeEGammaTriggerContent = True
    EGAM4SlimmingHelper.IncludeMuonTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM4SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM4TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM4SlimmingHelper)

    # Add full CellContainer
    EGAM4SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
    ]

    EGAM4ItemList = EGAM4SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM4",
            ItemList=EGAM4ItemList,
            AcceptAlgs=["EGAM4Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM4",
            AcceptAlgs=["EGAM4Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
