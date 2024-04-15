# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
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
    triggers = TriggerLists.jetTrig(flags)
    
    # Trigger API doesn't currently return all triggers used in Run-3
    # Adding all jets triggers via explicit list for the moment
    triggers += ["HLT_j0_pf_ftf_L1RD0_FILLED",
                 "HLT_j0_perf_pf_subjesgscIS_ftf_L1RD0_FILLED",
                 "HLT_j15_pf_ftf_L1RD0_FILLED",
                 "HLT_j25_pf_ftf_L1RD0_FILLED",
                 "HLT_j25_pf_subjesgscIS_ftf_L1RD0_FILLED",
                 "HLT_j35_pf_ftf_L1RD0_FILLED",
                 "HLT_j45_pf_ftf_preselj20_L1RD0_FILLED",
                 "HLT_j45_pf_ftf_preselj20_L1J15",
                 "HLT_j60_pf_ftf_preselj50_L1J20",
                 "HLT_j85_pf_ftf_preselj50_L1J20",
                 "HLT_j110_pf_ftf_preselj80_L1J30",
                 "HLT_j175_pf_ftf_preselj140_L1J50",
                 "HLT_j260_pf_ftf_preselj200_L1J75",
                 "HLT_j360_pf_ftf_preselj225_L1J100",
                 "HLT_j420_pf_ftf_preselj225_L1J100",
                 "HLT_j0_L1RD0_FILLED",
                 "HLT_j15_L1RD0_FILLED",
                 "HLT_j25_L1RD0_FILLED",
                 "HLT_j35_L1RD0_FILLED",
                 "HLT_j45_preselj20_L1RD0_FILLED",
                 "HLT_j45_preselj20_L1J15",
                 "HLT_j60_preselj50_L1J20",
                 "HLT_j85_preselj50_L1J20",
                 "HLT_j110_preselj80_L1J30",
                 "HLT_j175_preselj140_L1J50",
                 "HLT_j260_preselj200_L1J75",
                 "HLT_j360_preselj225_L1J100",
                 "HLT_j420_L1J100",
                 "HLT_j420_pf_ftf_L1J100",
                 "HLT_j15f_L1RD0_FILLED",
                 "HLT_j25f_L1RD0_FILLED",
                 "HLT_j35f_L1RD0_FILLED",
                 "HLT_j45f_L1J15p31ETA49",
                 "HLT_j60f_L1J20p31ETA49",
                 "HLT_j85f_L1J20p31ETA49",
                 "HLT_j110f_L1J30p31ETA49",
                 "HLT_j175f_L1J50p31ETA49",
                 "HLT_j220f_L1J75p31ETA49",
                 "HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100",
                 "HLT_3j200_pf_ftf_presel3j150_L1J100",
                 "HLT_4j115_pf_ftf_presel4j85_L13J50",
                 "HLT_5j70c_pf_ftf_presel5c50_L14J15",
                 "HLT_5j85_pf_ftf_presel5j50_L14J15",
                 "HLT_6j55c_pf_ftf_presel6j40_L14J15",
                 "HLT_6j70_pf_ftf_presel6j40_L14J15",
                 "HLT_7j45_pf_ftf_presel7j30_L14J15",
                 "HLT_10j40_pf_ftf_presel7j30_L14J15",
                 "HLT_3j200_L1J100",
                 "HLT_4j120_L13J50",
                 "HLT_5j70c_L14J15",
                 "HLT_5j85_L14J15",
                 "HLT_6j55c_L14J15",
                 "HLT_6j70_L14J15",
                 "HLT_7j45_L14J15",
                 "HLT_10j40_L14J15",
                 "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15",
                 "HLT_3j200_pf_ftf_L1J100",
                 "HLT_6j35c_pf_ftf_presel6c25_L14J15",
                 "HLT_j85_a10sd_cssk_pf_jes_ftf_preselj50_L1J20",
                 "HLT_j85_a10t_lcw_jes_L1J20",
                 "HLT_j110_a10sd_cssk_pf_jes_ftf_preselj80_L1J30",
                 "HLT_j110_a10t_lcw_jes_L1J30",
                 "HLT_j175_a10sd_cssk_pf_jes_ftf_preselj140_L1J50",
                 "HLT_j175_a10t_lcw_jes_L1J50",
                 "HLT_j260_a10sd_cssk_pf_jes_ftf_preselj200_L1J75",
                 "HLT_j260_a10t_lcw_jes_L1J75",
                 "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                 "HLT_j360_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                 "HLT_j360_a10t_lcw_jes_L1J100",
                 "HLT_j360_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_j420_35smcINF_a10t_lcw_jes_L1J100",
                 "HLT_j420_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                 "HLT_j420_35smcINF_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                 "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                 "HLT_j460_a10sd_cssk_pf_jes_ftf_preselj225_L1SC111-CJ15",
                 "HLT_j460_a10t_lcw_jes_L1J100",
                 "HLT_j460_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_j460_a10r_L1J100",
                 "HLT_j460_a10r_L1SC111-CJ15",
                 "HLT_j460_a10_lcw_subjes_L1J100",
                 "HLT_j460_a10_lcw_subjes_L1SC111-CJ15",
                 "HLT_j420_a10t_lcw_jes_L1J100",
                 "HLT_j420_a10sd_cssk_pf_jes_ftf_preselj225_L1J100",
                 "HLT_2j330_35smcINF_a10t_lcw_jes_L1J100",
                 "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1J100",
                 "HLT_2j330_35smcINF_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_2j330_35smcINF_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                 "HLT_j360_60smcINF_j360_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_j370_35smcINF_j370_a10t_lcw_jes_L1SC111-CJ15",
                 "HLT_j360_60smcINF_j360_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                 "HLT_j370_35smcINF_j370_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                 "HLT_2j330_a10t_lcw_jes_L1J100",
                 "HLT_2j330_a10sd_cssk_pf_jes_ftf_presel2j225_L1SC111-CJ15",
                 "HLT_j0_HT1000_L1J100",
                 "HLT_j0_HT1000_L1HT190-J15s5pETA21",
                 "HLT_j0_HT1000_pf_ftf_preselj180_L1J100",
                 "HLT_j0_HT1000_pf_ftf_preselj180_L1HT190-J15s5pETA21",
                 "HLT_j0_HT1000_pf_ftf_preselcHT450_L1HT190-J15s5pETA21"]

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
    from JetRecConfig.JetConfigFlags import jetInternalFlags
    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlow, AntiKt4EMPFlowNoPtCut, AntiKt4EMTopoNoPtCut

    #=======================================
    # Schedule additional jet decorations
    #=======================================
    bJVTTool = getModifier(AntiKt4EMPFlow, stdJetModifiers['bJVT'], stdJetModifiers['bJVT'].modspec)
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
        jetList += [AntiKt4UFOCSSKNoPtCut]
    else:
        from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
        jetList += [AntiKt4UFOCSSK]

    jetInternalFlags.isRecoJob = True

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
        triggerNames = ["JetContainer_a4tcemsubjesFS", "JetContainer_a4tcemsubjesISFS", "JetContainer_a10tclcwsubjesFS", "JetContainer_GSCJet"]

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

    JETM1SlimmingHelper.AllVariables = [ "MuonSegments", "EventInfo",
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
        JETM1SlimmingHelper.ExtraVariables += ["TruthVertices.barcode.z"]

    JETM1SlimmingHelper.AppendToDictionary.update({'Kt4UFOCSSKEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux':'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux':'xAOD::EventShapeAuxInfo'})

    # Trigger content
    JETM1SlimmingHelper.IncludeTriggerNavigation = False
    JETM1SlimmingHelper.IncludeJetTriggerContent = True
    JETM1SlimmingHelper.IncludeMuonTriggerContent = False
    JETM1SlimmingHelper.IncludeEGammaTriggerContent = False
    JETM1SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    JETM1SlimmingHelper.IncludeTauTriggerContent = False
    JETM1SlimmingHelper.IncludeEtMissTriggerContent = False
    JETM1SlimmingHelper.IncludeBJetTriggerContent = False
    JETM1SlimmingHelper.IncludeBPhysTriggerContent = False
    JETM1SlimmingHelper.IncludeMinBiasTriggerContent = False

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

