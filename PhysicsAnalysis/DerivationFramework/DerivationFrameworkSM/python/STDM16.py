# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# STDM16.py for c-fragmentation analysi 
# Contact: eleni.skorda@cern.ch or andrew.chisolm@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
logSTDM16 = logging.getLogger('STDM16')

# Particle masses
massD0 = 1864.84 # MeV
massPiPlus = 139.570 # MeV
massKPlus = 493.677 # MeV

CandidatesContainerName= "STDM16_D0Candidates"
streamName = "StreamDAOD_STDM16"

# Main algorithm config

def DStarSelectionToolCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.DStarSelectionTool(name = "STDM16_DStarSelectionTool",
                                                                         InputVtxContainerName = CandidatesContainerName,
                                                                         DeltaMassMax          = 200.0), primary = True)
    return acc 


def STDM16Kernel(flags, name='STDM16Kernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for STDM16"""
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)

    acc = ComponentAccumulator()

    # #============================================================================
    # # Adding jets, from DerivationFrameworkPhys/python/PhysCommonConfig.py
    # #============================================================================

    # MC truth
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import (
            AddStandardTruthContentsCfg,
            #AddHFAndDownstreamParticlesCfg,
            AddMiniTruthCollectionLinksCfg,
            AddPVCollectionCfg,
            AddTruthCollectionNavigationDecorationsCfg,
            TruthClassificationAugmentationsCfg)
        acc.merge(TruthClassificationAugmentationsCfg(flags))
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import DFCommonTruthCharmToolCfg
        STDM16CommonTruthCharmTool = acc.getPrimaryAndMerge(DFCommonTruthCharmToolCfg(
            flags,
            name = "STDM16CommonTruthCharmTool"))
        CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
        acc.addEventAlgo(CommonAugmentation("STDM16CommonTruthCharmKernel",AugmentationTools=[STDM16CommonTruthCharmTool]))

        acc.merge(AddStandardTruthContentsCfg(flags))
        acc.merge(AddTruthCollectionNavigationDecorationsCfg(
            flags,
            TruthCollections=["TruthMuons", 
                              "TruthBottom", 
                              "TruthCharm"
                              ],
            prefix = 'STDM16_'))
        # Re-point links on reco objects
        acc.merge(AddMiniTruthCollectionLinksCfg(flags))
        acc.merge(AddPVCollectionCfg(flags))

    # InDet, Muon, Egamma common augmentations
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    from DerivationFrameworkMuons.MuonsCommonConfig import MuonsCommonCfg
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
    acc.merge(MuonsCommonCfg(flags))
    acc.merge(EGammaCommonCfg(flags))

    
    from DerivationFrameworkJetEtMiss.JetCommonConfig import JetCommonCfg
    #from DerivationFrameworkFlavourTag.FtagDerivationConfig import FtagJetCollectionsCfg
    
    acc.merge(JetCommonCfg(flags))
    
   # FTagJetColl = ['AntiKt4EMPFlowJets', 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets']

    #acc.merge(FtagJetCollectionsCfg(flags,FTagJetColl))
        
    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg     
    
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, "STDM16"))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, "STDM16"))    # VKalVrt vertex fitter
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, "STDM16"))
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, "STDM16"))     
    v0fitter = acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))     

    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    acc.addPublicTool(trackselect)
    acc.addPublicTool(vpest)
    acc.addPublicTool(v0fitter)
    
    #====================================================================
    # AUGMENTATION TOOLS
    # Largely based on PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/share/BPHY1.py
    #====================================================================
    
    STDM16_AugmentationTools = []
    
    #====================================================================
    # Perform di-track vertex fit for D0 -> K- pi+ (and c.c.) candidates
    #====================================================================
    
    STDM16_Finder_D0 = CompFactory.Analysis.JpsiFinder(
        name                        = "STDM16_Finder_D0",
        # OutputLevel                 = INFO,
        muAndMu                     = False,
        muAndTrack                  = False,
        TrackAndTrack               = True,
        assumeDiMuons               = False,
        invMassUpper                = 3000.0,
        invMassLower                = 0.0,
        Chi2Cut                     = 200.,
        oppChargesOnly              = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False,
        track1Mass                  = massPiPlus, # Not very important, only used to calculate inv. mass cut, leave it loose here
        track2Mass                  = massPiPlus, # Not very important, only used to calculate inv. mass cut, leave it loose here
        trackThresholdPt            = 1000.0,
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = v0fitter,             # V0 vertex fitter
        useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False)

    acc.addPublicTool(STDM16_Finder_D0)

    
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg   
    STDM16_Reco_D0 = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "STDM16_Reco_D0",
        VertexSearchTool       = STDM16_Finder_D0,
        OutputVtxContainerName = "STDM16_D0Candidates",
        PVContainerName        = "PrimaryVertices",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 7, #ES not sure how this actually works, it only takes 7(all) and 1 but what does it mean ?????
        RefPVContainerName     = "SHOULDNOTBEUSED",
        RefitPV                = False)

    acc.addPublicTool(STDM16_Reco_D0)
    STDM16_AugmentationTools += [STDM16_Reco_D0]

    #====================================================================
    # Perform some selection on the D0 vertex candidates
    #====================================================================

    # Loose D0 vertex cuts
    cutMinLxy = -999 # mm
    cutMinMass = 1600.0 # MeV
    cutMaxMass = 2100.0 # MeV
    cutMaxChiSq = 50.0
    
    # Need two of these, one for each track mass hypothesis
    STDM16_Select_D0 = CompFactory.DerivationFramework.Select_onia2mumu(
        name                  = "STDM16_Select_D0",
        HypothesisName        = "D0",
        InputVtxContainerName = STDM16_Reco_D0.OutputVtxContainerName,
        TrkMasses             = [massPiPlus,massKPlus],
        VtxMassHypo           = massD0,
        MassMin               = cutMinMass,
        MassMax               = cutMaxMass,
        Chi2Max               = cutMaxChiSq,
        LxyMin                = cutMinLxy)

    acc.addPublicTool(STDM16_Select_D0)
    STDM16_AugmentationTools += [STDM16_Select_D0]
    
    STDM16_Select_D0b = CompFactory.DerivationFramework.Select_onia2mumu(
    name                  = "STDM16_Select_D0b",
    HypothesisName        = "D0b",
    InputVtxContainerName = STDM16_Reco_D0.OutputVtxContainerName,
    TrkMasses             = [massKPlus,massPiPlus],
    VtxMassHypo           = massD0,
    MassMin               = cutMinMass,
    MassMax               = cutMaxMass,
    Chi2Max               = cutMaxChiSq,
    LxyMin                = cutMinLxy)

    acc.addPublicTool(STDM16_Select_D0b)
    STDM16_AugmentationTools += [STDM16_Select_D0b]

    #====================================================================
    # Look for D*+ -> D0 + pi+ candidates by looking for tracks which
    # lead to low DeltaM w.r.t. D0 candidates. Good candidates (tracks and vertices) 
    # are augmented with "passed_Dstar" flag
    #====================================================================

    STDM16_DStarSelectionTool = acc.getPrimaryAndMerge(DStarSelectionToolCfg(flags,
                                                                          name = "STDM16_DStarSelectionTool",
                                                                          InputVtxContainerName = STDM16_Reco_D0.OutputVtxContainerName,
                                                                          DeltaMassMax          = 200.0))

    STDM16_AugmentationTools += [STDM16_DStarSelectionTool]
    

    #=======================================
    # SKIMMING TOOLS
    #=======================================
    STDM16_SkimmingTools = []

    #====================================================================
    # Only retain events with at least one D* candidate
    #====================================================================
    
    # Skimming based on number of vertex candidates
    SelectExpression = "count(STDM16_D0Candidates.passed_Dstar) > 0"

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import (
        xAODStringSkimmingToolCfg)
    STDM16_SelectEvent = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(
        flags, name = "STDM16_SelectEvent", expression = SelectExpression))
    STDM16_SkimmingTools += [STDM16_SelectEvent]
    
    #====================================================================
    # THINNING TOOLS
    #====================================================================
    STDM16_ThinningTools = []
    
    #====================================================================
    # Only retain vertices passing loose D0 selection in Select_onia2mumu tool
    #====================================================================
    
    STDM16_Thin_Vertex = CompFactory.DerivationFramework.Thin_vtxTrk(
        name                       = "STDM16_Thin_Vertex",
        StreamName                 = streamName,
        ThinTracks                 = False,
        VertexContainerNames       = ["STDM16_D0Candidates"],
        PassFlags                  = ["passed_Dstar"] )
    STDM16_ThinningTools += [STDM16_Thin_Vertex]
    acc.addPublicTool(STDM16_Thin_Vertex)

    #====================================================================
    # Only retain tracks associated with above D0 vertices and soft pion
    # candiates found in DStarSelectionTool
    #====================================================================

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import GenericObjectThinningCfg

    track_thinning_expression = "(InDetTrackParticles.trackPassDstar == 1)"
    STDM16_Thin_Tracks = acc.getPrimaryAndMerge(
        GenericObjectThinningCfg(flags,
                                 name            = "STDM16_Thin_Tracks",
                                 ContainerName   = "InDetTrackParticles",
                                 StreamName      = streamName,
                                 SelectionString = track_thinning_expression))
    STDM16_ThinningTools += [STDM16_Thin_Tracks]
    acc.addPublicTool(STDM16_Thin_Tracks)

    acc.addPublicTool(STDM16_SelectEvent)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("STDM16Kernel",
                                                                      SkimmingTools = STDM16_SkimmingTools,
                                                                      ThinningTools = STDM16_ThinningTools,
                                                                      AugmentationTools = STDM16_AugmentationTools))
      
    return acc


