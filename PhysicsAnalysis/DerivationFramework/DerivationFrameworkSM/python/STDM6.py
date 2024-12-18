# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# STDM6.py - derivation dedicated for the special runs at low mu (AFP analysis)

# Based on PHYS derivation:
# * no skimming
# * slimming by default as in PHYS derivation
# * added content wrt PHYS:
#   - forward protons
#   - InDet tracks/particles
#   - ["TruthEvents", "TruthParticles", "TruthVertices", "AntiKt4TruthJets","AntiKt4TruthWZJets"]
#   - Calo clusters
# * Nov 2024 update: added JET-M1 containers


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
logSTDM6 = logging.getLogger('STDM6')



# Main algorithm config
def STDM6KernelCfg(flags, name='STDM6Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for STDM6"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        flags, 
        TriggerListsHelper     = kwargs['TriggerListsHelper'], 
        TauJets_EleRM_in_input = kwargs['TauJets_EleRM_in_input']
    ))

    from DerivationFrameworkInDet.InDetToolsConfig import InDetTrackSelectionToolWrapperCfg
    DFCommonTrackSelection = acc.getPrimaryAndMerge(InDetTrackSelectionToolWrapperCfg(
        flags,
        name           = "DFJETM1CommonTrackSelectionLoose",
        CutLevel       = "Loose",
        DecorationName = "DFJETM1Loose"))

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("JETM1CommonKernel", AugmentationTools = [DFCommonTrackSelection]))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule 
    thinningToolsArgs = {
        'TrackParticleThinningToolName'       : "STDM6TrackParticleThinningTool",
        'MuonTPThinningToolName'              : "STDM6MuonTPThinningTool",
        'TauJetThinningToolName'              : "STDM6TauJetThinningTool",
        'TauJets_MuonRMThinningToolName'      : "STDM6TauJets_MuonRMThinningTool",
        'DiTauTPThinningToolName'             : "STDM6DiTauTPThinningTool",
        'DiTauLowPtThinningToolName'          : "STDM6DiTauLowPtThinningTool",
        'DiTauLowPtTPThinningToolName'        : "STDM6DiTauLowPtTPThinningTool",
    } 
    # for AOD produced before 24.0.17, the electron removal tau is not available
    if kwargs.get('TauJets_EleRM_in_input', False):
        thinningToolsArgs['TauJets_EleRMThinningToolName'] = "STDM6TauJets_EleRMThinningTool"
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(flags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))       
    
    # not using the thinning from the JET-M1 derivation!
    
    # From JET-M1 - extra jet content:
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


