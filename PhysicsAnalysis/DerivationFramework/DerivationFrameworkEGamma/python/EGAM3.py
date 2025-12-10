# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM3.py
# This defines DAOD_EGAM3, a skimmed DAOD format for Run 3.
# Z->eegamma reduction for low-pT electron and photon studies
# It requires the flag EGAM3 in Derivation_tf.py
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
    ExtraContainersElectronTrigger,
    ExtraContainersTriggerDataOnly,
    ExtraVariablesHLTPhotons,
)


def EGAM3SkimmingToolCfg(flags):
    """Configure the EGAM3 skimming tool"""
    acc = ComponentAccumulator()

    # eegamma or eee selection for photon efficiency studies, ee triggers
    expression1a = " && ".join(
        [
            "(count(DFCommonPhotons_et>9.5*GeV)>=1)",
            "(count(EGAM3_DiElectronMass1 > 40.0*GeV)>=1)",
        ]
    )
    expression1b = " && ".join(
        [
            "(count(Electrons.pt>9.5*GeV)>=3)",
            "(count(EGAM3_DiElectronMass1 > 40.0*GeV)>=1)",
        ]
    )

    # eegamma selection for low-pT central electron studies with T&P
    expression2 = " && ".join(
        [
            "(count(DFCommonPhotons_et>9.5*GeV && "
            + "Photons.DFCommonPhotonsIsEMTight)>=1)",
            "(count(EGAM3_DiElectronMass2 > 40.0*GeV)>=1)",
        ]
    )

    # eegamma selection for low-pT forward electron studies with T&P
    expression3 = " && ".join(
        [
            "(count(DFCommonPhotons_et>9.5*GeV && "
            + "Photons.DFCommonPhotonsIsEMTight)>=1)",
            "(count(EGAM3_DiElectronMass3 > 40.0*GeV)>=1)",
        ]
    )

    # take OR of previous selections
    expression = (
        "( "
        + expression1a
        + " ) || "
        + "( "
        + expression1b
        + " ) || "
        + "( "
        + expression2
        + " ) || "
        + "( "
        + expression3
        + " )"
    )
    print("EGAM3 skimming expression: ", expression)

    skimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name="EGAM3SkimmingTool", expression=expression)
    if flags.Trigger.EDMVersion < 0:
        skimmingTool.TrigDecisionTool=None

    acc.setPrivateTools(skimmingTool)

    return acc


def EGAM3eeMassTool1Cfg(flags):
    """Configure the EGAM3 ee invariant mass augmentation tool 1"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 1. ee invariant mass of events passing eegamma or eee selection for
    #    photon efficiency studies, di-electron triggers
    #
    #   two opposite-sign medium el, pT>10 GeV, |eta|<2.5, mee>40 GeV
    #   eegamma: one reco photon, ET>10 GeV< |eta|<2.5
    #   eee: 3 electrons, pT>10 GeV, mee>40 GeV
    # if skim size too large either require tight electrons (at least one)
    # or raise electron pT threshold (at least one)
    # ====================================================================

    requirementElectrons = " && ".join(
        ["(Electrons.DFCommonElectronsLHMedium)", "(Electrons.pt > 9.5*GeV)"]
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM3_EEMassTool11",
            Object1Requirements=requirementElectrons,
            Object2Requirements=requirementElectrons,
            StoreGateEntryName="EGAM3_DiElectronMass1",
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


def EGAM3eeMassTool2Cfg(flags):
    """Configure the EGAM3 ee invariant mass augmentation tool 2"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 2. dielectron invariant mass for eegamma selection for low-pT
    #    electron studies with T&P
    #
    #    tag e: tight, |eta|<2.5, pT>25 GeV
    #    probe e: reco, ET>7 GeV, central electron
    #    gamma: tight, ET>10 GeV
    # ====================================================================
    # asymmetric electron cuts/single e trigger, low pT cut for subleading
    # e (for e calibration studies at low pT)

    requirementElectron1 = " && ".join(
        ["(Electrons.DFCommonElectronsLHTight)", "(Electrons.pt > 24.5*GeV)"]
    )
    requirementElectron2 = "(Electrons.pt > 6.5*GeV)"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM3_ZEEMassTool2",
            Object1Requirements=requirementElectron1,
            Object2Requirements=requirementElectron2,
            StoreGateEntryName="EGAM3_DiElectronMass2",
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