def STDM16Cfg(flags):

    acc = STDM16Kernel(flags)
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables

    # PFlow augmentation tool
    
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    

    ## primary vertices
    AllVariables += ["PrimaryVertices"]
        
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    

    #=======================================
    # Decide what to save 
    #=======================================
    
    StaticContent += ["xAOD::VertexContainer#%s"        % CandidatesContainerName ]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % CandidatesContainerName]

    
    # # Truth information for MC only
    if flags.Input.isMC :
        AllVariables += ["TruthEvents","TruthParticles", "TruthVertices","MuonTruthParticles", "AntiKt4TruthJets","AntiKt4TruthWZJets"]

    STDM16SlimmingHelper = SlimmingHelper("STDM16SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    
    STDM16SlimmingHelper.SmartCollections = ["AntiKt4EMPFlowJets",
                                             "EventInfo",
                                             "Muons",
                                             "PrimaryVertices",
                                             "InDetTrackParticles"]  

    # This variable is augmented by DStarSelectionTool
    STDM16SlimmingHelper.ExtraVariables += ["InDetTrackParticles.trackPassDstar",
                                            "AntiKt4EMPFlowJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.GhostPartons.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
                                            "TruthPrimaryVertices.t.x.y.z",
                                            "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                            "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
                                            "TauJets.dRmax.etOverPtLeadTrk",                                               "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                            "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                            "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]    

    # Needed for trigger objects
    
    STDM16SlimmingHelper.IncludeJetTriggerContent = True
    STDM16SlimmingHelper.IncludeMuonTriggerContent = True 
    STDM16SlimmingHelper.AllVariables = AllVariables
    STDM16SlimmingHelper.StaticContent = StaticContent
    STDM16ItemList = STDM16SlimmingHelper.GetItemList()
        
    acc.merge(OutputStreamCfg(flags, "DAOD_STDM16", ItemList=STDM16ItemList, AcceptAlgs=["STDM16Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_STDM16", AcceptAlgs=["STDM16Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
