# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_TOPQ7.py
# This defines DAOD_TOPQ7, an skimmed and thinned DAOD format containing all variables 
# from PHYS in boosted ttbar events with the additional 3 leading parton jets saved.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag TOPQ7 in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
logTOPQ7 = logging.getLogger('TOPQ7')

def BoostedTTbarSkimmingToolCfg(flags):
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.BoostedTTbarSkimmingToolAlg(
            name="TOPQ7BoostedTTbarSkimmingTool",
            ttbarCut = 700000.0
        ),
        primary=True
    )
    return acc


# Main algorithm config
def TOPQ7KernelCfg(flags, name='TOPQ7Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for TOPQ7"""
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


    #Skimming tool and augmentation - boosted ttbar semilep selection and building parton jets
    skimmingTools = []
    augmentationTools = []

    if flags.Input.isMC:
        # Skimming
        skimmingTool = acc.getPrimaryAndMerge(BoostedTTbarSkimmingToolCfg(flags))
        skimmingTools.append(skimmingTool)

        # Parton jet augmentation
        partonTool = CompFactory.DerivationFramework.PartonJetAugmentationTool("TOPQ7PartonJetTool")
        acc.addPublicTool(partonTool)
        augmentationTools.append(partonTool)
    

    # The kernel algorithm itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(
        DerivationKernel(
            name, 
            ThinningTools = thinningTools,
            SkimmingTools=skimmingTools,
            AugmentationTools=augmentationTools
            ))       
    return acc


