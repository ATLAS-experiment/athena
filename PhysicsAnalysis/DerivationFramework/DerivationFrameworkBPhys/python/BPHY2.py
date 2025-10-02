# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY2.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY2"
streamName = "StreamDAOD_BPHY2"


def BPHY2Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    BPHY2_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
       name = "BPHY2_AugOriginalCounts",
       VertexContainer = "PrimaryVertices",
       TrackContainer = "InDetTrackParticles" )
    isSimulation = flags.Input.isMC
 
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    BPHY2JpsiFinder = CompFactory.Analysis.JpsiFinder(name           = "BPHY2JpsiFinder",
                                         muAndMu                     = True,
                                         muAndTrack                  = False,
                                         TrackAndTrack               = False,
                                         assumeDiMuons               = True,
                                         invMassUpper                = 4700.0,
                                         invMassLower                = 2600.0,
                                         Chi2Cut                     = 15.,
                                         oppChargesOnly              = True,
                                         combOnly                = True,
                                         atLeastOneComb              = False,
                                         useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                                         muonCollectionKey           = "Muons",
                                         TrackParticleCollection     = "InDetTrackParticles",
                                         V0VertexFitterTool          = None,             # V0 vertex fitter
                                         useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
                                         TrkVertexFitterTool         = vkalvrt,        # VKalVrt vertex fitter
                                         TrackSelectorTool           = trackselect,
                                         VertexPointEstimator        = vpest,
                                         useMCPCuts                  = False)

    BPHY2JpsiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name           = "BPHY2JpsiSelectAndWrite",
                                                        VertexSearchTool       = BPHY2JpsiFinder,
                                                        OutputVtxContainerName = "BPHY2JpsiCandidates",
                                                        PVContainerName        = "PrimaryVertices",
                                                        RefPVContainerName     = "SHOULDNOTBEUSED",
                                                        V0Tools                = V0Tools,
                                                        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                                        DoVertexType           =1)
 
    BPHY2BsJpsiKK = CompFactory.Analysis.JpsiPlus2Tracks(name = "BPHY2BsJpsiKK",
                                         kaonkaonHypothesis      = True,
                                         pionpionHypothesis      = False,
                                         kaonpionHypothesis      = False,
                                         trkThresholdPt          = 800.0,
                                         trkMaxEta           = 3.0,
                                         BMassUpper            = 5800.0,
                                         BMassLower            = 5000.0,
                                         DiTrackMassUpper = 1019.445 + 100.,
                                         DiTrackMassLower = 1019.445 - 100.,
                                         Chi2Cut                     = 8.0,
                                         TrkQuadrupletMassUpper      = 6000.0,
                                         TrkQuadrupletMassLower      = 4800.0,
                                         JpsiContainerKey        = "BPHY2JpsiCandidates",
                                         TrackParticleCollection = "InDetTrackParticles",
                                         MuonsUsedInJpsi         = "Muons",
                                         TrkVertexFitterTool     = vkalvrt,
                                         TrackSelectorTool       = trackselect,
                                         UseMassConstraint       = False)

    BPHY2BsKKSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name   = "BPHY2BsKKSelectAndWrite",
                                                           VertexSearchTool       = BPHY2BsJpsiKK,
                                                           OutputVtxContainerName   = "BPHY2BsJpsiKKCandidates",
                                                           PVContainerName          = "PrimaryVertices",
                                                           RefPVContainerName       = "BPHY2RefittedPrimaryVertices",
                                                           RefitPV                  = True,
                                                           V0Tools                  = V0Tools,
                                                           PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                                           MaxPVrefit               = 10000, DoVertexType = 7)

    BPHY2_Select_Psi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY2_Select_Psi2mumu",
      HypothesisName        = "Psi",
      InputVtxContainerName = "BPHY2JpsiCandidates",
      V0Tools               = V0Tools,
      VtxMassHypo           = 3686.09,
      MassMin               = 3300.0,
      MassMax               = 4500.0,
      Chi2Max               = 200,
      DoVertexType          = 7)
 
    BPHY2_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY2_Select_Jpsi2mumu",
      HypothesisName        = "Jpsi",
      InputVtxContainerName = "BPHY2JpsiCandidates",
      V0Tools               = V0Tools,
      VtxMassHypo           = 3096.916,
      MassMin               = 2000.0,
      MassMax               = 3600.0,
      Chi2Max               = 200,
      DoVertexType          = 7)
 
    BPHY2_Select_Bs2JpsiKK = CompFactory.DerivationFramework.Select_onia2mumu(
      name                       = "BPHY2_Select_Bs2JpsiKK",
      HypothesisName             = "Bs",
      InputVtxContainerName      = "BPHY2BsJpsiKKCandidates",
      V0Tools                    = V0Tools,
      TrkMasses                  = [105.658, 105.658, 493.677, 493.677],
      VtxMassHypo                = 5366.3,
      MassMin                    = 5000.0,
      MassMax                    = 5800.0,
      Chi2Max                    = 200)
 
    #Thinning tools
 
    BPHY2_thinningTool_Tracks = CompFactory.DerivationFramework.Thin_vtxTrk(
      name                       = "BPHY2_thinningTool_Tracks",
      TrackParticleContainerName = "InDetTrackParticles",
      StreamName = streamName,
      VertexContainerNames       = ["BPHY2BsJpsiKKCandidates"],
      PassFlags                  = ["passed_Bs"] )
 
 
    BPHY2_thinningTool_TracksPsi = CompFactory.DerivationFramework.Thin_vtxTrk(
      name                       = "BPHY2_thinningTool_TracksPsi",
      TrackParticleContainerName = "InDetTrackParticles",
      StreamName = streamName,
      VertexContainerNames       = ["BPHY2JpsiCandidates"],
      PassFlags                  = ["passed_Psi", "passed_Jpsi"] )
 
    BPHY2_thinningTool_PV = CompFactory.DerivationFramework.BPhysPVThinningTool(
       name                       = "BPHY2_thinningTool_PV",
       CandidateCollections       = ["BPHY2BsJpsiKKCandidates"],
       StreamName = streamName,
       KeepPVTracks  =True)
  
    BPHY2MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name     = "BPHY2MuonTPThinningTool",
                                                                         MuonKey                 = "Muons",
                                                                         StreamName = streamName,
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
    if not isSimulation: #Only Skim Data
       BPHY2_SelectBsJpsiKKEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
       name = "BPHY2_SelectBsJpsiKKEvent",
       expression = "count(BPHY2BsJpsiKKCandidates.passed_Bs > 0) > 0")
 
       #====================================================================
       # Make event selection based on an OR of the input skimming tools
       #====================================================================
       BPHY2SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR("BPHY2SkimmingOR",
                                                                    FilterList = [BPHY2_SelectBsJpsiKKEvent ])
       acc.addPublicTool(BPHY2SkimmingOR)


    thiningCollection = [BPHY2_thinningTool_Tracks, BPHY2_thinningTool_TracksPsi, BPHY2_thinningTool_PV, BPHY2MuonTPThinningTool]
    augCollection = [ BPHY2JpsiSelectAndWrite, BPHY2BsKKSelectAndWrite, BPHY2_Select_Psi2mumu, BPHY2_Select_Jpsi2mumu, BPHY2_Select_Bs2JpsiKK, BPHY2_AugOriginalCounts]
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY2Kernel",
                                                     AugmentationTools = augCollection,
                                                     #Only skim if not MC
                                                     SkimmingTools     = [BPHY2SkimmingOR] if not isSimulation else [],
                                                     ThinningTools     = thiningCollection))
 
    for t in  augCollection + thiningCollection : acc.addPublicTool(t)
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY2SlimmingHelper = SlimmingHelper("BPHY2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
     
    # Needed for trigger objects
    BPHY2SlimmingHelper.IncludeMuonTriggerContent  = True
    BPHY2SlimmingHelper.IncludeBPhysTriggerContent = True
    SmartVar = []
    ## primary vertices
    SmartVar  += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY2RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY2RefittedPrimaryVerticesAux."]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    
    ## muon container
    SmartVar += ["Muons"]
    
    ## Jpsi candidates
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY2JpsiSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY2JpsiSelectAndWrite.OutputVtxContainerName]
    
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY2BsKKSelectAndWrite.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY2BsKKSelectAndWrite.OutputVtxContainerName]
    
    # Tagging information (in addition to that already requested by usual algorithms)
    AllVariables += ["MuonSpectrometerTrackParticles" ]

    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles" ]

    BPHY2SlimmingHelper.AllVariables = AllVariables
    BPHY2SlimmingHelper.StaticContent = StaticContent
    BPHY2SlimmingHelper.SmartCollections = SmartVar
    BPHY2ItemList = BPHY2SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY2", ItemList=BPHY2ItemList, AcceptAlgs=["BPHY2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY2", AcceptAlgs=["BPHY2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc

