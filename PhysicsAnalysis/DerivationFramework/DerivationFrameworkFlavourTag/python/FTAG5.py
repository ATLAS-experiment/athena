# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# DAOD_FTAG1.py
# This defines DAOD_FTAG1, an unskimmed DAOD format for Run 3.
# It contains the variables and objects needed for the large majority 
# of physics analyses in ATLAS.
# It requires the flag FTAG1 in Derivation_tf.py   
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
def FTAG1KernelCfg(flags, name='FTAG1Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for FTAG1"""
    acc = ComponentAccumulator()

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(flags, TriggerListsHelper = kwargs['TriggerListsHelper']))

    nametag = name.replace('Kernel', '') #get the name to label the tools below such that other formats can use this KernelCfg
    augmentationTools = []
    # Add V0Tool
    if flags.BTagging.AddV0Finder:
        acc.merge(V0ToolCfg(flags, augmentationTools=augmentationTools, tool_name_prefix=nametag, container_name_prefix="FTAG"))

    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    #acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowByVertexJets"], ByVertex=True))
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowJets"]))

    # thinning tools
    thinningTools = []

    # Finally the kernel itself
    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = augmentationTools, ThinningTools = thinningTools))      

    # Extra jet content:
    acc.merge(FTAG1ExtraContentCfg(flags))

    return acc


def FTAG1CoreCfg(flags, name_tag='FTAG1', extra_SmartCollections=None, extra_AllVariables=None, trigger_option='', TriggerListsHelper = None):

    if extra_SmartCollections is None: extra_SmartCollections = []
    if extra_AllVariables is None: extra_AllVariables = []


    acc = ComponentAccumulator()

    # ============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    
    FTAG1SlimmingHelper = SlimmingHelper(name_tag+"SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    # Many of these are added to AllVariables below as well. We add
    # these items in both places in case some of the smart collections
    # add variables from some other collection. For flavor tagging,
    # for example will add jet variables.
    
    from DerivationFrameworkFlavourTag import FtagBaseContent

    FTAG1SlimmingHelper.SmartCollections = []
    FtagBaseContent.add_baseline_slimming_smartcollections(FTAG1SlimmingHelper)

    addCommonAugmentation(flags, acc, FTAG1SlimmingHelper)

    FTAG1SlimmingHelper.SmartCollections += [
                                           "BTagging_AntiKt4UFOCSSK",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKt4EMPFlowJets_FTAG",
                                           "AntiKt4EMPFlowByVertexJets_FTAG",
                                          ]

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        FTAG1SlimmingHelper.SmartCollections += [
                                                "AntiKt4EMTopoJets",
                                                "BTagging_AntiKt4EMTopo",
                                                "MET_Baseline_AntiKt4EMTopo",
                                                ]

    if len(extra_SmartCollections)>0:
        for a_container in extra_SmartCollections:
            if a_container not in FTAG1SlimmingHelper.SmartCollections:
                FTAG1SlimmingHelper.SmartCollections.append(a_container)

    FTAG1SlimmingHelper.AllVariables = []
    FtagBaseContent.add_baseline_slimming_allvariables(FTAG1SlimmingHelper)

    FTAG1SlimmingHelper.AllVariables += [
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
            "BTagging_AntiKt4EMPFlowByVertex",
    ]
    
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        FTAG1SlimmingHelper.AllVariables += [
            "AntiKt4EMTopoJets",
            "BTagging_AntiKt4EMTopo",
            "BTagging_AntiKt4EMTopoJFVtx",
            "BTagging_AntiKt4EMTopoSecVtx",
            "AntiKt4TruthJets",
            ]


    if len(extra_AllVariables)>0:
        for a_container in extra_AllVariables:
            if a_container not in FTAG1SlimmingHelper.AllVariables:
                FTAG1SlimmingHelper.AllVariables.append(a_container)

    if flags.BTagging.Pseudotrack:
        FTAG1SlimmingHelper.AllVariables += [ "InDetPseudoTrackParticles" ]

    if flags.BTagging.Trackless:
        FTAG1SlimmingHelper.AllVariables += [
                "JetAssociatedPixelClusters",
                "JetAssociatedSCTClusters",
                ]

    # Add additional e/gamma variables
    FTAG1SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent

    # update AppendToDictionary
    extra_AppendToDictionary = {} #only add those items specifically for FTAG1 here!
    FtagBaseContent.update_AppendToDictionary_in_SlimmingHelper(FTAG1SlimmingHelper, flags, extra_AppendToDictionary)

    # Static content
    StaticContent = [] #only add extra static content for FTAG1 here!
    if flags.BTagging.AddV0Finder:
        FTAGV0ContainerName = "FTAGRecoV0Candidates"
        FTAGKshortContainerName = "FTAGRecoKshortCandidates"
        FTAGLambdaContainerName = "FTAGRecoLambdaCandidates"
        FTAGLambdabarContainerName = "FTAGRecoLambdabarCandidates"
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGV0ContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGV0ContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGKshortContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGKshortContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGLambdaContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGLambdaContainerName]
        StaticContent += ["xAOD::VertexContainer#%s"        %                 FTAGLambdabarContainerName]
        StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % FTAGLambdabarContainerName]
        CascadeCollections = []
        CascadeCollections += ["FTAGJpsiKshortCascadeSV2", "FTAGJpsiKshortCascadeSV1"]
        CascadeCollections += ["FTAGJpsiLambdaCascadeSV2", "FTAGJpsiLambdaCascadeSV1"]
        CascadeCollections += ["FTAGJpsiLambdabarCascadeSV2", "FTAGJpsiLambdabarCascadeSV1"]
        for cascades in CascadeCollections:
            StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
            StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]


    FtagBaseContent.add_static_content_to_SlimmingHelper(FTAG1SlimmingHelper, flags, StaticContent)


    # Add truth containers
    if flags.Input.isMC:
        FtagBaseContent.add_truth_to_SlimmingHelper(FTAG1SlimmingHelper)
        if flags.Trigger.EDMVersion == 3:
            # Add truth labels to Run 3 trigger jets
            from DerivationFrameworkFlavourTag.FtagDerivationConfig import HLTJetFTagDecorationCfg
            acc.merge(HLTJetFTagDecorationCfg(flags))

    # Add ExtraVariables
    FtagBaseContent.add_ExtraVariables_to_SlimmingHelper(FTAG1SlimmingHelper, flags)
   
    # Trigger content
    FtagBaseContent.trigger_setup(FTAG1SlimmingHelper, trigger_option)
    FtagBaseContent.trigger_matching(FTAG1SlimmingHelper, TriggerListsHelper, flags)

    jetOutputList = ["AntiKt4UFOCSSKJets", "AntiKt4EMPFlowByVertexJets"]
    from DerivationFrameworkJetEtMiss.JetCommonConfig import addJetsToSlimmingTool
    addJetsToSlimmingTool(FTAG1SlimmingHelper, jetOutputList, FTAG1SlimmingHelper.SmartCollections)
    
    # Flavour tagging (Mario)
    from DerivationFrameworkFlavourTag.FtagDerivationConfig import JetCollectionsBTaggingCfg
    acc.merge(JetCollectionsBTaggingCfg(flags, ["AntiKt4EMPFlowByVertexJets"], ByVertex=True))
  
    # Output stream    
    FTAG1ItemList = FTAG1SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_"+name_tag, ItemList=FTAG1ItemList, AcceptAlgs=[name_tag+"Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_"+name_tag, AcceptAlgs=[name_tag+"Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc

