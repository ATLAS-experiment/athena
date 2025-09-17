# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
def PHYSKernelCfg(flags, name='PHYSKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for PHYS"""
    acc = ComponentAccumulator()

    from TrkConfig.VertexFindingFlags import VertexSortingSetup
    if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting:
        from DerivationFrameworkPhys.GNNVertexConfig import GNNVertexCfg
        acc.merge(GNNVertexCfg(flags))

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        flags, 
        TriggerListsHelper     = kwargs['TriggerListsHelper']
    ))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule
    nametag = name.replace('Kernel', '') #get the name to label the tools below such that other formats can use this KernelCfg
    thinningToolsArgs = {
        'TrackParticleThinningToolName'       : nametag+"TrackParticleThinningTool",
        'MuonTPThinningToolName'              : nametag+"MuonTPThinningTool",
        'TauJetThinningToolName'              : nametag+"TauJetThinningTool",
        'TauJets_MuonRMThinningToolName'      : nametag+"TauJets_MuonRMThinningTool",
        'DiTauThinningToolName'               : nametag+"DiTauThinningTool",
        'DiTauTPThinningToolName'             : nametag+"DiTauTPThinningTool",
        'DiTauLowPtThinningToolName'          : nametag+"DiTauLowPtThinningTool",
        'DiTauLowPtTPThinningToolName'        : nametag+"DiTauLowPtTPThinningTool",
    } 
    # for AOD produced before 24.0.17, the electron removal tau is not available
    if flags.Tau.TauEleRM_isAvailable:
        thinningToolsArgs['TauJets_EleRMThinningToolName'] = nametag+"TauJets_EleRMThinningTool"
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
    return acc


def PHYSCoreCfg(flags, name_tag='PHYS', StreamName='StreamDAOD_PHYS', TriggerListsHelper=None, addExtraVariables=None):
    
    if TriggerListsHelper is None:
        from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
        TriggerListsHelper = TriggerListsHelper(flags)
    
    acc = ComponentAccumulator()

    ## Higgs augmentations - create 4l vertex
    from DerivationFrameworkHiggs.HiggsPhysContent import  HiggsAugmentationAlgsCfg
    acc.merge(HiggsAugmentationAlgsCfg(flags))

    ## CloseByIsolation correction augmentation
    ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
    acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, stream_name = StreamName))

    ## IFF augmentation - Adding Lepton Taggers
    from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg
    acc.merge(DecoratePLITAlgsCfg(flags))

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
    
    PHYSSlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    PHYSSlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "BTagging_AntiKt4EMPFlow",
                                           "AntiKt4EMPFlowJets_FTAG",
                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]
    if flags.Tau.TauEleRM_isAvailable:
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
    PHYSSlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.IsoFixedCone5PtPUsub",
                                              "AntiKt4EMPFlowJets.QGTransformer_ConstScore.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    if addExtraVariables:
        PHYSSlimmingHelper.ExtraVariables += addExtraVariables

    if flags.Tau.TauEleRM_isAvailable:
        PHYSSlimmingHelper.ExtraVariables += ["TauJets_EleRM.dRmax.etOverPtLeadTrk"]

    # IFF extra content
    from LeptonTaggers.LeptonTaggersConfig import GetExtraPLITVariablesForDxAOD
    PHYSSlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()

    # boosted jet taggers
    PHYSSlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.TopTransformer_ConstScore"]

    # Truth extra content
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(PHYSSlimmingHelper)
        PHYSSlimmingHelper.AllVariables += ['TruthLHEParticles', 'TruthHFWithDecayParticles','TruthHFWithDecayVertices','TruthCharm','TruthPileupParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        PHYSSlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink",
                                              "AntiKt4TruthDressedWZJets.IsoFixedCone5Pt.HFHadronOriginID"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        PHYSSlimmingHelper.AllVariables += ['TruthTausWithDecayParticles','TruthTausWithDecayVertices']

    ## Higgs content - 4l vertex and Higgs STXS truth variables
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(flags, PHYSSlimmingHelper)

    ## AFP content - SiT and ToF hits to then be used with AfpAnalysisToolbox reconstruction
    PHYSSlimmingHelper.AllVariables += [ 'AFPSiHitContainer', 'AFPToFHitContainer' ]
    
    # Trigger content
    PHYSSlimmingHelper.IncludeTriggerNavigation = False
    PHYSSlimmingHelper.IncludeJetTriggerContent = False
    PHYSSlimmingHelper.IncludeMuonTriggerContent = False
    PHYSSlimmingHelper.IncludeEGammaTriggerContent = False
    PHYSSlimmingHelper.IncludeTauTriggerContent = False
    PHYSSlimmingHelper.IncludeEtMissTriggerContent = False
    PHYSSlimmingHelper.IncludeBJetTriggerContent = False
    PHYSSlimmingHelper.IncludeBPhysTriggerContent = False
    PHYSSlimmingHelper.IncludeMinBiasTriggerContent = False
    # Compact b-jet trigger matching info
    PHYSSlimmingHelper.IncludeBJetTriggerByYearContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(PHYSSlimmingHelper)

    # L1 trigger objects
    from DerivationFrameworkPhys.TriggerMatchingCommonConfig import getDataYear
    if getDataYear(flags) >= 2024:
        # Run 3 with Phase I jet RoIs.
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper)
    elif getDataYear(flags) >= 2015:
        # Run 2 and early Run 3, legacy L1 RoIs
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddLegacyL1JetRoIsToSlimmingHelper
        AddLegacyL1JetRoIsToSlimmingHelper(SlimmingHelper = PHYSSlimmingHelper)

    # Output stream    
    PHYSItemList = PHYSSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=PHYSItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def PHYSCfg(flags):

    logPHYS.info('****************** STARTING PHYS *****************')

    stream_name = 'StreamDAOD_PHYS'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    PHYSTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(PHYSKernelCfg(
        flags,
        name="PHYSKernel",
        StreamName = stream_name,
        TriggerListsHelper = PHYSTriggerListsHelper
    ))
    # PHYS content
    acc.merge(PHYSCoreCfg(
        flags,
        "PHYS",
        StreamName = stream_name,
        TriggerListsHelper = PHYSTriggerListsHelper
        ))
    
    return acc
