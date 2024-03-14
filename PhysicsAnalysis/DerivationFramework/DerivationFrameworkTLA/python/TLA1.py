# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# Slimmed DAOD_PHYSLITE.py for Run 3 trigger-object level analyses (TLAs)
# It contains minimal variables needed for the Run 3 ISR+DiJet TLA searches
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Skimming config
def TLA1SkimmingCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkTLA.TLATriggerList import SupportPhotonTriggers, PrimaryISRTLATriggers, SupportTLATriggers
    
    tlaLiteTriggerList = PrimaryISRTLATriggers + SupportTLATriggers + SupportPhotonTriggers


    if not flags.Input.isMC:
        TLA1TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( 
                        name          = "TLA1TriggerSkimmingTool1",
                        TriggerListOR = tlaLiteTriggerList
        )
        acc.addPublicTool(TLA1TriggerSkimmingTool, primary=True)
    
    return acc


# Main thinning config and common augmentations
def TLA1KernelCfg(flags, name='TLA1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TLA1"""
    acc = ComponentAccumulator()

    # Skimming
    skimmingTool = None
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(TLA1SkimmingCfg(flags))

    # Common augmentations
    from DerivationFrameworkTLA.TLACommonConfig import TLACommonAugmentationsCfg
    acc.merge(TLACommonAugmentationsCfg(flags, prefix="TLA1_", TriggerListsHelper = kwargs['TriggerListsHelper']))
    
    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFCommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFTLA1Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("TLA1CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg

    # Include inner detector tracks associated with muons
    TLA1MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "TLA1MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    TLA1ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "TLA1ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA1_thinning_expression = "InDetTrackParticles.DFTLA1Loose && ( abs(InDetTrackParticles.d0) < 5.0*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"

    TLA1Akt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                    = "TLA1Akt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMTopoJets",
        SelectionString         = "AntiKt4EMTopoJets.pt > 18*GeV",
        TrackSelectionString    = TLA1_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    TLA1Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "TLA1Akt4PFlowJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt4EMPFlowJets",
        SelectionString              = "AntiKt4EMPFlowJets.pt > 18*GeV",
        TrackSelectionString         = TLA1_thinning_expression,
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [TLA1MuonTPThinningTool,
                     TLA1ElectronTPThinningTool,
                     TLA1Akt4JetTPThinningTool,
                     TLA1Akt4PFlowJetTPThinningTool]
    
    # create the derivation kernel
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(
        name, 
        ThinningTools = thinningTools,
        SkimmingTools = [skimmingTool] if skimmingTool is not None else []
    ))


    return acc

# Main setup of the config & format
def TLA1Cfg(flags):
    stream_name = 'StreamDAOD_TLA1'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TLA1TriggerListsHelper = TriggerListsHelper(flags)


    # Common augmentations and TLA1 thinning & skimming
    acc.merge(TLA1KernelCfg(flags, name="TLA1Kernel", StreamName = stream_name, TriggerListsHelper = TLA1TriggerListsHelper))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    TLA1SlimmingHelper = SlimmingHelper("TLA1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    TLA1SlimmingHelper.SmartCollections = [
                        "EventInfo",
                        "Electrons", 
                        "Photons",
                        "PrimaryVertices",
                        "Muons",
                        "AntiKt4EMTopoJets",
                        "AntiKt4EMPFlowJets",
                        "BTagging_AntiKt4EMPFlow",
    ]
    
    # Extra content
    if flags.Input.isMC:
        TLA1SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
            
            "TruthPrimaryVertices.t.x.y.z",
                                                        
            "EventInfo.hardScatterVertexLink.timeStampNSOffset",
        ]
    else:
        TLA1SlimmingHelper.ExtraVariables += [
            "AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
            
            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                                                    
            "EventInfo.hardScatterVertexLink.timeStampNSOffset", 
        ]

    TLA1SlimmingHelper.AllVariables = [
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
    TLA1SlimmingHelper.AppendToDictionary.update(
        {"L1_eEMRoI"+L1Calo_eEM_postfix :  "xAOD::eFexEMRoIContainer",
         "L1_eEMRoI"+L1Calo_eEM_postfix+"Aux" : "xAOD::eFexEMRoIAuxContainer"})
    # add the RoIs to the derivation   
    TLA1SlimmingHelper.AllVariables += ["L1_eEMRoI"+L1Calo_eEM_postfix]

    
    # Truth extra content
    if flags.Input.isMC:
        # from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        from DerivationFrameworkTLA.TLACommonConfig import addTLATruth3ContentToSlimmerTool
        addTLATruth3ContentToSlimmerTool(TLA1SlimmingHelper)
        TLA1SlimmingHelper.AllVariables += [
            'TruthHFWithDecayParticles',
            'TruthHFWithDecayVertices',
            'TruthCharm',
            'TruthPileupParticles',
            'InTimeAntiKt4TruthJets',
            'OutOfTimeAntiKt4TruthJets',
        ]
        TLA1SlimmingHelper.ExtraVariables += [
                                    "Electrons.TruthLink",
                                    "Photons.TruthLink"
                                ]
        # truth jet collections for calibrations and performance studies
        # replicates jet collection configuration in JETM1 (with the exception of AntiKt4TruthDressedWZJets which doesn't exist there)
        TLA1SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]
        TLA1SlimmingHelper.AllVariables += ["AntiKt4TruthJets", "AntiKt4TruthDressedWZJets"]
   
    # Trigger content
    # only save B-jet trigger content and trigger navigation when running on MC
    TLA1SlimmingHelper.IncludeTriggerNavigation = True
    TLA1SlimmingHelper.IncludeJetTriggerContent = True
    TLA1SlimmingHelper.IncludeMuonTriggerContent = False
    TLA1SlimmingHelper.IncludeTrackingTriggerContent = True
    TLA1SlimmingHelper.IncludeEGammaTriggerContent = True
    TLA1SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    TLA1SlimmingHelper.IncludeTauTriggerContent = False
    TLA1SlimmingHelper.IncludeEtMissTriggerContent = False
    TLA1SlimmingHelper.IncludeBJetTriggerContent = True
    TLA1SlimmingHelper.IncludeBPhysTriggerContent = False
    TLA1SlimmingHelper.IncludeMinBiasTriggerContent = False
    TLA1SlimmingHelper.OverrideJetTriggerContentWithTLAContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA1SlimmingHelper, 
                                        OutputContainerPrefix = "TrigMatch_", 
                                        TriggerList = TLA1TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TLA1SlimmingHelper, 
                                        OutputContainerPrefix = "TrigMatch_",
                                        TriggerList = TLA1TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3
    if flags.Trigger.EDMVersion == 3:
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TLA1SlimmingHelper)        

    # Output stream    
    TLA1ItemList = TLA1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TLA1", ItemList=TLA1ItemList, AcceptAlgs=["TLA1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TLA1", AcceptAlgs=["TLA1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

