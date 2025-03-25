# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG_PU.py
# This defines DAOD_FTAG_PU, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the using btagging for the pileup dataset.
# It requires the flag FTAG_PU in Derivation_tf.py   
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaConfiguration.Enums import LHCPeriod

from DerivationFrameworkEGamma.ElectronsCPDetailedContent import (
    ElectronsCPDetailedContent
)
from DerivationFrameworkFlavourTag.FtagBaseContent import (
    addCommonAugmentation
)


# Main algorithm config
def FTAG_PUKernelCfg(flags, name='FTAG_PUKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG_PU"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    augmentationTools = []

    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowJets"]))

    # thinning tools
    thinningTools = []

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools))      

    # Extra jet content:
    acc.merge(FTAG_PUExtraContentCfg(flags))

    return acc


def FTAG_PUCoreCfg(flags, name_tag='FTAG_PU', extra_SmartCollections=None, extra_AllVariables=None, trigger_option='', TriggerListsHelper = None):

    if extra_SmartCollections is None: extra_SmartCollections = []
    if extra_AllVariables is None: extra_AllVariables = []


    acc = ComponentAccumulator()

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG_PUSlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    # Many of these are added to AllVariables below as well. We add
    # these items in both places in case some of the smart collections
    # add variables from some other collection. For flavor tagging,
    # for example will add jet variables.
    
    from DerivationFrameworkFlavourTag import FtagBaseContent

    FTAG_PUSlimmingHelper.SmartCollections = []
    FtagBaseContent.add_baseline_slimming_smartcollections(FTAG_PUSlimmingHelper)

    addCommonAugmentation(flags, acc, FTAG_PUSlimmingHelper)

    FTAG_PUSlimmingHelper.SmartCollections += [
                                           "BTagging_AntiKt4UFOCSSK",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKt4EMPFlowJets_FTAG",
                                           "AntiKt4EMPFlowByVertexJets_FTAG",
                                          ]


    if len(extra_SmartCollections)>0:
        for a_container in extra_SmartCollections:
            if a_container not in FTAG_PUSlimmingHelper.SmartCollections:
                FTAG_PUSlimmingHelper.SmartCollections.append(a_container)

    FTAG_PUSlimmingHelper.AllVariables = []
    FtagBaseContent.add_baseline_slimming_allvariables(FTAG_PUSlimmingHelper)

    FTAG_PUSlimmingHelper.AllVariables += [
            "InDetLargeD0TrackParticles",
            "AntiKt4EMPFlowJets",
            "AntiKt4UFOCSSKJets",
            "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
            "UFOCSSK",
            "GlobalChargedParticleFlowObjects",
            "GlobalNeutralParticleFlowObjects",
            "CHSGChargedParticleFlowObjects",
            "CHSGNeutralParticleFlowObjects",
            "TruthParticles",
            "TruthVertices",
    ]
    
    if len(extra_AllVariables)>0:
        for a_container in extra_AllVariables:
            if a_container not in FTAG_PUSlimmingHelper.AllVariables:
                FTAG_PUSlimmingHelper.AllVariables.append(a_container)

    if flags.BTagging.Pseudotrack:
        FTAG_PUSlimmingHelper.AllVariables += [ "InDetPseudoTrackParticles" ]

    if flags.BTagging.Trackless:
        FTAG_PUSlimmingHelper.AllVariables += [
                "JetAssociatedPixelClusters",
                "JetAssociatedSCTClusters",
                ]

    # Add additional e/gamma variables
    FTAG_PUSlimmingHelper.ExtraVariables += ElectronsCPDetailedContent

    # update AppendToDictionary
    extra_AppendToDictionary = {} #only add those items specifically for FTAG_PU here!
    FtagBaseContent.update_AppendToDictionary_in_SlimmingHelper(FTAG_PUSlimmingHelper, flags, extra_AppendToDictionary)

    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG_PUSlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

    # Add ExtraVariables
    FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG_PUSlimmingHelper, flags)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG_PUSlimmingHelper, trigger_option)
    FtagBaseContent.trigger_matching(FTAG_PUSlimmingHelper, TriggerListsHelper, flags)

    jetOutputList = ["AntiKt4UFOCSSKJets", "AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(FTAG_PUSlimmingHelper, jetOutputList, FTAG_PUSlimmingHelper.SmartCollections)
    
    # Flavour tagging (Mario)
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowByVertexJets"], ByVertex=True, dzCut_vec=[5], useMinZ0Vertex_vec=[True,False]))
  
    # Output stream    
    FTAG_PUItemList = FTAG_PUSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=FTAG_PUItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def FTAG_PUCfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG_PUTriggerListsHelper = TriggerListsHelper(flags)
   
    # name_tag has to be consistent between KernelCfg and CoreCfg
    FTAG_PU_name_tag = 'FTAG_PU'

    # Common augmentations
    acc.merge(FTAG_PUKernelCfg(flags, name=FTAG_PU_name_tag + "Kernel", StreamName = 'StreamDAOD_'+FTAG_PU_name_tag, TriggerListsHelper = FTAG_PUTriggerListsHelper))
    # Content of FTAG_PU 
    acc.merge(FTAG_PUCoreCfg(flags, FTAG_PU_name_tag, trigger_option='FTAG_PU', TriggerListsHelper = FTAG_PUTriggerListsHelper))

    return acc

def FTAG_PUExtraContentCfg(flags):
    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    jetList = []
    #=======================================
    # CSSK R = 0.4 UFO jets
    #=======================================
    from JetRecConfig.StandardSmallRJets import AntiKt4UFOCSSK
    jetList += [AntiKt4UFOCSSK]

    from JetRecConfig.StandardSmallRJets import AntiKt4PV0Track, AntiKt4EMPFlowByVertex 

    #======================================= 
    # R = 0.4 track-jets (needed for Rtrk) 
    #=======================================
    jetList += [AntiKt4PV0Track]

    #======================================= 
    # R = 0.4 by-vertex jets 
    #=======================================
    jetList += [AntiKt4EMPFlowByVertex]

    for jd in jetList:
        acc.merge(JetRecCfg(flags,jd))

    #=======================================
    # More detailed truth information
    #=======================================

    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTopQuarkAndDownstreamParticlesCfg
        acc.merge(AddTopQuarkAndDownstreamParticlesCfg(flags, generations=4, rejectHadronChildren=True))

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



