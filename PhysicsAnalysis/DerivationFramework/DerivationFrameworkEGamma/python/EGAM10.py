# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM10.py
# This defines DAOD_EGAM10, a skimmed DAOD format for Run 3.
# Inclusive photon reduction - for e/gamma photon studies
# (migrated from r21 STDM2)
# It requires the flag EGAM10 in Derivation_tf.py
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

from DerivationFrameworkEGamma.PhotonsCPDetailedContent import (
    PhotonsCPDetailedContent,
)

from DerivationFrameworkEGamma.TriggerContent import (
    singlePhotonTriggers,
    diPhotonTriggers,
    triPhotonTriggers,
    noalgTriggers,
)

electronRequirements = " && ".join(
    [
        "(Electrons.pt > 15*GeV)",
        "(abs(Electrons.eta) < 2.5)",
        "(Electrons.DFCommonElectronsLHLoose)",
    ]
)
photonRequirements = " && ".join(
    ["(DFCommonPhotons_et >= 15*GeV)", "(abs(DFCommonPhotons_eta) < 2.5)"]
)


def EGAM10SkimmingToolCfg(flags):
    """Configure the EGAM10 skimming tool"""
    acc = ComponentAccumulator()

    # off-line based selection
    photonSelection = "(count(" + photonRequirements + ") >= 1)"
    print("EGAM10 offline skimming expression: ", photonSelection)
    EGAM10_OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name="EGAM10_OfflineSkimmingTool", expression=photonSelection
    )

    # trigger-based selection
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"
    else:
        MenuType = ""
    allTriggers = (
        singlePhotonTriggers[MenuType]
        + diPhotonTriggers[MenuType]
        + triPhotonTriggers[MenuType]
        + noalgTriggers[MenuType]
    )
    # remove duplicates
    allTriggers = list(set(allTriggers))

    print("EGAM10 trigger skimming list (OR): ", allTriggers)
    EGAM10_TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
        name="EGAM10_TriggerSkimmingTool", TriggerListOR=allTriggers
    )

    # do the AND of trigger-based and offline-based selection
    print("EGAM10 skimming is logical AND of previous selections")
    EGAM10_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(
        name="EGAM10_SkimmingTool",
        FilterList=[EGAM10_OfflineSkimmingTool, EGAM10_TriggerSkimmingTool],
    )

    acc.addPublicTool(EGAM10_OfflineSkimmingTool)
    acc.addPublicTool(EGAM10_TriggerSkimmingTool)
    acc.addPublicTool(EGAM10_SkimmingTool, primary=True)

    return acc


def EGAM10KernelCfg(flags, name="EGAM10Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM10"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM10 augmentations
    augmentationTools = []

    # ====================================================================
    # PhotonVertexSelectionWrapper decoration tool - needs PhotonPointing tool
    # ====================================================================
    from DerivationFrameworkEGamma.EGammaToolsConfig import (
        PhotonVertexSelectionWrapperKernelCfg)
    acc.merge(PhotonVertexSelectionWrapperKernelCfg(flags))

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
        TrackThinningKeepElectronTracks = True
        TrackThinningKeepPhotonTracks = True
        TrackThinningKeepAllElectronTracks = True

        # Tracks associated with high-pT Electrons (deltaR=0.6)
        if TrackThinningKeepElectronTracks:
            EGAM10ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM10ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString=electronRequirements,
                    BestMatchOnly=True,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM10ElectronTPThinningTool)
            thinningTools.append(EGAM10ElectronTPThinningTool)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM10PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM10PhotonTPThinningTool",
                    StreamName=streamName,
                    SGKey="Photons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    GSFConversionVerticesKey="GSFConversionVertices",
                    SelectionString=photonRequirements,
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM10PhotonTPThinningTool)
            thinningTools.append(EGAM10PhotonTPThinningTool)

        # Tracks associated with all Electrons (for ambiguity resolver tool)
        if TrackThinningKeepAllElectronTracks:
            EGAM10ElectronTPThinningToolAR = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM10ElectronTPThinningToolAR",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString=electronRequirements,
                    BestMatchOnly=True,
                )
            )
            acc.addPublicTool(EGAM10ElectronTPThinningToolAR)
            thinningTools.append(EGAM10ElectronTPThinningToolAR)

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM10SkimmingToolCfg(flags))

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