def EGAM3eeMassTool3Cfg(flags):
    """Configure the EGAM3 ee invariant mass augmentation tool 3"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 3. eegamma selection for low-pT electron studies with T&P
    #    tag e: tight, |eta|<2.5, pT>25 GeV
    #    probe e: reco, ET>7 GeV, forward electron
    #    gamma: tight, ET>10 GeV
    # ====================================================================

    requirementElectron1 = " && ".join(
        ["(Electrons.DFCommonElectronsLHTight)", "(Electrons.pt > 24.5*GeV)"]
    )
    requirementElectron2 = "(ForwardElectrons.pt > 6.5*GeV)"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM3_EEMassTool3",
            Object1Requirements=requirementElectron1,
            Object2Requirements=requirementElectron2,
            StoreGateEntryName="EGAM3_DiElectronMass3",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="ForwardElectrons",
            CheckCharge=True,
            DoTransverseMass=False,
            MinDeltaR=0.0,
        )
    )

    return acc


# Main algorithm config
def EGAM3KernelCfg(flags, name="EGAM3Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM3"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM3 augmentations
    augmentationTools = []

    # ====================================================================
    # ee and egamma invariant masses
    # ====================================================================
    EGAM3eeMassTool1 = acc.popToolsAndMerge(EGAM3eeMassTool1Cfg(flags))
    acc.addPublicTool(EGAM3eeMassTool1)
    augmentationTools.append(EGAM3eeMassTool1)

    EGAM3eeMassTool2 = acc.popToolsAndMerge(EGAM3eeMassTool2Cfg(flags))
    acc.addPublicTool(EGAM3eeMassTool2)
    augmentationTools.append(EGAM3eeMassTool2)

    EGAM3eeMassTool3 = acc.popToolsAndMerge(EGAM3eeMassTool3Cfg(flags))
    acc.addPublicTool(EGAM3eeMassTool3)
    augmentationTools.append(EGAM3eeMassTool3)

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
            EGAM3ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM3ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM3ElectronTPThinningTool)
            thinningTools.append(EGAM3ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM3ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM3ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM3ElectronTPThinningTool2)
            thinningTools.append(EGAM3ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM3PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM3PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM3PhotonTPThinningTool)
            thinningTools.append(EGAM3PhotonTPThinningTool)

        # Tracks associated with Photons (all tracks, large cone,
        # for track isolation studies of the selected photons)
        if TrackThinningKeepAllPhotonTracks:
            EGAM3PhotonTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM3PhotonTPThinningTool2",
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
            acc.addPublicTool(EGAM3PhotonTPThinningTool2)
            thinningTools.append(EGAM3PhotonTPThinningTool2)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM3JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM3JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM3JetTPThinningTool)
            thinningTools.append(EGAM3JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM3MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM3MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM3MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM3TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM3TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM3TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt>10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM3TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM3TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM3TPThinningTool)

    # skimming
    skimmingTool = acc.popToolsAndMerge(EGAM3SkimmingToolCfg(flags))
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


def EGAM3Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM3TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM3KernelCfg(
            flags,
            name="EGAM3Kernel",
            StreamName="StreamDAOD_EGAM3",
            TriggerListsHelper=EGAM3TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM3SlimmingHelper = SlimmingHelper(
        "EGAM3SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM3SlimmingHelper.AllVariables = [
        "Photons",
        "GSFTrackParticles",
        "egammaClusters",
        "ForwardElectrons",
        "ForwardElectronClusters",
    ]

    # for trigger studies we also add:
    MenuType = ""
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"

    if MenuType:
        EGAM3SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
        EGAM3SlimmingHelper.AllVariables += ExtraContainersPhotonTrigger[MenuType]
        EGAM3SlimmingHelper.AllVariables += ExtraContainersElectronTrigger[MenuType]
        if not flags.Input.isMC:
            EGAM3SlimmingHelper.AllVariables += ExtraContainersTriggerDataOnly[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM3SlimmingHelper.AllVariables += [
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
    EGAM3SlimmingHelper.SmartCollections = [
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
        EGAM3SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]
    if flags.Derivation.Egamma.addHLTJets:
        EGAM3SlimmingHelper.SmartCollections += [
            "HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf",
        ]

    # then add extra variables:

    # electrons
    EGAM3SlimmingHelper.ExtraVariables += ["Electrons.Loose.Medium.Tight"]

    # muons
    EGAM3SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM3SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM3SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # energy density
    EGAM3SlimmingHelper.ExtraVariables += [
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
    EGAM3SlimmingHelper.AppendToDictionary.update(densityDict)
    EGAM3SlimmingHelper.ExtraVariables += densityList

    # To have ptcone40
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(
        DerivationTrackIsoCfg(
            flags, object_types=("Photons",), ptCuts=(500, 1000), postfix="Extra"
        )
    )

    # electrons: detailed shower shape and track variables
    EGAM3SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    EGAM3SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent

    # photons and electrons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM3Kernel")
    print("EGAM3 gain decorations: ", gainDecorations)
    EGAM3SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM3Kernel")
    print("EGAM3 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM3SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # photon HLT variables
    if MenuType:
        EGAM3SlimmingHelper.ExtraVariables += ExtraVariablesHLTPhotons[MenuType]

    # truth
    if flags.Input.isMC:
        EGAM3SlimmingHelper.ExtraVariables += [
            "Electrons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM3SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM3SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM3SlimmingHelper.IncludeEGammaTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM3SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM3TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM3SlimmingHelper)

    # Add full CellContainer
    EGAM3SlimmingHelper.StaticContent = [
        "CaloCellContainer#AllCalo",
        "CaloClusterCellLinkContainer#egammaClusters_links",
    ]

    EGAM3ItemList = EGAM3SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM3",
            ItemList=EGAM3ItemList,
            AcceptAlgs=["EGAM3Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM3",
            AcceptAlgs=["EGAM3Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
