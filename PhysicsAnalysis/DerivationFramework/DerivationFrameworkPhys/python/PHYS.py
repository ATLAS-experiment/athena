# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DAOD_PHYS.py
# This defines DAOD_PHYS, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag PHYS in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
logPHYS = logging.getLogger('PHYS')

# Main algorithm config
def PHYSKernelCfg(ConfigFlags, name='PHYSKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYS"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        ConfigFlags, 
        TriggerListsHelper     = kwargs['TriggerListsHelper'], 
        TauJets_EleRM_in_input = kwargs['TauJets_EleRM_in_input']
    ))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule 
    thinningToolsArgs = {
        'TrackParticleThinningToolName'       : "PHYSTrackParticleThinningTool",
        'MuonTPThinningToolName'              : "PHYSMuonTPThinningTool",
        'TauJetThinningToolName'              : "PHYSTauJetThinningTool",
        'TauJets_MuonRMThinningToolName'      : "PHYSTauJets_MuonRMThinningTool",
        'DiTauTPThinningToolName'             : "PHYSDiTauTPThinningTool",
        'DiTauLowPtThinningToolName'          : "PHYSDiTauLowPtThinningTool",
        'DiTauLowPtTPThinningToolName'        : "PHYSDiTauLowPtTPThinningTool",
    } 
    # for AOD produced before 24.0.17, the electron removal tau is not available
    if kwargs.get('TauJets_EleRM_in_input', False):
        thinningToolsArgs['TauJets_EleRMThinningToolName'] = "PHYSTauJets_EleRMThinningTool"
    # Configure the thinning tools
    from DerivationFrameworkPhys.PhysCommonThinningConfig import PhysCommonThinningCfg
    acc.merge(PhysCommonThinningCfg(ConfigFlags, StreamName = kwargs['StreamName'], **thinningToolsArgs))
    # Get them from the CA so they can be added to the kernel
    thinningTools = []
    for key in thinningToolsArgs:
        thinningTools.append(acc.getPublicTool(thinningToolsArgs[key]))

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, ThinningTools = thinningTools))       
    return acc


def PHYSCfg(ConfigFlags):
    
    logPHYS.info('****************** STARTING PHYS *****************')

    stream_name = 'StreamDAOD_PHYS'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSTriggerListsHelper = TriggerListsHelper(ConfigFlags)

    # for AOD produced before 24.0.17, the electron removal tau is not available
    TauJets_EleRM_in_input = (ConfigFlags.Input.TypedCollections.count('xAOD::TauJetContainer#TauJets_EleRM') > 0)
    if TauJets_EleRM_in_input:
        logPHYS.info("TauJets_EleRM is in the input AOD. Relevant containers will be scheduled")
    else:
        logPHYS.info("TauJets_EleRM is Not in the input AOD. No relevant containers will be written")

    # Common augmentations
    acc.merge(PHYSKernelCfg(
        ConfigFlags, 
        name="PHYSKernel", 
        StreamName = stream_name, 
        TriggerListsHelper = PHYSTriggerListsHelper, 
        TauJets_EleRM_in_input=TauJets_EleRM_in_input
    ))
    
    ## Higgs augmentations - create 4l vertex
    from DerivationFrameworkHiggs.HiggsPhysContent import  HiggsAugmentationAlgsCfg
    acc.merge(HiggsAugmentationAlgsCfg(ConfigFlags))
    
    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
    acc.merge(IsoCloseByAlgsCfg(ConfigFlags, suff = "_PHYS", isPhysLite = False, stream_name = stream_name))


    #===================================================
    # HEAVY FLAVOR CLASSIFICATION FOR ttbar+jets EVENTS
    #===================================================
    from DerivationFrameworkMCTruth.HFClassificationCommonConfig import HFClassificationCommonCfg
    acc.merge(HFClassificationCommonCfg(ConfigFlags))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    PHYSSlimmingHelper = SlimmingHelper("PHYSSlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    PHYSSlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
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
        PHYSSlimmingHelper.SmartCollections.append("TauJets_EleRM")
    
    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]   

    PHYSSlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    PHYSSlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]
    if TauJets_EleRM_in_input:
        PHYSSlimmingHelper.ExtraVariables += ["TauJets_EleRM.dRmax.etOverPtLeadTrk"]

    # FTAG Xbb extra content
    extraList = []
    for tagger in ["GN2Xv00", "GN2XWithMassv00", "GN2Xv01"]:
        for score in ["phbb", "phcc", "ptop", "pqcd"]:
            extraList.append(f"{tagger}_{score}")
    PHYSSlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets." + ".".join(extraList)]
 
    # Truth extra content
    if ConfigFlags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSSlimmingHelper)
        PHYSSlimmingHelper.AllVariables += ['TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        PHYSSlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(ConfigFlags))
        PHYSSlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']

    ## Higgs content - 4l vertex and Higgs STXS truth variables
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(ConfigFlags, PHYSSlimmingHelper)
   
    # Trigger content
    PHYSSlimmingHelper.IncludeTriggerNavigation = False
    PHYSSlimmingHelper.IncludeJetTriggerContent = False
    PHYSSlimmingHelper.IncludeMuonTriggerContent = False
    PHYSSlimmingHelper.IncludeEGammaTriggerContent = False
    PHYSSlimmingHelper.IncludeJetTauEtMissTriggerContent = False
    PHYSSlimmingHelper.IncludeTauTriggerContent = False
    PHYSSlimmingHelper.IncludeEtMissTriggerContent = False
    PHYSSlimmingHelper.IncludeBJetTriggerContent = False
    PHYSSlimmingHelper.IncludeBPhysTriggerContent = False
    PHYSSlimmingHelper.IncludeMinBiasTriggerContent = False

    # Trigger matching
    # Run 2
    if ConfigFlags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_", 
                                         TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                         OutputContainerPrefix = "TrigMatch_",
                                         TriggerList = PHYSTriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if ConfigFlags.Trigger.EDMVersion == 3 or (ConfigFlags.Trigger.EDMVersion == 2 and ConfigFlags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSSlimmingHelper)
        ##################################################### 
        ## NOTE: This block is temporary, during validation of the doEDMVersionConversion flag.
        ## This adds a LOT of containers to the output! In order to help validate the conversion.
        ## It should be removed once doEDMVersionConversion goes into production use.
        if ConfigFlags.Trigger.doEDMVersionConversion:   
            from DerivationFrameworkTrigger.TrigSlimmingHelper import addTrigEDMSetToOutput
            logPHYS.warn('doEDMVersionConversion is still in validation, WRITING FULL TRIGGER EDM TO THE DAOD!')
            addTrigEDMSetToOutput(ConfigFlags, PHYSSlimmingHelper, "AODFULL")
            PHYSSlimmingHelper.AppendToDictionary.update({'HLTNav_R2ToR3Summary':'xAOD::TrigCompositeContainer','HLTNav_R2ToR3SummaryAux':'xAOD::TrigCompositeAuxContainer'})
            PHYSSlimmingHelper.AllVariables += ['HLTNav_R2ToR3Summary']
        ##
        #####################################################

    # Output stream    
    PHYSItemList = PHYSSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_PHYS", ItemList=PHYSItemList, AcceptAlgs=["PHYSKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_PHYS", AcceptAlgs=["PHYSKernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

