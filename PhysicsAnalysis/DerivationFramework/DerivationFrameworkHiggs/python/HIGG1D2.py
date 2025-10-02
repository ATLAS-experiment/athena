# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_HIGG1D2.py
# This defines DAOD_HIGG1D2, a di-photon skimmed DAOD format for Run 3.
# It contains the variables and objects needed for analyses with 
# merged electrons, particularly H->yy*.
# It requires the flag HIGG1D2 in Derivation_tf.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.SystemOfUnits import GeV

# Main algorithm config
def HIGG1D2KernelCfg(flags, name='HIGG1D2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for HIGG1D2"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Common Calo decorator tool (as in EGAM, for calibration of merged electrons)
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
        CaloDecoratorKernelCfg)
    acc.merge(CaloDecoratorKernelCfg(flags))

    # Thinning tools
    from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg
    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import GenericTruthThinningCfg

    # Inner detector group recommendations for indet tracks in analysis
    # https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    HIGG1D2_thinning_expression = "InDetTrackParticles.DFCommonTightPrimary && abs(DFCommonInDetTrackZ0AtPV)*sin(InDetTrackParticles.theta) < 3.0*mm && InDetTrackParticles.pt > 10*GeV"

    HIGG1D2TrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(
        flags,
        name                    = "HIGG1D2TrackParticleThinningTool",
        StreamName              = kwargs['StreamName'],
        SelectionString         = HIGG1D2_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    # Include inner detector tracks associated with muons
    HIGG1D2MuonTPThinningTool = acc.getPrimaryAndMerge(
        MuonTrackParticleThinningCfg(
            flags,
            name                    = "HIGG1D2MuonTPThinningTool",
            StreamName              = kwargs['StreamName'],
            MuonKey                 = "Muons",
            InDetTrackParticlesKey  = "InDetTrackParticles"
        )
    )

    # Include inner detector tracks associated with electrons
    HIGG1D2ElectronTPThinningTool = (
        CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name = "HIGG1D2ElectronTPThinningTool",
            StreamName = kwargs['StreamName'],
            SGKey = "Electrons",
            BestMatchOnly = False
        )
    )
    acc.addPublicTool(HIGG1D2ElectronTPThinningTool)

    # Include inner detector tracks associated with photons
    HIGG1D2PhotonTPThinningTool = (
        CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name = "HIGG1D2PhotonTPThinningTool",
            StreamName = kwargs['StreamName'],
            SGKey = "Photons",
            GSFConversionVerticesKey = "GSFConversionVertices",
            BestMatchOnly = False
        )
    )
    acc.addPublicTool(HIGG1D2PhotonTPThinningTool)

    # Include topoclusters around electrons, needed for calibration of merged electrons
    from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import CaloClusterThinningCfg

    HIGG1D2CCTCThinningTool = acc.getPrimaryAndMerge(
        CaloClusterThinningCfg(
            flags,
            name = "HIGG1D2CCTCThinningTool",
            StreamName = kwargs['StreamName'],
            SGKey = "Electrons",
            SelectionString = "Electrons.pt>4*GeV",
            TopoClCollectionSGKey = "CaloCalTopoClusters",
            ConeSize = 0.5
        )
    )

    thinningTools = [HIGG1D2TrackParticleThinningTool,
                     HIGG1D2MuonTPThinningTool,
                     HIGG1D2ElectronTPThinningTool,
                     HIGG1D2PhotonTPThinningTool,
                     HIGG1D2CCTCThinningTool]

    # Photon vertex 
    from DerivationFrameworkEGamma.EGammaToolsConfig import PhotonVertexSelectionWrapperKernelCfg
    acc.merge(PhotonVertexSelectionWrapperKernelCfg(flags))

    # Truth thinning 
    if flags.Input.isMC :
        truth_cond_1 = "((abs(TruthParticles.pdgId) >= 23) && (abs(TruthParticles.pdgId) <= 25))" # W, Z and Higgs
        truth_cond_2 = "((abs(TruthParticles.pdgId) >= 11) && (abs(TruthParticles.pdgId) <= 16))" # Leptons
        truth_cond_3 = "((abs(TruthParticles.pdgId) ==  6))"                                     # Top quark
        truth_cond_4 = "((abs(TruthParticles.pdgId) == 22) && (TruthParticles.pt > 1*GeV))"       # Photon
        truth_cond_finalState = '(TruthParticles.isGenStable)' # stable particles
        truth_expression = '('+truth_cond_1+' || '+truth_cond_2 +' || '+truth_cond_3 +' || '+truth_cond_4+') || ('+truth_cond_finalState+')'
    
        HIGG1D2GenericTruthThinningTool   = acc.getPrimaryAndMerge(GenericTruthThinningCfg(
            flags,
            name                          = "HIGG1D2GenericTruthThinningTool",
            StreamName                    = kwargs['StreamName'],
            ParticleSelectionString       = truth_expression,
            PreserveDescendants           = False,
            PreserveGeneratorDescendants  = True,
            PreserveAncestors             = True))
        acc.addPublicTool(HIGG1D2GenericTruthThinningTool)
        thinningTools.append(HIGG1D2GenericTruthThinningTool)

    # SkimmingTool
    from DerivationFrameworkHiggs.SkimmingToolHIGG1Config import SkimmingToolHIGG1Cfg
    from DerivationFrameworkHiggs.HIGG1TriggerContent import expressionTriggers, mergedTriggers

    SkipTriggerRequirement = flags.Input.isMC or not flags.Reco.EnableTrigger
    TriggerExp = []
    TriggerMerged = []
    
    if not SkipTriggerRequirement:

        MenuType = None
        if float(flags.Beam.Energy) == 6500000.0:
            # 13 TeV
            MenuType = "Run2"
        elif float(flags.Beam.Energy) == 6800000.0:
            # 13.6 TeV
            MenuType = "Run3"

        TriggerExp = expressionTriggers[MenuType]
        TriggerMerged = mergedTriggers[MenuType]

    # Merged electrons for skimming
    from ROOT import egammaPID

    MergedElectronIsEM = CompFactory.AsgElectronIsEMSelector("MergedElectronIsEM")
    MergedElectronIsEM.ConfigFile = "ElectronPhotonSelectorTools/trigger/rel21_20161021/ElectronIsEMMergedTightSelectorCutDefs.conf"
    MergedElectronIsEM.isEMMask = egammaPID.ElectronTightHLT
    acc.addPublicTool(MergedElectronIsEM)

    # Set up skimming for merged and resolved events
    skimmingTool = acc.popToolsAndMerge( SkimmingToolHIGG1Cfg(flags,
                                                              name = "HIGG1D2SkimmingTool",
                                                              RequireGRL = False,
                                                              ReqireLArError = True,
                                                              RequireTrigger = not SkipTriggerRequirement,
                                                              IncludeDoublePhotonPreselection = False,
                                                              RequirePreselection = False,
                                                              RequireKinematic = False,
                                                              RequireQuality = False,
                                                              RequireIsolation = False,
                                                              RequireInvariantMass = False,
                                                              Triggers = TriggerExp,
                                                              MergedElectronTriggers = TriggerMerged,
                                                              IncludeSingleElectronPreselection = False,
                                                              IncludeDoubleElectronPreselection = False,
                                                              IncludeSingleMuonPreselection = False,
                                                              IncludePhotonDoubleElectronPreselection = True,
                                                              IncludeDoubleMuonPreselection = True,
                                                              IncludePhotonMergedElectronPreselection = True,
                                                              IncludeHighPtPhotonElectronPreselection = True,
                                                              MinimumPhotonPt = 9.9*GeV,
                                                              MinimumElectronPt = 4.4*GeV,
                                                              MinimumMergedElectronPt = 18*GeV,
                                                              MinimumMuonPt = 2.9*GeV,
                                                              MaxMuonEta = 2.7,
                                                              RemoveCrack = False,
                                                              MaxEta = 2.5,
                                                              MergedElectronCutTool = MergedElectronIsEM))

    acc.addPublicTool(skimmingTool)

    # Augmentation tool for merged electron ID
    from DerivationFrameworkHiggs.MergedElectronConfig import MergedElectronDetailsDecoratorCfg
    HIGG1D2MergedElectronDetailsDecorator = acc.getPrimaryAndMerge(MergedElectronDetailsDecoratorCfg(flags,
                                                                                                  name = "HIGG1D2MergedElectronDetailsDecorator"))
    augmentationTools = [HIGG1D2MergedElectronDetailsDecorator]

    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
    contNames = ["Muons", "Electrons", "Photons"]
    acc.merge(IsoCloseByAlgsCfg(flags, suff = "_HIGG1D2", isPhysLite = False, containerNames = contNames, stream_name = kwargs['StreamName']))

    # Kernel now
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name,
                                      SkimmingTools = [skimmingTool],
                                      ThinningTools = thinningTools,
                                      AugmentationTools = augmentationTools))
    return acc


