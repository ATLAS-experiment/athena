# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# Slimmed DAOD_PHYSLITE.py for Run 3 trigger-object level analyses (TLAs)
# It contains minimal variables needed for the Run 3 ISR+DiJet TLA searches
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Skimming config
def TLA2SkimmingCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkTLA.TLATriggerList import SupportPhotonTriggers, PrimaryISRTLATriggers, SupportTLATriggers, FTagPEBTLATriggers
    
    tlaLiteTriggerList = PrimaryISRTLATriggers + SupportTLATriggers + SupportPhotonTriggers + FTagPEBTLATriggers


    if not flags.Input.isMC:
        TLA2TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( 
                        name          = "TLA2TriggerSkimmingTool1",
                        TriggerListOR = tlaLiteTriggerList
        )
        acc.addPublicTool(TLA2TriggerSkimmingTool, primary=True)
    
    return acc


# Main thinning config and common augmentations
def TLA2KernelCfg(flags, name='TLA2Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TLA2"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTool = None
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(TLA2SkimmingCfg(flags))

    # Common augmentations
    from DerivationFrameworkTLA.TLACommonConfig import TLACommonAugmentationsCfg
    acc.merge(TLACommonAugmentationsCfg(flags, prefix="TLA2_", TriggerListsHelper = kwargs['TriggerListsHelper']))

    # Jets
    from DerivationFrameworkJetEtMiss.JetCommonConfig import JetCommonCfg
    acc.merge(JetCommonCfg(flags))

    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFCommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFTLA2Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("TLA2CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg

    # Include inner detector tracks associated with muons
    TLA2MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "TLA2MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    TLA2ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "TLA2ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA2_thinning_expression = "InDetTrackParticles.DFTLA2Loose && ( abs(InDetTrackParticles.d0) < 5.0*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"

    TLA2Akt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                    = "TLA2Akt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMTopoJets",
        SelectionString         = "AntiKt4EMTopoJets.pt > 18*GeV",
        TrackSelectionString    = TLA2_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA2Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "TLA2Akt4PFlowJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt4EMPFlowJets",
        SelectionString              = "AntiKt4EMPFlowJets.pt > 18*GeV",
        TrackSelectionString         = TLA2_thinning_expression,
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [TLA2MuonTPThinningTool,
                     TLA2ElectronTPThinningTool,
                     TLA2Akt4JetTPThinningTool,
                     TLA2Akt4PFlowJetTPThinningTool]
    
    # create the derivation kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name, 
        ThinningTools = thinningTools,
        SkimmingTools = [skimmingTool] if skimmingTool is not None else []
    ))


    return acc

# Main setup of the config & format
def TLA2Cfg(flags):
    stream_name = 'StreamDAOD_TLA2'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TLA2TriggerListsHelper = TriggerListsHelper(flags)


    # Common augmentations and TLA2 thinning & skimming
    acc.merge(TLA2KernelCfg(flags, name="TLA2Kernel", StreamName = stream_name, TriggerListsHelper = TLA2TriggerListsHelper))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    TLA2SlimmingHelper = SlimmingHelper("TLA2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    TLA2SlimmingHelper.SmartCollections = [
                        "EventInfo",
                        "Electrons", 
                        "Photons",
                        "PrimaryVertices",
                        "Muons",
                        "AntiKt4EMTopoJets",
                        "AntiKt4EMPFlowJets",

    ]
    
    # Extra content
    if flags.Input.isMC:
        TLA2SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "TruthPrimaryVertices.t.x.y.z",
                                                        
            "EventInfo.hardScatterVertexLink.timeStampNSOffset",
        ]
    else:
        TLA2SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_truthjet_nCharged.HECQuality.TrackSumMass.PSFrac.TrackSumPt.SumPtChargedPFOPt1000.EMFrac.Width.JetEMScaleMomentum_pt.JetEMScaleMomentum_eta.TracksForBTagging.SumPtTrkPt1000.TrackWidthPt500",
                                                                    
            "EventInfo.hardScatterVertexLink.timeStampNSOffset",

            "PrimaryVertices.neutralWeights.chiSquared.numberDoF.sumPt2.covariance.trackWeights"
        ]
        
    TLA2SlimmingHelper.AllVariables = [
        # store event shape variables to get full objects (also included by jet CP content)
        "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape", # EMTopo and EMPFlow event shapes
        "Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape", # newer event shapes for testing (e.g. if offline jet calibration changes)
        # store muon segments in case they are needed for offline jet calibrations
        "MuonSegments",
    ]

    # add eEM RoIs
    # based on L1CALOCore.py implementation
    L1Calo_eEM_postfix = "" # empty unless otherwise set
    # append to slimming helper dictionaties so that the code knows the container type
    TLA2SlimmingHelper.AppendToDictionary.update(
        {"L1_eEMRoI"+L1Calo_eEM_postfix :  "xAOD::eFexEMRoIContainer",
         "L1_eEMRoI"+L1Calo_eEM_postfix+"Aux" : "xAOD::eFexEMRoIAuxContainer"})
    # add the RoIs to the derivation   
    TLA2SlimmingHelper.AllVariables += ["L1_eEMRoI"+L1Calo_eEM_postfix]

    
    # Truth extra content
    if flags.Input.isMC:
        from DerivationFrameworkTLA.TLACommonConfig import addTLATruth3ContentToSlimmerTool
        addTLATruth3ContentToSlimmerTool(TLA2SlimmingHelper)
        TLA2SlimmingHelper.AllVariables += [
            'TruthHFWithDecayParticles',
            'TruthHFWithDecayVertices',
            'TruthCharm',
            'TruthPileupParticles',
            'InTimeAntiKt4TruthJets',
            'OutOfTimeAntiKt4TruthJets',
        ]
        TLA2SlimmingHelper.ExtraVariables += [
                                    "Electrons.TruthLink",
                                    "Photons.TruthLink"
                                ]
        # truth jet collections for calibrations and performance studies
        # replicates jet collection configuration in JETM1 (with the exception of AntiKt4TruthDressedWZJets which doesn't exist there)
        TLA2SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]
        TLA2SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "AntiKt4TruthDressedWZJets"]
   
    # Trigger content
    # only save B-jet trigger content and trigger navigation when running on MC
    TLA2SlimmingHelper.IncludeTriggerNavigation = True
    TLA2SlimmingHelper.IncludeJetTriggerContent = True
    TLA2SlimmingHelper.IncludeMuonTriggerContent = False
    TLA2SlimmingHelper.IncludeTrackingTriggerContent = True
    TLA2SlimmingHelper.IncludeEGammaTriggerContent = True
    TLA2SlimmingHelper.IncludeTauTriggerContent = False
    TLA2SlimmingHelper.IncludeEtMissTriggerContent = False
    TLA2SlimmingHelper.IncludeBJetTriggerContent = True
    TLA2SlimmingHelper.IncludeBPhysTriggerContent = False
    TLA2SlimmingHelper.IncludeMinBiasTriggerContent = False
    TLA2SlimmingHelper.OverrideJetTriggerContentWithTLAContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA2SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TLA2TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA2SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TLA2TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TLA2SlimmingHelper)     

    # Output stream    
    TLA2ItemList = TLA2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TLA2", ItemList=TLA2ItemList, AcceptAlgs=["TLA2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TLA2", AcceptAlgs=["TLA2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

