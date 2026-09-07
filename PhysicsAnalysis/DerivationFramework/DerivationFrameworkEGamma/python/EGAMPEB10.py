# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAMPEB10.py
# This defines DAOD_EGAMPEB10, a skimmed DAOD format for Run 3.
# Inclusive photon reduction - for e/gamma photon studies
# (migrated from r21 STDM2).
# 
# Compared to EGAM10, the trigger matching is removed, MC/Truth is 
# disabled. See EGAMPEBPHYS for more info, and details on jet setup 
# and isolation variables and more.
# 
# It requires the flag EGAMPEB10 in Derivation_tf.py
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

def EGAMPEB10SkimmingToolCfg(flags):
    """Configure the EGAMPEB10 skimming tool"""
    acc = ComponentAccumulator()

    # off-line based selection
    photonSelection = "(count(" + photonRequirements + ") >= 1)"
    print("EGAMPEB10 offline skimming expression: ", photonSelection)
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    EGAMPEB10_OfflineSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name="EGAMPEB10_OfflineSkimmingTool", expression=photonSelection))
    filterList = [EGAMPEB10_OfflineSkimmingTool]

    # trigger-based selection
    MenuType = None
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"
    if MenuType:
        allTriggers = (
            singlePhotonTriggers[MenuType]
            + diPhotonTriggers[MenuType]
            + triPhotonTriggers[MenuType]
            + noalgTriggers[MenuType]
        )
        # remove duplicates
        allTriggers = list(set(allTriggers))
        print("EGAMPEB10 trigger skimming list (OR): ", allTriggers)
        EGAMPEB10_TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name="EGAMPEB10_TriggerSkimmingTool", TriggerListOR=allTriggers
        )
        acc.addPublicTool(EGAMPEB10_TriggerSkimmingTool)
        filterList += [EGAMPEB10_TriggerSkimmingTool]
    else:
        print("Unknown Trigger.EDMVersion ", flags.Trigger.EDMVersion)
        print("Will not apply trigger-based skimming")

    # do the AND of trigger-based and offline-based selection
    print("EGAMPEB10 skimming is logical AND of previous selections")
    EGAMPEB10_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationAND(
        name="EGAMPEB10_SkimmingTool", FilterList=filterList)

    acc.addPublicTool(EGAMPEB10_SkimmingTool, primary=True)
    return acc


def EGAMPEB10KernelCfg(flags, name="EGAMPEB10Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAMPEB10"""
    acc = ComponentAccumulator()

    # Common augmentations
    # from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    from DerivationFrameworkEGamma.EGAMPEBPHYS import EGAMPEBPHYSCommonAugmentationsCfg

    acc.merge(
        EGAMPEBPHYSCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAMPEB10 augmentations
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
        CaloDecoratorKernelCfg
    )
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
            EGAMPEB10ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAMPEB10ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString=electronRequirements,
                    BestMatchOnly=True,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAMPEB10ElectronTPThinningTool)
            thinningTools.append(EGAMPEB10ElectronTPThinningTool)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAMPEB10PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAMPEB10PhotonTPThinningTool",
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
            acc.addPublicTool(EGAMPEB10PhotonTPThinningTool)
            thinningTools.append(EGAMPEB10PhotonTPThinningTool)

        # Tracks associated with all Electrons (for ambiguity resolver tool)
        if TrackThinningKeepAllElectronTracks:
            EGAMPEB10ElectronTPThinningToolAR = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAMPEB10ElectronTPThinningToolAR",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString=electronRequirements,
                    BestMatchOnly=True,
                )
            )
            acc.addPublicTool(EGAMPEB10ElectronTPThinningToolAR)
            thinningTools.append(EGAMPEB10ElectronTPThinningToolAR)

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAMPEB10SkimmingToolCfg(flags))

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