def TOPQ7CoreCfg(flags, name_tag='TOPQ7', StreamName='StreamDAOD_TOPQ7', TriggerListsHelper=None, addExtraVariables=None):
    
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
    
    TOPQ7SlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TOPQ7SlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
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
        TOPQ7SlimmingHelper.SmartCollections.append("TauJets_EleRM")

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData] 
    StaticContent += ["xAOD::VertexContainer#NVSI_SecVrt_Tight"]
    StaticContent += ["xAOD::VertexAuxContainer#NVSI_SecVrt_TightAux."+excludedVertexAuxData]   

    TOPQ7SlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    TOPQ7SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.IsoFixedCone5PtPUsub",
                                              "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification.HF_ClassificationC5J20.HF_ClassificationC5J25.HF_ClassificationC15J20.HF_ClassificationC15J25",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey",
                                              "HLT_AnomDet_ComboHypo.adScore"]

    if addExtraVariables:
        TOPQ7SlimmingHelper.ExtraVariables += addExtraVariables

    if flags.Tau.TauEleRM_isAvailable:
        TOPQ7SlimmingHelper.ExtraVariables += ["TauJets_EleRM.dRmax.etOverPtLeadTrk"]

    # IFF extra content
    from LeptonTaggers.LeptonTaggersConfig import GetExtraPLITVariablesForDxAOD
    TOPQ7SlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()

    # boosted jet taggers
    TOPQ7SlimmingHelper.ExtraVariables += ["AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10TruthLabel_R22v1_TruthJetMass",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10TruthLabel_R22v1_TruthJetPt",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10TruthLabel_R22v1_TruthGroomedJetMass",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10TruthLabel_R22v1_TruthGroomedJetPt",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10WZTruthLabel_R22v1_TruthJetMass",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10WZTruthLabel_R22v1_TruthJetPt",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10WZTruthLabel_R22v1_TruthGroomedJetMass",
                                          "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets.R10WZTruthLabel_R22v1_TruthGroomedJetPt"]

    # Truth extra content
    if flags.Input.isMC:

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(TOPQ7SlimmingHelper)
        TOPQ7SlimmingHelper.AllVariables += ['TruthLHEParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
        TOPQ7SlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
                                              "Muons.TruthLink",
                                              "Photons.TruthLink",
                                              "AntiKt4TruthDressedWZJets.IsoFixedCone5Pt.HFHadronOriginID",
                                              "TruthHFWithDecayParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.parentLinks.childLinks.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status",
                                              "TruthHFWithDecayVertices.incomingParticleLinks.outgoingParticleLinks.uid.status.x.y.z.t",
                                              "TruthCharm.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.parentLinks.childLinks.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status.barcode.polarizationPhi.polarizationTheta",
                                              "TruthPileupParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status.barcode.PVz.pileupEventNumber.parentHadronID"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        TOPQ7SlimmingHelper.ExtraVariables += ["TruthTausWithDecayParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status",
                                              "TruthTausWithDecayVertices.incomingParticleLinks.outgoingParticleLinks.uid.status.x.y.z.t"]

        #parton jets
        TOPQ7SlimmingHelper.AppendToDictionary.update({
            "PartonJets": "xAOD::JetContainer",
            "PartonJetsAux": "xAOD::JetAuxContainer"
        })

        TOPQ7SlimmingHelper.AllVariables += ["PartonJets"]


    ## Higgs content - 4l vertex and Higgs STXS truth variables
    from DerivationFrameworkHiggs.HiggsPhysContent import  setupHiggsSlimmingVariables
    setupHiggsSlimmingVariables(flags, TOPQ7SlimmingHelper)

    ## AFP content - SiT and ToF hits to then be used with AfpAnalysisToolbox reconstruction
    TOPQ7SlimmingHelper.AllVariables += [ 'AFPSiHitContainer', 'AFPToFHitContainer' ]

    ## Hadronic Recoil content
    TOPQ7SlimmingHelper.AppendToDictionary.update({'MET_Core_AntiKt4EMPFlowHR':'xAOD::MissingETContainer', 'MET_Core_AntiKt4EMPFlowHRAux':'xAOD::MissingETAuxContainer',
                                                   'METAssoc_AntiKt4EMPFlowHR':'xAOD::MissingETAssociationMap', 'METAssoc_AntiKt4EMPFlowHRAux':'xAOD::MissingETAuxAssociationMap'})

    TOPQ7SlimmingHelper.AllVariables += ['METAssoc_AntiKt4EMPFlowHR']

    TOPQ7SlimmingHelper.ExtraVariables += ['Muons.UEcorr_Pt','Electrons.UEcorr_Pt','MET_Core_AntiKt4EMPFlowHR.name.mpx.mpy.sumet.source']

    # Trigger content
    TOPQ7SlimmingHelper.IncludeTriggerNavigation = False
    TOPQ7SlimmingHelper.IncludeJetTriggerContent = False
    TOPQ7SlimmingHelper.IncludeMuonTriggerContent = False
    TOPQ7SlimmingHelper.IncludeEGammaTriggerContent = False
    TOPQ7SlimmingHelper.IncludeTauTriggerContent = False
    TOPQ7SlimmingHelper.IncludeEtMissTriggerContent = False
    TOPQ7SlimmingHelper.IncludeBJetTriggerContent = False
    TOPQ7SlimmingHelper.IncludeBPhysTriggerContent = False
    TOPQ7SlimmingHelper.IncludeMinBiasTriggerContent = False
    # Compact b-jet trigger matching info
    TOPQ7SlimmingHelper.IncludeBJetTriggerByYearContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TOPQ7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_", 
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = TOPQ7SlimmingHelper, 
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = TriggerListsHelper.Run2TriggerNamesNoTau)
    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(TOPQ7SlimmingHelper)

    # L1 trigger objects
    from Campaigns.Utils import getDataYear
    if getDataYear(flags) >= 2024:
        # Run 3 with Phase I jet RoIs.
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = TOPQ7SlimmingHelper)
    elif getDataYear(flags) >= 2015:
        # Run 2 and early Run 3, legacy L1 RoIs
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddLegacyL1JetRoIsToSlimmingHelper
        AddLegacyL1JetRoIsToSlimmingHelper(SlimmingHelper = TOPQ7SlimmingHelper)

    # Output stream    
    TOPQ7ItemList = TOPQ7SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=TOPQ7ItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def TOPQ7Cfg(flags):

    logTOPQ7.info('****************** STARTING TOPQ7 *****************')

    stream_name = 'StreamDAOD_TOPQ7'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    TOPQ7TriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(TOPQ7KernelCfg(
        flags,
        name="TOPQ7Kernel",
        StreamName = stream_name,
        TriggerListsHelper = TOPQ7TriggerListsHelper
    ))
    # TOPQ7 content
    acc.merge(TOPQ7CoreCfg(
        flags,
        "TOPQ7",
        StreamName = stream_name,
        TriggerListsHelper = TOPQ7TriggerListsHelper
        ))
    
    return acc