def FTAG1Cfg(flags):

    acc = ComponentAccumulator()

    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    FTAG1TriggerListsHelper = TriggerListsHelper(flags)
   
    # name_tag has to be consistent between KernelCfg and CoreCfg
    FTAG1_name_tag = 'FTAG1'

    # Common augmentations
    acc.merge(FTAG1KernelCfg(flags, name=FTAG1_name_tag + "Kernel", StreamName = 'StreamDAOD_'+FTAG1_name_tag, TriggerListsHelper = FTAG1TriggerListsHelper))
    # Content of FTAG1 
    acc.merge(FTAG1CoreCfg(flags, FTAG1_name_tag, trigger_option='FTAG1', TriggerListsHelper = FTAG1TriggerListsHelper))

    return acc


def V0ToolCfg(flags, augmentationTools=None, tool_name_prefix="FTAG1", container_name_prefix="FTAG"):
    
    acc = ComponentAccumulator()
    
    if augmentationTools is None:
        augmentationTools = []
    
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (
        BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
        BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import (
        PrimaryVertexRefittingToolCfg, JpsiFinderCfg)

    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, tool_name_prefix))
    acc.addPublicTool(V0Tools)

    vkalvrt = acc.popToolsAndMerge(
        BPHY_TrkVKalVrtFitterCfg(flags, tool_name_prefix))

    trackselect = acc.popToolsAndMerge(
        BPHY_InDetDetailedTrackSelectorToolCfg(flags, tool_name_prefix))

    vpest = acc.popToolsAndMerge(
        BPHY_VertexPointEstimatorCfg(flags, tool_name_prefix))

    JpsiFinder = acc.popToolsAndMerge(JpsiFinderCfg(flags,
            name                        = tool_name_prefix+"JpsiFinder",
            muAndMu                     = True,
            muAndTrack                  = False,
            TrackAndTrack               = False,
            assumeDiMuons               = True,
            invMassUpper                = 4000.0,
            invMassLower                = 2600.0,
            Chi2Cut                     = 200.,
            oppChargesOnly              = True,
            combOnly                    = True,
            atLeastOneComb              = False,
            useCombinedMeasurement      = False, # Only takes effect if combOnly=True   
            muonCollectionKey           = "Muons",
            TrackParticleCollection     = "InDetTrackParticles",
            V0VertexFitterTool          = None,             # V0 vertex fitter
            useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
            TrkVertexFitterTool         = acc.addPublicTool(vkalvrt),        # VKalVrt vertex fitter
            TrackSelectorTool           = acc.addPublicTool(trackselect),
            VertexPointEstimator        = acc.addPublicTool(vpest),
            useMCPCuts                  = False))
    acc.addPublicTool(JpsiFinder)
    JpsiSelectAndWrite   = CompFactory.DerivationFramework.Reco_Vertex(
            name                   = tool_name_prefix+"JpsiSelectAndWrite",
            VertexSearchTool       = JpsiFinder,
            OutputVtxContainerName = container_name_prefix+"JpsiCandidates",
            PVContainerName        = "PrimaryVertices",
            V0Tools                = V0Tools,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            RefPVContainerName     = "SHOULDNOTBEUSED",
            DoVertexType = 1)
    Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
            name                  = tool_name_prefix+"_Select_Jpsi2mumu",
            HypothesisName        = "Jpsi",
            InputVtxContainerName = container_name_prefix+"JpsiCandidates",
            V0Tools               = V0Tools,
            VtxMassHypo           = 3096.916,
            MassMin               = 2600.0,
            MassMax               = 4000.0,
            Chi2Max               = 200,
            DoVertexType =1)

    V0ContainerName = container_name_prefix+"RecoV0Candidates"
    KshortContainerName = container_name_prefix+"RecoKshortCandidates"
    LambdaContainerName = container_name_prefix+"RecoLambdaCandidates"
    LambdabarContainerName = container_name_prefix+"RecoLambdabarCandidates"

    from DerivationFrameworkBPhys.V0ToolConfig import BPHY_Reco_V0FinderCfg
    Reco_V0Finder = acc.popToolsAndMerge(BPHY_Reco_V0FinderCfg(
        flags, derivation = tool_name_prefix,
        V0ContainerName = V0ContainerName,
        KshortContainerName = KshortContainerName,
        LambdaContainerName = LambdaContainerName,
        LambdabarContainerName = LambdabarContainerName,
        CheckVertexContainers = [container_name_prefix+'JpsiCandidates']))

    from TrkConfig.TrkVKalVrtFitterConfig import JpsiV0VertexFitCfg
    JpsiV0VertexFit = acc.popToolsAndMerge(JpsiV0VertexFitCfg(flags))
    acc.addPublicTool(JpsiV0VertexFit)

    JpsiKshort  = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiKshort",
            V0Tools                 = V0Tools,
            HypothesisName          = "Bd",
            TrkVertexFitterTool     = JpsiV0VertexFit,
            V0Hypothesis            = 310,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 400.,
            V0MassUpperCut          = 600.,
            MassLowerCut            = 4300.,
            MassUpperCut            = 6300.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices2",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiKshortCascadeSV2", container_name_prefix+"JpsiKshortCascadeSV1"],
            V0Vertices              = V0ContainerName)

    JpsiLambda   = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiLambda",
            V0Tools                 = V0Tools,
            HypothesisName          = "Lambda_b",
            TrkVertexFitterTool     = JpsiV0VertexFit,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            V0Hypothesis            = 3122,
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 1050.,
            V0MassUpperCut          = 1250.,
            MassLowerCut            = 4600.,
            MassUpperCut            = 6600.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices3",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiLambdaCascadeSV2", container_name_prefix+"JpsiLambdaCascadeSV1"],
            V0Vertices              = V0ContainerName)
    JpsiLambdabar         = CompFactory.DerivationFramework.JpsiPlusV0Cascade(
            name                    = tool_name_prefix+"JpsiLambdabar",
            HypothesisName          = "Lambda_bbar",
            V0Tools                 = V0Tools,
            TrkVertexFitterTool     = JpsiV0VertexFit,
            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
            V0Hypothesis            = -3122,
            JpsiMassLowerCut        = 2800.,
            JpsiMassUpperCut        = 4000.,
            V0MassLowerCut          = 1050.,
            V0MassUpperCut          = 1250.,
            MassLowerCut            = 4600.,
            MassUpperCut            = 6600.,
            RefitPV                 = True,
            RefPVContainerName      = container_name_prefix+"RefittedPrimaryVertices4",
            JpsiVertices            = container_name_prefix+"JpsiCandidates",
            CascadeVertexCollections= [container_name_prefix+"JpsiLambdabarCascadeSV2", container_name_prefix+"JpsiLambdabarCascadeSV1"],
            V0Vertices              = V0ContainerName)

    _augmentationTools = [JpsiSelectAndWrite,  Select_Jpsi2mumu,
            Reco_V0Finder, JpsiKshort, JpsiLambda, JpsiLambdabar,
            ]
    for t in  _augmentationTools : acc.addPublicTool(t)
    augmentationTools += _augmentationTools
    return acc

def FTAG1ExtraContentCfg(flags):
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



