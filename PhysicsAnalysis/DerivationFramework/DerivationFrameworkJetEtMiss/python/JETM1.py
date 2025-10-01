# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_JETM1.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

# Main algorithm config
def JETM1SkimmingToolCfg(flags):
    """Configure the skimming tool"""
    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss import TriggerLists
    # Use this function until trigAPI works for Run 3 jet triggers.
    triggers = TriggerLists.get_jetTrig(flags)

    if not flags.Input.isMC:

        JETM1TrigSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool( name                   = "JETM1TrigSkimmingTool1",
                                                                                     TriggerListOR          = triggers )

        acc.addPublicTool(JETM1TrigSkimmingTool)

        expression = 'HLT_xe120_pufit_L1XE50'
        JETM1OfflineSkimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(name       = "JETM1OfflineSkimmingTool1",
                                                                                          expression = expression)

        acc.addPublicTool(JETM1OfflineSkimmingTool)
        
        # OR of the above two selections
        acc.addPublicTool(CompFactory.DerivationFramework.FilterCombinationOR(name="JETM1ORTool", 
                                                                              FilterList=[JETM1TrigSkimmingTool,JETM1OfflineSkimmingTool] ), 
                          primary = True)

    return(acc)


# Main algorithm config
def JETM1KernelCfg(flags, name='JETM1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for JETM1"""
    acc = ComponentAccumulator()

    # Skimming
    if not flags.Input.isMC:
        skimmingTool = acc.getPrimaryAndMerge(JETM1SkimmingToolCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFJETM1CommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFJETM1Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("JETM1CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools...
    from DerivationFrameworkInDet.InDetToolsConfig import MuonTrackParticleThinningCfg, EgammaTrackParticleThinningCfg, JetTrackParticleThinningCfg

    # Include inner detector tracks associated with muons
    JETM1MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(
        flags,
        name                    = "JETM1MuonTPThinningTool",
        StreamName              = kwargs['StreamName'],
        MuonKey                 = "Muons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))
    
    # Include inner detector tracks associated with electonrs
    JETM1ElectronTPThinningTool = acc.getPrimaryAndMerge(EgammaTrackParticleThinningCfg(
        flags,
        name                    = "JETM1ElectronTPThinningTool",
        StreamName              = kwargs['StreamName'],
        SGKey                   = "Electrons",
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    JETM1_thinning_expression = "InDetTrackParticles.DFJETM1Loose && ( abs(InDetTrackParticles.d0) < 5.0*mm ) && ( abs(DFCommonInDetTrackZ0AtPV*sin(InDetTrackParticles.theta)) < 5.0*mm )"

    JETM1Akt4JetTPThinningTool  = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                    = "JETM1Akt4JetTPThinningTool",
        StreamName              = kwargs['StreamName'],
        JetKey                  = "AntiKt4EMTopoJets",
        SelectionString         = "AntiKt4EMTopoJets.pt > 18*GeV",
        TrackSelectionString    = JETM1_thinning_expression,
        InDetTrackParticlesKey  = "InDetTrackParticles"))

    JETM1Akt4PFlowJetTPThinningTool = acc.getPrimaryAndMerge(JetTrackParticleThinningCfg(
        flags,
        name                         = "JETM1Akt4PFlowJetTPThinningTool",
        StreamName                   = kwargs['StreamName'],
        JetKey                       = "AntiKt4EMPFlowJets",
        SelectionString              = "AntiKt4EMPFlowJets.pt > 18*GeV",
        TrackSelectionString         = JETM1_thinning_expression,
        InDetTrackParticlesKey       = "InDetTrackParticles"))

    # Finally the kernel itself
    thinningTools = [JETM1MuonTPThinningTool,
                     JETM1ElectronTPThinningTool,
                     JETM1Akt4JetTPThinningTool,
                     JETM1Akt4PFlowJetTPThinningTool]
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, 
                                      ThinningTools = thinningTools,
                                      SkimmingTools = [skimmingTool] if not flags.Input.isMC else []))       

    
    # Extra jet content:
    acc.merge(JETM1ExtraContentCfg(flags))

    return acc


def JETM1ExtraContentCfg(flags):

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg, getModifier
    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlow, AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoNoPtCut

    #=======================================
    # Schedule additional jet decorations
    #=======================================
    bJVTTool = getModifier(AntiKt4EMPFlow, stdJetModifiers['bJVT'], stdJetModifiers['bJVT'].modspec, flags=flags)
    acc.addEventAlgo(CompFactory.JetDecorationAlg(name='bJVTAlg',
                                                  JetContainer='AntiKt4EMPFlowJets', 
                                                  Decorators=[bJVTTool]))

    #======================================= 
    # R = 0.4 track-jets (needed for Rtrk) 
    #=======================================
    jetList = [AntiKt4PV0Track]

    #=======================================
    # SCHEDULE SMALL-R JETS WITH NO PT CUT
    #=======================================
    if flags.Input.isMC:
        jetList += [AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoNoPtCut]

    #=======================================
    # CSSK R = 0.4 UFO jets
    #=======================================
    if flags.Input.isMC:
        from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSKNoPtCut
        AntiKt4UFOCSSKNoPtCut_JETM1 = AntiKt4UFOCSSKNoPtCut.clone(
            modifiers = AntiKt4UFOCSSKNoPtCut.modifiers+("NNJVT",)
        )
        jetList += [AntiKt4UFOCSSKNoPtCut_JETM1]
    else:
        from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
        AntiKt4UFOCSSK_JETM1 = AntiKt4UFOCSSK.clone(
            modifiers = AntiKt4UFOCSSK.modifiers+("NNJVT",)
        )
        jetList += [AntiKt4UFOCSSK_JETM1]


    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    #=======================================
    # UFO CSSK event shape 
    #=======================================

    from JetRecConfig.JetRecConfig import getConstitPJGAlg
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    from JetRecConfig.JetInputConfig import buildEventShapeAlg

    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix=None))
    acc.addEventAlgo(getConstitPJGAlg(cst.UFOCSSK, suffix='Neut'))
    acc.addEventAlgo(buildEventShapeAlg(cst.UFOCSSK,'', suffix='Neut'))

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags, generations=4,rejectHadronChildren=True))

    #=======================================
    # Add Run-2 jet trigger collections
    # Only needed for Run-2 due to different aux container type (JetTrigAuxContainer) which required special wrapper for conversion to AuxContainerBase
    # In Run-3, the aux. container type is directly JetAuxContainer (no conversion needed)
    #=======================================

    if flags.Trigger.EDMVersion == 2:
        triggerNames = ["JetContainer_a4tcemsubjesFS", "JetContainer_a4tcemsubjesISFS", "JetContainer_GSCJet",
                        "JetContainer_a10tclcwsubjesFS", "JetContainer_a10tclcwsubFS", "JetContainer_a10ttclcwjesFS"]

        for trigger in triggerNames:
            wrapperName = trigger+'AuxWrapper'
            auxContainerName = 'HLT_xAOD__'+trigger+'Aux'

            acc.addEventAlgo(CompFactory.xAODMaker.AuxStoreWrapper( wrapperName, SGKeys = [ auxContainerName+"." ] ))

    return acc