def EGAMPEB10Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # TODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAMPEB10TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAMPEB10KernelCfg(
            flags,
            name="EGAMPEB10Kernel",
            StreamName="StreamDAOD_EGAMPEB10",
            TriggerListsHelper=EGAMPEB10TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAMPEB10SlimmingHelper = SlimmingHelper(
        "EGAMPEB10SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAMPEB10SlimmingHelper.AllVariables = [
        "CaloCalTopoClusters",
        "egammaClusters"
    ]

    # # and on MC we also add:
    # if flags.Input.isMC:
    #     EGAMPEB10SlimmingHelper.AppendToDictionary.update(
    #         {
    #             "TruthIsoCentralEventShape": "xAOD::EventShape",
    #             "TruthIsoCentralEventShapeAux": "xAOD::EventShapeAuxInfo",
    #             "TruthIsoForwardEventShape": "xAOD::EventShape",
    #             "TruthIsoForwardEventShapeAux": "xAOD::EventShapeAuxInfo",
    #         }
    #     )
    #     EGAMPEB10SlimmingHelper.AllVariables += [
    #         "TruthEvents",
    #         "TruthParticles",
    #         "TruthVertices",
    #         "TruthMuons",
    #         "TruthElectrons",
    #         "TruthPhotons",
    #         "TruthNeutrinos",
    #         "TruthTaus",
    #         "AntiKt4TruthJets",
    #         "AntiKt4TruthDressedWZJets",
    #         "egammaTruthParticles",
    #         "TruthIsoCentralEventShape",
    #         "TruthIsoForwardEventShape",
    #     ]

    # -------------------------------------------
    # containers that we slim
    # -------------------------------------------

    # first add variables from smart-slimming
    # adding only also those for which we add all variables since
    # the XXXCPContent.py files also bring in some extra variables
    # for other collections
    # muons, tau, MET, b-tagging could be switched off if not needed
    # and use too much space
    EGAMPEB10SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "InDetTrackParticles",
        "PrimaryVertices",
        "AntiKt4EMPFlowJets",
    ]

    # if flags.Input.isMC:
    #     EGAMPEB10SlimmingHelper.SmartCollections += [
    #         "AntiKt4TruthJets",
    #         "AntiKt4TruthDressedWZJets",
    #     ]

    # then add extra variables:

    # egamma clusters
    EGAMPEB10SlimmingHelper.ExtraVariables += [
        "egammaClusters.PHI2CALOFRAME.ETA2CALOFRAME.phi_sampl",
    ]

    # photons
    EGAMPEB10SlimmingHelper.ExtraVariables += [
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
    EGAMPEB10SlimmingHelper.ExtraVariables += [
        "Electrons.topoetcone30.topoetcone40.ptcone20.ptcone30",
        "Electrons.ptcone40.maxEcell_time.maxEcell_energy.maxEcell_gain",
        "Electrons.maxEcell_onlId.maxEcell_x.maxEcell_y.maxEcell_z",
    ]

    # primary vertices
    EGAMPEB10SlimmingHelper.ExtraVariables += [
        "PrimaryVertices.covariance.trackWeights.sumPt2.sumPt",
        "PrimaryVertices.pt.eta.phi",
    ]

    # tracks
    EGAMPEB10SlimmingHelper.ExtraVariables += [
        "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights"
    ]

    # photons and electrons: detailed shower shape variables and track variables
    EGAMPEB10SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAMPEB10Kernel")
    print("EGAMPEB10 gain decorations: ", gainDecorations)
    EGAMPEB10SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAMPEB10Kernel")
    print("EGAMPEB10 cluster energy decorations: ", clusterEnergyDecorations)
    EGAMPEB10SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAMPEB10SlimmingHelper.ExtraVariables += [
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
    EGAMPEB10SlimmingHelper.AppendToDictionary.update(densityDict)
    EGAMPEB10SlimmingHelper.ExtraVariables += densityList + [f"Photons{pflowIsoVar}"]

    # To have ptcone40, needed for efficiency measurement with MM
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(
        DerivationTrackIsoCfg(
            flags, object_types=("Photons",), ptCuts=(500, 1000), postfix="Extra"
        )
    )

    # truth
    # if flags.Input.isMC:
    #     EGAMPEB10SlimmingHelper.ExtraVariables += [
    #         "Electrons.truthClassification.truthOrigin.truthType.truthParticleLink.truthPdgId",
    #         "Electrons.lastEgMotherTruthClassification.lastEgMotherTruthType.lastEgMotherTruthOrigin",
    #         "Electrons.lastEgMotherTruthParticleLink.lastEgMotherPdgId",
    #         "Electrons.firstEgMotherTruthClassification.firstEgMotherTruthType.firstEgMotherTruthOrigin",
    #         "Electrons.firstEgMotherTruthParticleLink.firstEgMotherPdgId",
    #     ]

    #     EGAMPEB10SlimmingHelper.ExtraVariables += [
    #         "Photons.truthClassification.truthOrigin.truthType.truthParticleLink"
    #     ]

    #     EGAMPEB10SlimmingHelper.ExtraVariables += [
    #         "TruthIsoCentralEventShape.DensitySigma.Density.DensityArea",
    #         "TruthIsoForwardEventShape.DensitySigma.Density.DensityArea",
    #     ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAMPEB10SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAMPEB10SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAMPEB10SlimmingHelper.IncludeEGammaTriggerContent = True

    # Add full CellContainer
    EGAMPEB10SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
    ]

    EGAMPEB10ItemList = EGAMPEB10SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAMPEB10",
            ItemList=EGAMPEB10ItemList,
            AcceptAlgs=["EGAMPEB10Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAMPEB10",
            AcceptAlgs=["EGAMPEB10Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
