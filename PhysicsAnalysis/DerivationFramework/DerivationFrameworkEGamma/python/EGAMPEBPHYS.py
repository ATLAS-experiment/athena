# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# =======================================================================
# EGAMPEBPHYS.py
# This defines DAOD_EGAMPEBPHYS, a specialist format for the EGammaPEB stream
# based on DAOD_PHYS. All calls to unavailable Muon objects are disabled.
#
# Tau, flavour, met, Higgs, AFP, large-r are removed. 
#
# MC, Truth, Trigger, CloseByIsolation, IFF, TrackParticleThinningTool 
# are currently disabled (to be enabled in the future?)
#
# CloseByIsolation, TrackParticleThinningTool fail due to requesting
# MDT geometry info, conditions config error?
# 
# IFF fails due to retriveing non-existant btag decoration
#
# Trigger fails due to missing HLTNav_Summary
#
# HLT TLA Photons and EMclusters are added.
# =======================================================================


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory #, LHCPeriod
from AthenaCommon.Logging import logging
logEGAMPEBPHYS = logging.getLogger('EGAMPEBPHYS')

def JetEGAMPEBPHYSConfig(ConfigFlags):
    from DerivationFrameworkJetEtMiss.JetCommonConfig import AddBadBatmanCfg, AddDistanceInTrainCfg, AddSidebandEventShapeCfg, AddEventCleanFlagsCfg
    
    """EGAMPEBPHYS config for jet reconstruction and decorations"""

    acc = ComponentAccumulator()

    
    # modified StandardJetsInDerivCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo_deriv, AntiKt4EMPFlow, AntiKt4EMPFlow_deriv, flavourghosts, truthlabels
    from JetRecConfig.JetRecConfig import JetRecCfg

    acc = ComponentAccumulator()
    
    standardghosts_EgammaPEB = ["Track","Truth","Tower"] # No muon segments in the EGammaPEB stream

    # AntiKt4EMPFlow_EgammaPEB = AntiKt4EMPFlow.clone(
    #     ghostdefs = standardghosts_EgammaPEB+flavourghosts
    # )

    AntiKt4EMPFlow_EgammaPEB_deriv = AntiKt4EMPFlow_deriv.clone(
        ghostdefs = standardghosts_EgammaPEB+flavourghosts,
        modifiers = AntiKt4EMPFlow.modifiers+("JetPtAssociation","NNJVT","CaloEnergiesClus","JetPileupLabel","qgtransformer")+truthlabels   #no fJVT
    )

    AntiKt4EMTopo_EGammaPEB_deriv = AntiKt4EMTopo_deriv.clone(ghostdefs = standardghosts_EgammaPEB+["TrackLRT"]+flavourghosts)

    jetList = [AntiKt4EMTopo_EGammaPEB_deriv, AntiKt4EMPFlow_EgammaPEB_deriv]

    for jd in jetList:
        acc.merge(JetRecCfg(ConfigFlags,jd))

    # JetEGAMPEBPHYSCfg
    if "McEventCollection#GEN_EVENT" not in ConfigFlags.Input.TypedCollections:
        acc.merge(AddBadBatmanCfg(ConfigFlags))
    acc.merge(AddDistanceInTrainCfg(ConfigFlags))
    acc.merge(AddSidebandEventShapeCfg(ConfigFlags))
    acc.merge(AddEventCleanFlagsCfg(ConfigFlags))
    
    return acc
    