def JETM1Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    JETM1TriggerListsHelper = TriggerListsHelper(flags)

    # Skimming, thinning, augmentation, extra content
    acc.merge(JETM1KernelCfg(flags, name="JETM1Kernel", StreamName = 'StreamDAOD_JETM1', TriggerListsHelper = JETM1TriggerListsHelper))

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    JETM1SlimmingHelper = SlimmingHelper("JETM1SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    JETM1SlimmingHelper.SmartCollections = ["Electrons", "Photons", "Muons", "PrimaryVertices",
                                            "InDetTrackParticles",
                                            "AntiKt4EMTopoJets",
                                            "AntiKt10UFOCSSKJets",
                                            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                            "BTagging_AntiKt4EMPFlow"]

    JETM1SlimmingHelper.ExtraVariables  = ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                           "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                           "AntiKt4EMPFlowJets.passOnlyBJVT.DFCommonJets_bJvt.isJvtHS.isJvtPU",
                                           "InDetTrackParticles.truthMatchProbability",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.zg.rg.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack",
                                           "AntiKt10UFOCSSKJets.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack"]

    JETM1SlimmingHelper.AllVariables = [ "MuonSegments", "UnAssocMuonSegments", "EventInfo",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape",
                                         "AntiKt4EMPFlowJets"]
    
    # Truth containers
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(JETM1SlimmingHelper)

        JETM1SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        JETM1SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]
        JETM1SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices",
                                             "AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles"]
        JETM1SlimmingHelper.ExtraVariables += ["TruthVertices.uid.z"]

    JETM1SlimmingHelper.AppendToDictionary.update({'Kt4UFOCSSKEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux':'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux':'xAOD::EventShapeAuxInfo'})

    # Trigger content
    JETM1SlimmingHelper.IncludeTriggerNavigation = False
    JETM1SlimmingHelper.IncludeJetTriggerContent = True
    JETM1SlimmingHelper.IncludeMuonTriggerContent = False
    JETM1SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM1SlimmingHelper.IncludeTauTriggerContent = False
    JETM1SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM1SlimmingHelper.IncludeBJetTriggerContent = False
    JETM1SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM1SlimmingHelper.IncludeMinBiasTriggerContent = False

    if flags.Trigger.EDMVersion == 2:
        triggerNames = ["a4tcemsubjesFS", "a4tcemsubjesISFS", "a10tclcwsubjesFS", "a10tclcwsubFS", "a10ttclcwjesFS", "GSCJet"]
        for trigger in triggerNames:
            JETM1SlimmingHelper.FinalItemList.append('xAOD::AuxContainerBase!#HLT_xAOD__JetContainer_'+trigger+'Aux.pt.eta.phi.m')
    
    from DerivationFrameworkPhys.TriggerMatchingCommonConfig import getDataYear
    if getDataYear(flags) >= 2024:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = JETM1SlimmingHelper)

    jetOutputList = ["AntiKt4PV0TrackJets", "AntiKt4UFOCSSKJets"]
    if flags.Input.isMC:
        jetOutputList = ["AntiKt4PV0TrackJets","AntiKt4UFOCSSKNoPtCutJets","AntiKt4EMPFlowNoPtCutJets","AntiKt4EMTopoNoPtCutJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(JETM1SlimmingHelper, jetOutputList, JETM1SlimmingHelper.SmartCollections)

    # Output stream    
    JETM1ItemList = JETM1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_JETM1", ItemList=JETM1ItemList, AcceptAlgs=["JETM1Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_JETM1", AcceptAlgs=["JETM1Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

