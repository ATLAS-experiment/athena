# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY12.py
#====================================================================


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY12"
streamName = "StreamDAOD_BPHY12"
#====================================================================
# FLAGS TO PERSONALIZE THE DERIVATION
#====================================================================

skimTruth = False

def BPHY12Cfg(flags):
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
    BPHY12_Finder_DiMuon = CompFactory.Analysis.JpsiFinder(
                             name                        = "BPHY12_Finder_DiMuon",
                             muAndMu                     = True,
                             muAndTrack                  = False,
                             TrackAndTrack               = False,
                             assumeDiMuons               = True,
                             muonThresholdPt             = 3000.,
                             higherPt                    = 3500.,
                             invMassUpper                = 7000.,
                             invMassLower                = 1.,
                             Chi2Cut                     = 30.,
                             oppChargesOnly              = False,
                             allChargeCombinations       = True,                             
                             atLeastOneComb              = True,
                             useCombinedMeasurement      = False,
                             muonCollectionKey           = "Muons",
                             TrackParticleCollection     = "InDetTrackParticles",
                             V0VertexFitterTool          = None, 
                             useV0Fitter                 = False,
                             TrkVertexFitterTool         = vkalvrt, 
                             TrackSelectorTool           = trackselect,
                             VertexPointEstimator        = vpest,
                             useMCPCuts                  = False )
                             
    extraTools = [BPHY12_Finder_DiMuon]
    
    
                
    BPHY12_SelectAndWrite_DiMuon = CompFactory.DerivationFramework.Reco_Vertex(
                             name                   = "BPHY12_SelectAndWrite_DiMuon",
                             VertexSearchTool       = BPHY12_Finder_DiMuon,                             
                             OutputVtxContainerName = "BPHY12_DiMuon_Candidates",
                             PVContainerName        = "PrimaryVertices",
                             V0Tools                = V0Tools,  
                             PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                             RefPVContainerName     = "SHOULDNOTBEUSED",
                             DoVertexType           = 7)

    BPHY12_Select_DiMuons = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY12_Select_DiMuons",
      HypothesisName        = "Jpsi",
      V0Tools               = V0Tools,
      InputVtxContainerName = "BPHY12_DiMuon_Candidates",
      VtxMassHypo           = 3096.916,
      MassMax               = 10000.,      
      MassMin               = 0.,
      Chi2Max               = 1000.,
      DoVertexType          = 7
      )

    BPHY12_Finder_BdKstarKpiMuMu = CompFactory.Analysis.JpsiPlus2Tracks(
        name                    = "BPHY12_Finder_BdKstarKpiMuMu",
        kaonkaonHypothesis      = False,
        pionpionHypothesis      = False,
        kaonpionHypothesis      = True,
        oppChargesOnly          = False,
        SameChargesOnly         = False,                        
        trkThresholdPt          = 500.0,
        trkMaxEta               = 3.0, 
        DiTrackMassUpper        = 1110.,
        DiTrackMassLower        = 690.,
        DiTrackPt               = 500.,        
        FinalDiTrackPt          = 500.,
        TrkQuadrupletMassUpper  = 10000.0,
        TrkQuadrupletMassLower  = 1000.0, 
        BMassUpper              = 6500.0,
        BMassLower              = 3000.0,                         
        BThresholdPt            = 1000.,
        Chi2Cut                 = 30./5.,
        JpsiContainerKey        = "BPHY12_DiMuon_Candidates",
        TrackParticleCollection = "InDetTrackParticles",
        ExcludeCrossJpsiTracks  = False,   #setting this to False rejects the muons from J/psi candidate
        TrkVertexFitterTool     = vkalvrt,
        TrackSelectorTool       = trackselect,
        UseMassConstraint       = False,
        CandidateLimit          = 5000
        )
    
    BPHY12_SelectAndWrite_BdKstarKpiMuMu  = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY12_SelectAndWrite_BdKstarKpiMuMu",
        VertexSearchTool       = BPHY12_Finder_BdKstarKpiMuMu,
        OutputVtxContainerName = "BPHY12_BdKstarKpiMuMu_Candidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "BPHY12_BdKstarKpiMuMu_refitPV",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        RefitPV                = True,
        MaxPVrefit             = 10000,
        DoVertexType           = 7
        )

    BPHY12_Select_BdKstarKpiMuMu = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY12_Select_BdKstarKpiMuMu",
        V0Tools                    = V0Tools,
        HypothesisName             = "Bd",
        InputVtxContainerName      = "BPHY12_BdKstarKpiMuMu_Candidates",
        TrkMasses                  = [105.658, 105.658, 493.677, 139.570],
        VtxMassHypo                = 5279.6,
        MassMax                    = 10000.,                 
        MassMin                    = 0.,     
        Chi2Max                    = 1000.)
    
    BPHY12_Select_BdKstarKpiMuMu_anti = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY12_Select_BdKstarKpiMuMu_anti",
        V0Tools                    = V0Tools,
        HypothesisName             = "Bdbar",
        InputVtxContainerName      = "BPHY12_BdKstarKpiMuMu_Candidates",
        TrkMasses                  = [105.658, 105.658, 139.570, 493.677],
        VtxMassHypo                = 5279.6,
        MassMax                    = 10000.,
        MassMin                    = 0.,     
        Chi2Max                    = 1000.)  
    
    BPHY12_ReVertex_Kstar = CompFactory.DerivationFramework.ReVertex(
                           name                   = "BPHY12_ReVertex_Kstar",
                           InputVtxContainerName  = "BPHY12_BdKstarKpiMuMu_Candidates",
                           V0Tools                = V0Tools,
                           PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                           TrackIndices           = [ 2, 3 ],
                           TrkVertexFitterTool    = vkalvrt,
                           OutputVtxContainerName = "BPHY12_Kstar_ReVertexCandidates"
                           )
    
    BPHY12_Select_KstarKpi = CompFactory.DerivationFramework.Select_onia2mumu(
                          name                  = "BPHY12_Select_KstarKpi",
                          HypothesisName        = "Kstar",
                          InputVtxContainerName = "BPHY12_Kstar_ReVertexCandidates",
                          V0Tools               = V0Tools,
                          TrkMasses             = [ 493.677, 139.570 ],
                          VtxMassHypo           = 891.66,
                          MassMin               = 0.,
                          MassMax               = 10000.,
                          Chi2Max               = 1000.
                          )
    
    BPHY12_Select_KstarKpi_anti = CompFactory.DerivationFramework.Select_onia2mumu(
                       name                  = "BPHY12_Select_KstarKpi_anti",
                       HypothesisName        = "Kstarbar",
                       InputVtxContainerName = "BPHY12_Kstar_ReVertexCandidates",
                       V0Tools               = V0Tools,
                       TrkMasses             = [ 139.570, 493.677 ],
                       VtxMassHypo           = 891.66,
                       MassMin               = 0.,
                       MassMax               = 10000.,
                       Chi2Max               = 1000.
                       )

    
    BPHY12_SelectBmumuKstEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
                       name = "BPHY12_SelectBmumuKstEvent",
                       expression = "(count(BPHY12_BdKstarKpiMuMu_Candidates.passed_Bd > 0) + count(BPHY12_BdKstarKpiMuMu_Candidates.passed_Bdbar > 0)) > 0")
          
    extraTools += [BPHY12_SelectBmumuKstEvent]
    
    if skimTruth or not isSimulation: #Only Skim Data
        filterlist = [BPHY12_SelectBmumuKstEvent]
        for t in  filterlist : acc.addPublicTool(t)
        #====================================================================
        # Make event selection based on an OR of the input skimming tools (though it seems we only have one here!)
        #====================================================================
        BPHY12SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR(
            name       = "BPHY12SkimmingOR",
            FilterList = filterlist) #OR of all your different filters

    BPHY12Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
            name                       = "BPHY12Thin_vtxTrk",
            TrackParticleContainerName = "InDetTrackParticles",
            StreamName = streamName,
            VertexContainerNames       = ["BPHY12_BdKstarKpiMuMu_Candidates"],
            PassFlags                  = ["passed_Bd", "passed_Bdbar"] )
            
    BPHY12MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name    = "BPHY12MuonTPThinningTool",
                                                                         StreamName = streamName,
                                                                         MuonKey                 = "Muons",
                                                                         InDetTrackParticlesKey  = "InDetTrackParticles")
    
    BPHY12_thinningTool_PV = CompFactory.DerivationFramework.BPhysPVThinningTool(
                                name                 = "BPHY12_thinningTool_PV",
                                StreamName = streamName,
                                CandidateCollections = ["BPHY12_BdKstarKpiMuMu_Candidates"],
                                KeepPVTracks         = True
                                )
    
    BPHY12TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "BPHY12TruthThinTool",
                                                        ParticleSelectionString = "TruthParticles.pdgId == 511 || TruthParticles.pdgId == -511 || TruthParticles.pdgId == 531 || TruthParticles.pdgId == -531",
                                                        PreserveDescendants     = True,
                                                        PreserveAncestors      = True)

    BPHY12ThinningTools = [BPHY12Thin_vtxTrk, BPHY12_thinningTool_PV, BPHY12MuonTPThinningTool]
    if isSimulation:
       BPHY12ThinningTools.append(BPHY12TruthThinTool)

    augTools = [BPHY12_SelectAndWrite_DiMuon, BPHY12_Select_DiMuons,
                BPHY12_SelectAndWrite_BdKstarKpiMuMu, BPHY12_Select_BdKstarKpiMuMu, BPHY12_Select_BdKstarKpiMuMu_anti,
                BPHY12_ReVertex_Kstar, BPHY12_Select_KstarKpi, BPHY12_Select_KstarKpi_anti]
    skimTools = [BPHY12SkimmingOR] if skimTruth or not isSimulation else []


    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
         "BPHY12Kernel",
          AugmentationTools = augTools,
          SkimmingTools     = skimTools,
          ThinningTools     = BPHY12ThinningTools
          ))

    for t in  augTools + skimTools + BPHY12ThinningTools + extraTools: acc.addPublicTool(t)
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY12SlimmingHelper = SlimmingHelper("BPHY12SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []

    # Needed for trigger objects
    BPHY12SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY12SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY12_BdKstarKpiMuMu_refitPV"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY12_BdKstarKpiMuMu_refitPVAux."]

    ## ID track particles
    AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]

    ## muon container
    AllVariables += ["Muons"]

    ## Jpsi candidates
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY12_SelectAndWrite_DiMuon.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY12_SelectAndWrite_DiMuon.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY12_SelectAndWrite_BdKstarKpiMuMu.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY12_SelectAndWrite_BdKstarKpiMuMu.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY12_ReVertex_Kstar.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY12_ReVertex_Kstar.OutputVtxContainerName]
    
    # Added by ASC
    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY12SlimmingHelper.AllVariables = AllVariables
    BPHY12SlimmingHelper.StaticContent = StaticContent
    BPHY12ItemList = BPHY12SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY12", ItemList=BPHY12ItemList, AcceptAlgs=["BPHY12Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY12", AcceptAlgs=["BPHY12Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