def EGAMPEBPHYSCommonAugmentationsCfg(flags,**kwargs):
    """Configure the EGAMPEBPHYS augmentation, modified from common Phys"""
    acc = ComponentAccumulator()

    from TrkConfig.VertexFindingFlags import VertexSortingSetup
    if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting:
        from DerivationFrameworkPhys.GNNVertexConfig import GNNVertexCfg
        acc.merge(GNNVertexCfg(flags))
        
    # # MC truth
    # if flags.Input.isMC:
    #     from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
    #         AddStandardTruthContentsCfg,
    #         AddHFAndDownstreamParticlesCfg,
    #         AddMiniTruthCollectionLinksCfg,
    #         AddPVCollectionCfg,
    #         TruthClassificationAugmentationsCfg)
    #     acc.merge(TruthClassificationAugmentationsCfg(flags))
    #     from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthCharmToolCfg
    #     PhysCommonTruthCharmTool = acc.getPrimaryAndMerge(DFCommonTruthCharmToolCfg(
    #         flags,
    #         name = "PhysCommonTruthCharmTool"))
    #     CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    #     acc.addEventAlgo(CommonAugmentation("PhysCommonTruthCharmKernel",AugmentationTools=[PhysCommonTruthCharmTool]))
    #     acc.merge(AddHFAndDownstreamParticlesCfg(flags))
    #     acc.merge(AddStandardTruthContentsCfg
    #               (flags,
    #                navInputCollections = ["TruthElectrons",
    #                                       "TruthMuons",
    #                                       "TruthPhotons",
    #                                       "TruthTaus",
    #                                       "TruthNeutrinos",
    #                                       "TruthBSM",
    #                                       "TruthBottom",
    #                                       "TruthTop",
    #                                       "TruthBoson",
    #                                       "TruthCharm",
    #                                       "TruthHFWithDecayParticles"]))
    #     # Re-point links on reco objects
    #     acc.merge(AddMiniTruthCollectionLinksCfg(flags))
    #     acc.merge(AddPVCollectionCfg(flags))

    # InDet, Muon, Egamma common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    # TODO: need to find the new flags equivalent for the missing settings below, then we can
    # drop these kwargs and do everything via the flags
    acc.merge(InDetCommonCfg(flags,
                             DoVertexFinding = flags.Tracking.doVertexFinding,
                             AddPseudoTracks = flags.Tracking.doPseudoTracking,
                             DecoLRTTTVA = False,
                             DoR3LargeD0 = flags.Tracking.doLargeD0,
                             StoreSeparateLargeD0Container = flags.Tracking.storeSeparateLargeD0Container,
                             MergeLRT = False)) 
    acc.merge(EGammaCommonCfg(flags))
    # Jets,
    acc.merge(JetEGAMPEBPHYSConfig(flags))
    #We also need to build links between the newly created jet constituents (GlobalFE)
    #and electrons,photons,muons and taus
    from eflowRec.PFCfg import getEGamFlowElementAssocAlgorithm 
    PFCfgresult=ComponentAccumulator()
    kwargs.setdefault("useGlobal", True)
    PFCfgresult.addEventAlgo(getEGamFlowElementAssocAlgorithm(flags, algName="PFEGamGlobalFlowElementAssoc", **kwargs))
    acc.merge(PFCfgresult)

    from AssociationUtils.AssociationUtilsConfig import FEAssociationCfg
    acc.merge(FEAssociationCfg(flags,
        SmallRJetChargedFELinksDecorKey="",
        SmallRJetNeutralFELinksDecorKey="",
        LargeRJetChargedFELinksDecorKey="",
        LargeRJetNeutralFELinksDecorKey=""))
    
    # # Trigger matching and postprocessing
    # if flags.Reco.EnableTrigger or flags.Trigger.triggerConfig == 'INFILE':

    #     from DerivationFrameworkPhys.TriggerMatchingCommonConfig import TriggerMatchingCommonRun3Cfg
    #     triggerListsHelper = kwargs['TriggerListsHelper']

    #     # This sets up the Run-3 style navigation slimming for trigger-matching from DAOD
    #     acc.merge(TriggerMatchingCommonRun3Cfg(
    #         flags, TriggerList = triggerListsHelper.Run3TriggerNames + ["HLT_2g13_loose_EgammaPEBTLA_L12DR15-0M30-2eEM12L","HLT_2g13_loose_EgammaPEBTLA_L113DR25-25M70-2eEM12L"]))

    return acc

