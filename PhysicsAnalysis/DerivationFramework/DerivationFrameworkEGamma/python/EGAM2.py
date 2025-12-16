# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# ====================================================================
# EGAM2.py
# This defines DAOD_EGAM2, a skimmed DAOD format for Run 3.
# J/psi->ee derivation for calibration
# It requires the flag EGAM2 in Derivation_tf.py
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
    JPsiTriggers,
)


def EGAM2SkimmingToolCfg(flags):
    """Configure the EGAM2 skimming tool"""
    acc = ComponentAccumulator()

    skimmingTools = []

    # off-line based selection
    expression_calib = (
        "(count(EGAM2_DiElectronMass1 > 1.0*GeV && "
        + "EGAM2_DiElectronMass1 < 5.0*GeV)>=1)"
    )
    expression_TP = (
        "(count(EGAM2_DiElectronMass2 > 1.0*GeV && "
        + "EGAM2_DiElectronMass2 < 6.0*GeV)>=1)"
    )
    expression = expression_calib + " || " + expression_TP
    print("EGAM2 offline skimming expression: ", expression)

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    EGAM2_OfflineSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "EGAM2_OfflineSkimmingTool", expression = expression))
    skimmingTools += [EGAM2_OfflineSkimmingTool]

    # trigger-based selection
    MenuType = ""
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"

    if MenuType:
        triggers = JPsiTriggers[MenuType]
        print("EGAM2 trigger skimming list (OR): ", triggers)

        EGAM2_TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool(
            name="EGAM2_TriggerSkimmingTool", TriggerListOR=triggers
        )
        acc.addPublicTool(EGAM2_TriggerSkimmingTool)
        skimmingTools += [EGAM2_TriggerSkimmingTool]

    # do the OR of trigger-based and offline-based selection
    print("EGAM2 skimming is logical OR of previous selections")
    EGAM2_SkimmingTool = CompFactory.DerivationFramework.FilterCombinationOR(
        name="EGAM2_SkimmingTool",
        FilterList=skimmingTools,
    )

    acc.addPublicTool(EGAM2_SkimmingTool, primary=True)

    return acc


