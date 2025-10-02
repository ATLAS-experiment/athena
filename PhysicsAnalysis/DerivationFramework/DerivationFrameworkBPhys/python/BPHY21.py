# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY21.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY21"
streamName = "StreamDAOD_BPHY21"

def BPHY21Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)

    BPHY21_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
                        name = "BPHY21_AugOriginalCounts",
                        VertexContainer = "PrimaryVertices",
                        TrackContainer = "InDetTrackParticles")
    #====================================================================
    # TriggerCounting for Kernel1
    #====================================================================

    BPHY21_triggerList = [
                "HLT_2mu10",
                "HLT_2mu10_nomucomb",
                "HLT_2mu14",
                "HLT_2mu14_nomucomb",
                "HLT_mu18_mu8noL1",
                "HLT_mu18_nomucomb_mu8noL1",
                "HLT_mu20_mu8noL1",
                "HLT_mu20_nomucomb_mu8noL1",
                "HLT_mu22_mu8noL1",
                "HLT_mu22_nomucomb_mu8noL1",
                "HLT_mu20_mu8noL1",
                "HLT_mu24_mu8noL1",
                "HLT_mu10_mu6_bJpsimumu",
                "HLT_mu22_mu8noL1_calotag_0eta010_L1MU1"
               ]

    BPHY21_JpsiFinder = CompFactory.Analysis.JpsiFinder(
                         name                       = "BPHY21_JpsiFinder",
                         muAndMu                    = True,
                         muAndTrack                 = False,
                         TrackAndTrack              = False,
                         assumeDiMuons              = True, 
                         muonThresholdPt            = 2700,
                         invMassUpper               = 3400.0,
                         invMassLower               = 2800.0,
                         Chi2Cut                    = 10.,
                         oppChargesOnly             = True,
                         combOnly                   = True,
                         atLeastOneComb             = False,
                         useCombinedMeasurement     = False, # Only takes effect if combOnly=True    
                         muonCollectionKey          = "Muons",
                         TrackParticleCollection    = "InDetTrackParticles",
                         V0VertexFitterTool         = None,             # V0 vertex fitter
                         useV0Fitter                = False,                   # if False a TrkVertexFitterTool will be used
                         TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
                         TrackSelectorTool          = trackselect,
                         VertexPointEstimator       = vpest,
                         useMCPCuts                 = False)
    acc.addPublicTool(BPHY21_JpsiFinder)
    BPHY21_JpsiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY21_JpsiSelectAndWrite",
        VertexSearchTool       = BPHY21_JpsiFinder,
        OutputVtxContainerName = "BPHY21_JpsiCandidates",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        DoVertexType           = 1)
    BPHY21_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
        name                  = "BPHY21_Select_Jpsi2mumu",
        HypothesisName        = "Jpsi",
        InputVtxContainerName = "BPHY21_JpsiCandidates",
        V0Tools               = V0Tools,
        VtxMassHypo           = 3096.900,
        MassMin               = 2600.0,
        MassMax               = 3600.0,
        Chi2Max               = 200,
        LxyMin                = 0.1,
        DoVertexType          = 1)

    if not isSimulation: #Only Skim Data
        BPHY21_TriggerSkim = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "BPHY21_TriggerSkim",
                                                        TriggerListOR = BPHY21_triggerList)
        BPHY21_SelectJpsiEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
          name = "BPHY21_SelectJpsiEvent",
          expression = "count(BPHY21_JpsiCandidates.passed_Jpsi) > 0")

        BPHY21_SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR("BPHY21_SkimmingOR",
                 FilterList = [ BPHY21_TriggerSkim, BPHY21_SelectJpsiEvent] )
        acc.addPublicTool(BPHY21_SelectJpsiEvent)
        acc.addPublicTool(BPHY21_TriggerSkim)
        acc.addPublicTool(BPHY21_SkimmingOR)

    augTools = [BPHY21_JpsiSelectAndWrite, BPHY21_Select_Jpsi2mumu, BPHY21_AugOriginalCounts]
    for t in  augTools : acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY21Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = [BPHY21_SkimmingOR] if not isSimulation else [],
                                                    ThinningTools     = []))
    #====================================================================
    # Slimming 
    #====================================================================
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY21_SlimmingHelper = SlimmingHelper("BPHY21_SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY21_AllVariables  = getDefaultAllVariables()
    BPHY21_StaticContent = []
    
    # Needed for trigger objects
    BPHY21_SlimmingHelper.IncludeMuonTriggerContent  = True
    BPHY21_SlimmingHelper.IncludeBPhysTriggerContent = True
    
    ## primary vertices
    BPHY21_AllVariables  += ["PrimaryVertices"]
    BPHY21_StaticContent += ["xAOD::VertexContainer#BPHY21_RefittedPrimaryVertices"]
    BPHY21_StaticContent += ["xAOD::VertexAuxContainer#BPHY21_RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    BPHY21_AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    BPHY21_AllVariables += ["CombinedMuonTrackParticles"]
    BPHY21_AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    BPHY21_AllVariables += ["Muons"] 
    
    
    ## Jpsi candidates 
    BPHY21_StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY21_JpsiSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY21_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY21_JpsiSelectAndWrite.OutputVtxContainerName]
    
    
    # Tagging information (in addition to that already requested by usual algorithms)
    #AllVariables += ["GSFTrackParticles", "MuonSpectrometerTrackParticles" ] 
    
    # Added by ASC
    # Truth information for MC only
    if isSimulation:
        BPHY21_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]
    
    
    BPHY21_AllVariables = list(set(BPHY21_AllVariables)) # remove duplicates
    BPHY21_SlimmingHelper.AllVariables = BPHY21_AllVariables
    BPHY21_SlimmingHelper.StaticContent = BPHY21_StaticContent
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY21", ItemList=BPHY21_SlimmingHelper.GetItemList(), AcceptAlgs=["BPHY21Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY21", AcceptAlgs=["BPHY21Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