def EGAM10Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # TODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM10TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM10KernelCfg(
            flags,
            name="EGAM10Kernel",
            StreamName="StreamDAOD_EGAM10",
            TriggerListsHelper=EGAM10TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM10SlimmingHelper = SlimmingHelper(
        "EGAM10SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        ConfigFlags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM10SlimmingHelper.AllVariables = ["CaloCalTopoClusters"]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM10SlimmingHelper.AppendToDictionary.update(
            {
                "TruthIsoCentralEventShape": "xAOD::EventShape",
                "TruthIsoCentralEventShapeAux": "xAOD::EventShapeAuxInfo",
                "TruthIsoForwardEventShape": "xAOD::EventShape",
                "TruthIsoForwardEventShapeAux": "xAOD::EventShapeAuxInfo",
            }
        )
        EGAM10SlimmingHelper.AllVariables += [
            "TruthEvents",
            "TruthParticles",
            "TruthVertices",
            "TruthMuons",
            "TruthElectrons",
            "TruthPhotons",
            "TruthNeutrinos",
            "TruthTaus",
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
            "egammaTruthParticles",
            "TruthIsoCentralEventShape",
            "TruthIsoForwardEventShape",
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
    EGAM10SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "InDetTrackParticles",
        "PrimaryVertices",
        "AntiKt4EMPFlowJets",
    ]

    if flags.Input.isMC:
        EGAM10SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # egamma clusters
    EGAM10SlimmingHelper.ExtraVariables += [
        "egammaClusters.PHI2CALOFRAME.ETA2CALOFRAME.phi_sampl"
    ]

    # photons
    EGAM10SlimmingHelper.ExtraVariables += [
        "Photons.ptcone30.ptcone40.f3.f3core",
        "Photons.maxEcell_time.maxEcell_energy.maxEcell_gain.maxEcell_onlId",
        "Photons.maxEcell_x.maxEcell_y.maxEcell_z",
        "Photons.ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "Photons.ptcone40_Nonprompt_All_MaxWeightTTVA_pt500",
        "Photons.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500",
        "Photons.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000",
        "Photons.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500",
    ]

    # electrons
    EGAM10SlimmingHelper.ExtraVariables += [
        "Electrons.topoetcone30.topoetcone40.ptcone20.ptcone30",
        "Electrons.ptcone40.maxEcell_time.maxEcell_energy.maxEcell_gain",
        "Electrons.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z",
    ]

    # primary vertices
    EGAM10SlimmingHelper.ExtraVariables += [
        "PrimaryVertices.covariance.trackWeights.sumPt2.sumPt",
        "PrimaryVertices.pt.eta.phi",
    ]

    # tracks
    EGAM10SlimmingHelper.ExtraVariables += [
        "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights"
    ]

    # photons and electrons: detailed shower shape variables and track variables
    EGAM10SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM10Kernel")
    print("EGAM10 gain decorations: ", gainDecorations)
    EGAM10SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM10Kernel")
    print("EGAM10 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM10SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM10SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
    ]

    from DerivationFrameworkEGamma import EGammaIsoConfig

    (
        pflowIsoVar,
        densityList,
        densityDict,
        acc1,
    ) = EGammaIsoConfig.makeEGammaCommonIsoCfg(flags)
    acc.merge(acc1)
    EGAM10SlimmingHelper.AppendToDictionary.update(densityDict)
    EGAM10SlimmingHelper.ExtraVariables += densityList + [f"Photons{pflowIsoVar}"]

    # To have ptcone40, needed for efficiency measurement with MM
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(
        DerivationTrackIsoCfg(
            flags, object_types=("Photons",), ptCuts=(500, 1000), postfix="Extra"
        )
    )

    # truth
    if flags.Input.isMC:
        EGAM10SlimmingHelper.ExtraVariables += [
            "Electrons.truthOrigin.truthType.truthParticleLink.truthPdgId",
            "Electrons.lastEgMotherTruthType.lastEgMotherTruthOrigin",
            "Electrons.lastEgMotherTruthParticleLink.lastEgMotherPdgId",
            "Electrons.firstEgMotherTruthType.firstEgMotherTruthOrigin",
            "Electrons.firstEgMotherTruthParticleLink.firstEgMotherPdgId",
        ]

        EGAM10SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

        EGAM10SlimmingHelper.ExtraVariables += [
            "TruthIsoCentralEventShape.DensitySigma.Density.DensityArea",
            "TruthIsoForwardEventShape.DensitySigma.Density.DensityArea",
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM10SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM10SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM10SlimmingHelper.IncludeEGammaTriggerContent = True

    # Add trigger matching info
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            AddRun2TriggerMatchingToSlimmingHelper,
        )

        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=EGAM10SlimmingHelper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=EGAM10TriggerListsHelper.Run2TriggerNamesNoTau,
        )
    # Run 3
    if flags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import (
            AddRun3TrigNavSlimmingCollectionsToSlimmingHelper,
        )

        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM10SlimmingHelper)

    EGAM10ItemList = EGAM10SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM10",
            ItemList=EGAM10ItemList,
            AcceptAlgs=["EGAM10Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM10",
            AcceptAlgs=["EGAM10Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