def STDM6CoreCfg(flags, name_tag='STDM6', StreamName='StreamDAOD_STDM6', TriggerListsHelper=None, TauJets_EleRM_in_input=None):
    
    if TriggerListsHelper is None:
        from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
        TriggerListsHelper = TriggerListsHelper(flags)
    
    if TauJets_EleRM_in_input is None:
        # for AOD produced before 24.0.17, the electron removal tau is not available
        TauJets_EleRM_in_input = (flags.Input.TypedCollections.count('xAOD::TauJetContainer#TauJets_EleRM') > 0)
    
    acc = ComponentAccumulator()

    ## Higgs augmentations - create 4l vertex
    from DerivationFrameworkHiggs.HiggsPhysContent import  HiggsAugmentationAlgsCfg
    acc.merge(HiggsAugmentationAlgsCfg(flags))
    
    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
    acc.merge(IsoCloseByAlgsCfg(flags, suff = "_"+name_tag, isPhysLite = False, stream_name = StreamName))

    #===================================================
    # HEAVY FLAVOR CLASSIFICATION FOR ttbar+jets EVENTS
    #===================================================
    from DerivationFrameworkMCTruth.HFClassificationCommonConfig import HFClassificationCommonCfg
    acc.merge(HFClassificationCommonCfg(flags))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    STDM6SlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    STDM6SlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "AntiKt10UFOCSSKJets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "BTagging_AntiKtVR30Rmax4Rmin02Track",
                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]
    if TauJets_EleRM_in_input:
        STDM6SlimmingHelper.SmartCollections.append("TauJets_EleRM")

    # STDM6 needs additionally full info on: 
    # - AFP
    # - Calo clusters
    STDM6SlimmingHelper.AllVariables += [ "AFPSiHitContainer",
                                         "AFPToFHitContainer",
                                         "AFPSiHitsClusterContainer",
                                         "AFPTrackContainer",
                                         "AFPToFTrackContainer",    
                                         "AFPProtonContainer",
                                         "AFPVertexContainer",
                                         "CaloCalTopoClusters",
    ]

    # additional contenct from JET-M1
    # Maciej LewickiL: I'd rather keep it the way it is because it will be easier to keep up with any future changes in JET-M1
    STDM6SlimmingHelper.AllVariables += ["MuonSegments", "EventInfo",
                                         "Kt4EMTopoOriginEventShape","Kt4EMPFlowEventShape","Kt4EMPFlowPUSBEventShape","Kt4EMPFlowNeutEventShape","Kt4UFOCSSKEventShape","Kt4UFOCSSKNeutEventShape",
                                         "AntiKt4EMPFlowJets"]

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    STDM6SlimmingHelper.StaticContent = StaticContent

    # Extra content
    STDM6SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]
    if TauJets_EleRM_in_input:
        STDM6SlimmingHelper.ExtraVariables += ["TauJets_EleRM.dRmax.etOverPtLeadTrk"]

    # additional content from JET-M1
    # Maciej Lewicki: I'd rather keep it the way it is because it will be easier to keep up with any future changes in JET-M1
    STDM6SlimmingHelper.ExtraVariables  += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                           "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1",
                                           "AntiKt4EMPFlowJets.passOnlyBJVT.DFCommonJets_bJvt.isJvtHS.isJvtPU",
                                           "InDetTrackParticles.truthMatchProbability",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.zg.rg.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack",
                                           "AntiKt10UFOCSSKJets.NumTrkPt1000.TrackWidthPt1000.GhostMuonSegmentCount.EnergyPerSampling.GhostTrack"]


    # FTAG Xbb extra content
    extraList = []
    for tagger in ["GN2Xv00", "GN2XWithMassv00", "GN2Xv01"]:
        for score in ["phbb", "phcc", "ptop", "pqcd"]:
            extraList.append(f"{tagger}_{score}")
    STDM6SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets." + ".".join(extraList)]
 
    # Truth extra content
    # ---> extetnding deafult PHYS conttents with:
    #      - AntiKt4EMTTopoJets 
    #      - AntiKt4EMPFlowJets 
    #      - TruthPrimaryVertices 
    #      - InDetTrackParticles
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(STDM6SlimmingHelper)
        
        # coming from the JET-M1 derivation
        STDM6SlimmingHelper.AppendToDictionary.update({'TruthParticles': 'xAOD::TruthParticleContainer',
                                                       'TruthParticlesAux': 'xAOD::TruthParticleAuxContainer'})

        STDM6SlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        STDM6SlimmingHelper.AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices", "AntiKt4TruthJets","AntiKt4TruthWZJets"]
        STDM6SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink",
                                              "AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "PrimaryVertices.x.y.z.covariance.trackWeights.vertexType.sumPt2",
                                              ]

        # Maciej Lewicki: coming from the JET-M1 derivation
        STDM6SlimmingHelper.SmartCollections += ["AntiKt4TruthWZJets"]
        STDM6SlimmingHelper.AllVariables += ["TruthTopQuarkWithDecayParticles","TruthTopQuarkWithDecayVertices",
                                             "AntiKt4TruthJets", "InTimeAntiKt4TruthJets", "OutOfTimeAntiKt4TruthJets", "TruthParticles"]
        STDM6SlimmingHelper.ExtraVariables += ["TruthVertices.barcode.z"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        STDM6SlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']

    STDM6SlimmingHelper.AppendToDictionary.update({'Kt4UFOCSSKEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKEventShapeAux':'xAOD::EventShapeAuxInfo',
                                                   'Kt4UFOCSSKNeutEventShape':'xAOD::EventShape',
                                                   'Kt4UFOCSSKNeutEventShapeAux':'xAOD::EventShapeAuxInfo'})

    ## Higgs content - 4l vertex and Higgs STXS truth variables
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(flags, STDM6SlimmingHelper)
   
    # Trigger content
    STDM6SlimmingHelper.IncludeTriggerNavigation = False
    # modification from JET-M1 derivation:
    STDM6SlimmingHelper.IncludeJetTriggerContent = True
    STDM6SlimmingHelper.IncludeMuonTriggerContent = False
    STDM6SlimmingHelper.IncludeEGammaTriggerContent = False
    STDM6SlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    STDM6SlimmingHelper.IncludeTauTriggerContent = False
    STDM6SlimmingHelper.IncludeEtMissTriggerContent = False
    STDM6SlimmingHelper.IncludeBJetTriggerContent = False
    STDM6SlimmingHelper.IncludeBPhysTriggerContent = False
    STDM6SlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM6SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = STDM6SlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
        triggerNames = ["a4tcemsubjesFS", "a4tcemsubjesISFS", "a10tclcwsubjesFS", "a10tclcwsubFS", "a10ttclcwjesFS", "GSCJet"]
        for trigger in triggerNames:
            STDM6SlimmingHelper.FinalItemList.append('xAOD::AuxContainerBase!#HLT_xAOD__JetContainer_'+trigger+'Aux.pt.eta.phi.m')

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(STDM6SlimmingHelper)

    jetOutputList = ["AntiKt4PV0TrackJets", "AntiKt4UFOCSSKJets"]
    if flags.Input.isMC:
        jetOutputList = ["AntiKt4PV0TrackJets","AntiKt4UFOCSSKNoPtCutJets","AntiKt4EMPFlowNoPtCutJets","AntiKt4EMTopoNoPtCutJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(STDM6SlimmingHelper, jetOutputList, STDM6SlimmingHelper.SmartCollections)


    # Output stream    
    STDM6ItemList = STDM6SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=STDM6ItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def STDM6Cfg(flags):
    
    logSTDM6.info('****************** STARTING STDM6 *****************')

    stream_name = 'StreamDAOD_STDM6'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    STDM6TriggerListsHelper = TriggerListsHelper(flags)

    # for AOD produced before 24.0.17, the electron removal tau is not available
    TauJets_EleRM_in_input = (flags.Input.TypedCollections.count('xAOD::TauJetContainer#TauJets_EleRM') > 0)
    if TauJets_EleRM_in_input:
        logSTDM6.info("TauJets_EleRM is in the input AOD. Relevant containers will be scheduled")
    else:
        logSTDM6.info("TauJets_EleRM is Not in the input AOD. No relevant containers will be written")

    # Common augmentations
    acc.merge(STDM6KernelCfg(
        flags, 
        name="STDM6Kernel", 
        StreamName = stream_name, 
        TriggerListsHelper = STDM6TriggerListsHelper, 
        TauJets_EleRM_in_input=TauJets_EleRM_in_input
    ))
    # STDM6 content
    acc.merge(STDM6CoreCfg(
        flags, 
        "STDM6",
        StreamName = stream_name,
        TriggerListsHelper = STDM6TriggerListsHelper, 
        TauJets_EleRM_in_input=TauJets_EleRM_in_input
        ))

    return acc