# Main algorithm config
def EGAMPEBPHYSKernelCfg(flags, name='EGAMPEBPHYSKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for EGAMPEBPHYS"""
    acc = ComponentAccumulator()

    # Common augmentations
    acc.merge(EGAMPEBPHYSCommonAugmentationsCfg(
        flags, 
        TriggerListsHelper     = kwargs['TriggerListsHelper']
    ))

    # Thinning tools
    # These are set up in PhysCommonThinningConfig. Only thing needed here the list of tools to schedule
    nametag = name.replace('Kernel', '') #get the name to label the tools below such that other formats can use this KernelCfg
    thinningToolsArgs = {
        # 'TrackParticleThinningToolName'       : nametag+"TrackParticleThinningTool",
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


def EGAMPEBPHYSCoreCfg(flags, name_tag='EGAMPEBPHYS', StreamName='StreamDAOD_EGAMPEBPHYS', TriggerListsHelper=None, addExtraVariables=None):
    
    if TriggerListsHelper is None:
        from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
        TriggerListsHelper = TriggerListsHelper(flags)
    
    acc = ComponentAccumulator()

    # ## CloseByIsolation correction augmentation
    # ## For the moment, run BOTH CloseByIsoCorrection on AOD AND add in augmentation variables to be able to also run on derivation (the latter part will eventually be suppressed)
    # from IsolationSelection.IsolationSelectionConfig import  IsoCloseByAlgsCfg
    # acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, stream_name = StreamName))

    # ## IFF augmentation - Adding Lepton Taggers
    # from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg
    # acc.merge(DecoratePLITAlgsCfg(flags, lepton_type="Electrons"))
    
    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    EGAMPEBPHYSSlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    EGAMPEBPHYSSlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets"
                                          ]

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

    EGAMPEBPHYSSlimmingHelper.StaticContent = StaticContent
   
    # Extra content
    EGAMPEBPHYSSlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.IsoFixedCone5PtPUsub",
                                          "AntiKt4EMPFlowJets.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
                                          "TruthPrimaryVertices.t.x.y.z",
                                          "InDetTrackParticles.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                          "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification.HF_ClassificationC5J20.HF_ClassificationC5J25.HF_ClassificationC15J20.HF_ClassificationC15J25",
                                          ]

    if addExtraVariables:
        EGAMPEBPHYSSlimmingHelper.ExtraVariables += addExtraVariables

    # HSGNN Score
    from TrkConfig.VertexFindingFlags import VertexSortingSetup
    if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting:
        EGAMPEBPHYSSlimmingHelper.ExtraVariables += ["PrimaryVertices.gnnScore"]


    # IFF extra content
    # from LeptonTaggers.LeptonTaggersConfig import GetExtraPLITVariablesForDxAOD
    # EGAMPEBPHYSSlimmingHelper.ExtraVariables += GetExtraPLITVariablesForDxAOD()
                                  
    # # Truth extra content
    # if flags.Input.isMC:

    #     from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
    #     addTruth3ContentToSlimmerTool(EGAMPEBPHYSSlimmingHelper)
    #     EGAMPEBPHYSSlimmingHelper.AllVariables += ['TruthLHEParticles','InTimeAntiKt4TruthJets','OutOfTimeAntiKt4TruthJets']
    #     EGAMPEBPHYSSlimmingHelper.ExtraVariables += ["Electrons.TruthLink",
    #                                           "Muons.TruthLink",
    #                                           "Photons.TruthLink",
    #                                           "AntiKt4TruthDressedWZJets.IsoFixedCone5Pt.HFHadronOriginID",
    #                                           "TruthHFWithDecayParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.parentLinks.childLinks.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status",
    #                                           "TruthHFWithDecayVertices.incomingParticleLinks.outgoingParticleLinks.uid.status.x.y.z.t",
    #                                           "TruthCharm.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.parentLinks.childLinks.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status.barcode.polarizationPhi.polarizationTheta",
    #                                           "TruthPileupParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status.barcode.PVz.pileupEventNumber.parentHadronID"]

    #     from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
    #     acc.merge(AddTauAndDownstreamParticlesCfg(flags))
    #     EGAMPEBPHYSSlimmingHelper.ExtraVariables += ["TruthTausWithDecayParticles.prodVtxLink.prodVtxLink.prodVtxLink.decayVtxLink.decayVtxLink.decayVtxLink.m.px.py.pz.e.pdgId.Classification.uid.classifierParticleOrigin.classifierParticleType.classifierParticleOutCome.status",
    #                                           "TruthTausWithDecayVertices.incomingParticleLinks.outgoingParticleLinks.uid.status.x.y.z.t"]


    # Trigger content
    EGAMPEBPHYSSlimmingHelper.IncludeTriggerNavigation = False
    EGAMPEBPHYSSlimmingHelper.IncludeJetTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeMuonTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeEGammaTriggerContent = False #TODO: enable?
    EGAMPEBPHYSSlimmingHelper.IncludeTauTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeEtMissTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeBJetTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeBPhysTriggerContent = False
    EGAMPEBPHYSSlimmingHelper.IncludeMinBiasTriggerContent = False
    # Compact b-jet trigger matching info
    EGAMPEBPHYSSlimmingHelper.IncludeBJetTriggerByYearContent = False

    #Trigger content for EgammaPEB 
    EGAMPEBPHYSSlimmingHelper.AllVariables += ['HLT_egamma_Photons_TLA','HLT_CaloEMClusters_Photon']

    # Trigger matching
    # Run 3, or Run 2 with navigation conversion
    # if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
    #     from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
    #     AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(EGAMPEBPHYSSlimmingHelper)

    # L1 trigger objects
    from Campaigns.Utils import getDataYear
    if getDataYear(flags) >= 2024:
        # Run 3 with Phase I jet RoIs.
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = EGAMPEBPHYSSlimmingHelper)
    elif getDataYear(flags) >= 2015:
        # Run 2 and early Run 3, legacy L1 RoIs
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddLegacyL1JetRoIsToSlimmingHelper
        AddLegacyL1JetRoIsToSlimmingHelper(SlimmingHelper = EGAMPEBPHYSSlimmingHelper)

    # Output stream    
    EGAMPEBPHYSItemList = EGAMPEBPHYSSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=EGAMPEBPHYSItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def EGAMPEBPHYSCfg(flags):

    logEGAMPEBPHYS.info('****************** STARTING EGAMPEBPHYS *****************')

    stream_name = 'StreamDAOD_EGAMPEBPHYS'
    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    EGAMPEBPHYSTriggerListsHelper = TriggerListsHelper(flags)

    # Common augmentations
    acc.merge(EGAMPEBPHYSKernelCfg(
        flags,
        name="EGAMPEBPHYSKernel",
        StreamName = stream_name,
        TriggerListsHelper = EGAMPEBPHYSTriggerListsHelper
    ))
    # EGAMPEBPHYS content
    acc.merge(EGAMPEBPHYSCoreCfg(
        flags,
        "EGAMPEBPHYS",
        StreamName = stream_name,
        TriggerListsHelper = EGAMPEBPHYSTriggerListsHelper
        ))
    
    return acc