def EGAM2JpsieeMassTool1Cfg(flags):
    """Configure the EGAM2 ee invariant mass augmentation tool 1"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 1. di-electron invariant mass for events passing the J/psi->ee
    #    selection for e/gamma calibration
    #
    #    2 tight or medium e (depends on Run2 triggers..), pT>4.5 GeV, OS
    #    1<Mee<5 GeV (applied in skimming step later)
    # ====================================================================
    electronPtRequirement = "(Electrons.pt > 4.5*GeV)"
    electronQualityRequirement = "(Electrons.DFCommonElectronsLHMedium)"
    requirement_el = (
        "(" + electronQualityRequirement + "&&" + electronPtRequirement + ")"
    )

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM2_JpsieeMassTool1",
            Object1Requirements=requirement_el,
            Object2Requirements=requirement_el,
            StoreGateEntryName="EGAM2_DiElectronMass1",
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


def EGAM2JpsieeMassTool2Cfg(flags):
    """Configure the EGAM2 ee invariant mass augmentation tool 2"""
    acc = ComponentAccumulator()

    # ====================================================================
    # 2. di-electron invariant mass for events passing the J/psi->ee
    #    selection based on Jpsi->e+cluster trigger, for low pT (7-20 GeV)
    #    central photon efficiencies with tag and probe
    #
    # Tag: 1 tight e, central, pT>4.5 GeV
    # Probe: 1 e, central, pT>4.5 GeV
    # OS+SS
    # dR>0.15
    # 1<mee<6 GeV (applied in skimming step later)
    # ====================================================================
    requirement_el_tag = " && ".join(
        ["(Electrons.DFCommonElectronsLHTight)", "(Electrons.pt > 4.5*GeV)"]
    )
    requirement_el_probe = "Electrons.pt > 4.5*GeV"

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGInvariantMassTool(
            name="EGAM2_JpsieeMassTool2",
            Object1Requirements=requirement_el_tag,
            Object2Requirements=requirement_el_probe,
            StoreGateEntryName="EGAM2_DiElectronMass2",
            Mass1Hypothesis=0.511 * MeV,
            Mass2Hypothesis=0.511 * MeV,
            Container1Name="Electrons",
            Container2Name="Electrons",
            CheckCharge=False,
            DoTransverseMass=False,
            MinDeltaR=0.15,
        )
    )

    return acc


def EGAM2KernelCfg(flags, name="EGAM2Kernel", **kwargs):
    """Configure the derivation framework driving algorithm (kernel)
    for EGAM2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg

    acc.merge(
        PhysCommonAugmentationsCfg(
            flags, TriggerListsHelper=kwargs["TriggerListsHelper"]
        )
    )

    # EGAM2 augmentations
    augmentationTools = []

    # ====================================================================
    # ee invariant masses
    # ====================================================================
    EGAM2JpsieeMassTool1 = acc.popToolsAndMerge(EGAM2JpsieeMassTool1Cfg(flags))
    acc.addPublicTool(EGAM2JpsieeMassTool1)
    augmentationTools.append(EGAM2JpsieeMassTool1)

    EGAM2JpsieeMassTool2 = acc.popToolsAndMerge(EGAM2JpsieeMassTool2Cfg(flags))
    acc.addPublicTool(EGAM2JpsieeMassTool2)
    augmentationTools.append(EGAM2JpsieeMassTool2)

    # ====================================================================
    # Gain and cluster energies per layer decoration tool
    # ====================================================================
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloDecoratorKernelCfg,
    )

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
            EGAM2ElectronTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM2ElectronTPThinningTool",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 0*GeV",
                    BestMatchOnly=True,
                    ConeSize=0.3,
                )
            )
            acc.addPublicTool(EGAM2ElectronTPThinningTool)
            thinningTools.append(EGAM2ElectronTPThinningTool)

        # Tracks associated with Electrons (all tracks, large cone, for track
        # isolation studies of the selected electrons)
        if TrackThinningKeepAllElectronTracks:
            EGAM2ElectronTPThinningTool2 = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM2ElectronTPThinningTool2",
                    StreamName=streamName,
                    SGKey="Electrons",
                    GSFTrackParticlesKey="GSFTrackParticles",
                    InDetTrackParticlesKey="InDetTrackParticles",
                    SelectionString="Electrons.pt > 4*GeV",
                    BestMatchOnly=False,
                    ConeSize=0.6,
                )
            )
            acc.addPublicTool(EGAM2ElectronTPThinningTool2)
            thinningTools.append(EGAM2ElectronTPThinningTool2)

        # Tracks associated with Photons
        if TrackThinningKeepPhotonTracks:
            EGAM2PhotonTPThinningTool = (
                CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                    name="EGAM2PhotonTPThinningTool",
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
            acc.addPublicTool(EGAM2PhotonTPThinningTool)
            thinningTools.append(EGAM2PhotonTPThinningTool)

        # Tracks associated with Jets
        if TrackThinningKeepJetTracks:
            EGAM2JetTPThinningTool = (
                CompFactory.DerivationFramework.JetTrackParticleThinning(
                    name="EGAM2JetTPThinningTool",
                    StreamName=streamName,
                    JetKey="AntiKt4EMPFlowJets",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            acc.addPublicTool(EGAM2JetTPThinningTool)
            thinningTools.append(EGAM2JetTPThinningTool)

        # Tracks associated with Muons
        if TrackThinningKeepMuonTracks:
            EGAM2MuonTPThinningTool = acc.getPrimaryAndMerge(
                MuonTrackParticleThinningCfg(
                    flags,
                    name="EGAM2MuonTPThinningTool",
                    StreamName=streamName,
                    MuonKey="Muons",
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM2MuonTPThinningTool)

        # Tracks associated with Taus
        if TrackThinningKeepTauTracks:
            EGAM2TauTPThinningTool = acc.getPrimaryAndMerge(
                TauTrackParticleThinningCfg(
                    flags,
                    name="EGAM2TauTPThinningTool",
                    StreamName=streamName,
                    TauKey="TauJets",
                    ConeSize=0.6,
                    InDetTrackParticlesKey="InDetTrackParticles",
                    DoTauTracksThinning=True,
                    TauTracksKey="TauTracks",
                )
            )
            thinningTools.append(EGAM2TauTPThinningTool)

        # Tracks from primary vertex
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.DFCommonTightPrimary)",
                "(abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta)<3*mm)",
                "(InDetTrackParticles.pt > 10*GeV)",
            ]
        )
        if TrackThinningKeepPVTracks:
            EGAM2TPThinningTool = acc.getPrimaryAndMerge(
                TrackParticleThinningCfg(
                    flags,
                    name="EGAM2TPThinningTool",
                    StreamName=streamName,
                    SelectionString=thinning_expression,
                    InDetTrackParticlesKey="InDetTrackParticles",
                )
            )
            thinningTools.append(EGAM2TPThinningTool)

    # skimming
    skimmingTool = acc.getPrimaryAndMerge(EGAM2SkimmingToolCfg(flags))

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


def EGAM2Cfg(flags):
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down
    # in the config chain for actually configuring the matching, so we create
    # it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run
    # multiple times in a train.
    # TODO: restrict it to relevant triggers
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper

    EGAM2TriggerListsHelper = TriggerListsHelper(flags)

    # configure skimming/thinning/augmentation tools
    acc.merge(
        EGAM2KernelCfg(
            flags,
            name="EGAM2Kernel",
            StreamName="StreamDAOD_EGAM2",
            TriggerListsHelper=EGAM2TriggerListsHelper,
        )
    )

    # configure slimming
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    EGAM2SlimmingHelper = SlimmingHelper(
        "EGAM2SlimmingHelper",
        NamesAndTypes=flags.Input.TypedCollections,
        flags=flags,
    )

    # ------------------------------------------
    # containers for which we save all variables
    # -------------------------------------------

    # baseline
    EGAM2SlimmingHelper.AllVariables = [
        "Electrons",
        "GSFTrackParticles",
        "egammaClusters",
    ]

    # for trigger studies we also add:
    MenuType = ""
    if flags.Trigger.EDMVersion == 2:
        MenuType = "Run2"
    elif flags.Trigger.EDMVersion == 3:
        MenuType = "Run3"

    if MenuType:
        EGAM2SlimmingHelper.AllVariables += ExtraContainersTrigger[MenuType]
        EGAM2SlimmingHelper.AllVariables += ExtraContainersElectronTrigger[MenuType]

    # and on MC we also add:
    if flags.Input.isMC:
        EGAM2SlimmingHelper.AllVariables += [
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
    EGAM2SlimmingHelper.SmartCollections = [
        "Electrons",
        "Photons",
        "Muons",
        "TauJets",
        "InDetTrackParticles",
        "PrimaryVertices",
        "AntiKt4EMPFlowJets",
        "MET_Baseline_AntiKt4EMPFlow",

    ]
    if flags.Input.isMC:
        EGAM2SlimmingHelper.SmartCollections += [
            "AntiKt4TruthJets",
            "AntiKt4TruthDressedWZJets",
        ]

    # then add extra variables:

    # muons
    EGAM2SlimmingHelper.ExtraVariables += [
        "Muons.ptcone20.ptcone30.ptcone40.etcone20.etcone30.etcone40"
    ]

    # conversion vertices
    EGAM2SlimmingHelper.ExtraVariables += [
        "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo",
        "GSFConversionVertices.trackParticleLinks",
    ]

    # primary vertices
    EGAM2SlimmingHelper.ExtraVariables += ["PrimaryVertices.x.y.sumPt2"]

    # photons: detailed shower shape variables
    EGAM2SlimmingHelper.ExtraVariables += PhotonsCPDetailedContent

    # photons: gain and cluster energy per layer
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        getGainDecorations,
        getClusterEnergyPerLayerDecorations,
    )

    gainDecorations = getGainDecorations(acc, flags, "EGAM2Kernel")
    print("EGAM2 gain decorations: ", gainDecorations)
    EGAM2SlimmingHelper.ExtraVariables.extend(gainDecorations)
    clusterEnergyDecorations = getClusterEnergyPerLayerDecorations(acc, "EGAM2Kernel")
    print("EGAM2 cluster energy decorations: ", clusterEnergyDecorations)
    EGAM2SlimmingHelper.ExtraVariables.extend(clusterEnergyDecorations)

    # energy density
    EGAM2SlimmingHelper.ExtraVariables += [
        "TopoClusterIsoCentralEventShape.Density",
        "TopoClusterIsoForwardEventShape.Density",
        "NeutralParticleFlowIsoCentralEventShape.Density",
        "NeutralParticleFlowIsoForwardEventShape.Density",
    ]

    # truth
    if flags.Input.isMC:
        EGAM2SlimmingHelper.ExtraVariables += [
            "MuonTruthParticles.e.px.py.pz.status.pdgId.truthOrigin.truthType"
        ]

        EGAM2SlimmingHelper.ExtraVariables += [
            "Photons.truthOrigin.truthType.truthParticleLink"
        ]

    # Add event info
    if flags.Derivation.Egamma.doEventInfoSlimming:
        EGAM2SlimmingHelper.SmartCollections.append("EventInfo")
    else:
        EGAM2SlimmingHelper.AllVariables += ["EventInfo"]

    # Add egamma trigger objects
    EGAM2SlimmingHelper.IncludeEGammaTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = EGAM2SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = EGAM2TriggerListsHelper.Run2TriggerNamesNoTau)

        # add some single-leg trigger matching info needed for Jpsi triggers
        # This is scheduling an instance of TriggerMatchingCommonRun2Cfg which is in addition to the ones from PhysCommonAugmentationsCfg
        # And adding the output branches to the SlimmingHelper in addition
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import (
            TriggerMatchingCommonRun2Cfg
        )
        EGAM2TrigMatchList = [
            "HLT_e5_lhtight_nod0",
            "HLT_e9_lhtight_nod0",
            "HLT_e14_lhtight_nod0",
        ]
        acc.merge(TriggerMatchingCommonRun2Cfg(
            flags,
            name="EGAM2TrigMatch",
            OutputContainerPrefix="TrigMatch_",
            ChainNames=EGAM2TrigMatchList)
        )
        AddRun2TriggerMatchingToSlimmingHelper(
            SlimmingHelper=EGAM2SlimmingHelper,
            OutputContainerPrefix="TrigMatch_",
            TriggerList=EGAM2TrigMatchList
        )

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAM2SlimmingHelper)


    EGAM2ItemList = EGAM2SlimmingHelper.GetItemList()
    acc.merge(
        OutputStreamCfg(
            flags,
            "DAOD_EGAM2",
            ItemList=EGAM2ItemList,
            AcceptAlgs=["EGAM2Kernel"],
        )
    )
    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            "DAOD_EGAM2",
            AcceptAlgs=["EGAM2Kernel"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.TruthMetaData,
            ],
        )
    )

    return acc