def HIGG1D2Cfg(flags):

    acc = ComponentAccumulator()

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    HIGG1D2TriggerListsHelper = TriggerListsHelper(flags)

    acc.merge(HIGG1D2KernelCfg(flags, name="HIGG1D2Kernel", StreamName = 'StreamDAOD_HIGG1D2', TriggerListsHelper = HIGG1D2TriggerListsHelper))

    # Define contents of the format
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper

    HIGG1D2SlimmingHelper = SlimmingHelper("HIGG1D2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    HIGG1D2SlimmingHelper.SmartCollections = ["EventInfo",
                                              "Electrons",
                                              "Photons",
                                              "Muons",
                                              "PrimaryVertices",
                                              "InDetTrackParticles",
                                              "AntiKt4EMPFlowJets",
                                              "BTagging_AntiKt4EMPFlow"]

    # Trigger content
    HIGG1D2SlimmingHelper.IncludeTriggerNavigation = False
    HIGG1D2SlimmingHelper.IncludeJetTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeMuonTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeEGammaTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeTauTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeEtMissTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeBJetTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeBPhysTriggerContent = False
    HIGG1D2SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Variables to include
    HIGG1D2SlimmingHelper.AllVariables = ["Electrons","Photons","egammaClusters","GSFConversionVertices","PrimaryVertices","MET_Track","CaloCalTopoClusters"]

    HIGG1D2SlimmingHelper.ExtraVariables = ["Photons.zvertex",
                                            "Muons.quality.EnergyLoss.energyLossType.etcone20.ptconecoreTrackPtrCorrection",
                                            "MuonClusterCollection.eta_sampl.phi_sampl",
                                            "GSFTrackParticles.parameterX.parameterY.parameterZ.parameterPX.parameterPY.parameterPZ.parameterPosition.vx.vy.eProbabilityHT",
                                            "InDetTrackParticles.vx.vy.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                            "AntiKt4EMPFlowJets.Jvt.JVFCorr",
                                            "CombinedMuonTrackParticles.z0.vz",
                                            "BTagging_AntiKt4EMTopo.MV1_discriminant",
                                            "ExtrapolatedMuonTrackParticles.z0.vz",
                                            "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.TTVA_AMVFVertices_forReco.TTVA_AMVFWeights_forReco.TTVA_AMVFVertices_forHiggs.TTVA_AMVFWeights_forHiggs.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                            "EventInfo.hardScatterVertexLink.timeStampNSOffset"]
    
    # Add Btagging information
    from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
    HIGG1D2SlimmingHelper.ExtraVariables += BTaggingStandardContent("AntiKt4EMPFlowJets", flags)
    
    # Truth containers
    if flags.Input.isMC:
        HIGG1D2SlimmingHelper.AppendToDictionary = {'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                                    'MET_Truth':'xAOD::MissingETContainer','MET_TruthAux':'xAOD::MissingETAuxContainer',
                                                    'TruthElectrons':'xAOD::TruthParticleContainer','TruthElectronsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthMuons':'xAOD::TruthParticleContainer','TruthMuonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthPhotons':'xAOD::TruthParticleContainer','TruthPhotonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBoson':'xAOD::TruthParticleContainer','TruthBosonAux':'xAOD::TruthParticleAuxContainer',
                                                    'BornLeptons':'xAOD::TruthParticleContainer','BornLeptonsAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBosonsWithDecayParticles':'xAOD::TruthParticleContainer','TruthBosonsWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthBosonsWithDecayVertices':'xAOD::TruthVertexContainer','TruthBosonsWithDecayVerticesAux':'xAOD::TruthVertexAuxContainer',
                                                    'TruthHFWithDecayParticles':'xAOD::TruthParticleContainer','TruthHFWithDecayParticlesAux':'xAOD::TruthParticleAuxContainer',
                                                    'TruthPrimaryVertices':'xAOD::TruthVertexContainer','TruthPrimaryVerticesAux':'xAOD::TruthVertexAuxContainer'}

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(HIGG1D2SlimmingHelper)
        HIGG1D2SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles',
                                               'AntiKt4TruthDressedWZJets',
                                               'AntiKt4TruthWZJets',
                                               'TruthEvents',
                                               'TruthPrimaryVertices',
                                               'TruthVertices',
                                               'TruthParticles',
                                               'TruthElectrons',
                                               'TruthParticles',
                                               'TruthPhotons',
                                               'TruthMuons',
                                               'TruthBoson']
        HIGG1D2SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                                 "Muons.TruthLink",
                                                 "Photons.TruthLink"]


    HIGG1D2SlimmingHelper.AppendToDictionary.update({'MET_Track':'xAOD::MissingETContainer','MET_TrackAux':'xAOD::MissingETAuxContainer'})
    
    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = HIGG1D2SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = HIGG1D2TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = HIGG1D2SlimmingHelper,
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = HIGG1D2TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(HIGG1D2SlimmingHelper)
        
    # Output stream
    HIGG1D2ItemList = HIGG1D2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_HIGG1D2", ItemList=HIGG1D2ItemList, AcceptAlgs=["HIGG1D2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_HIGG1D2", AcceptAlgs=["HIGG1D2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc
