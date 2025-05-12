# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY16.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY16"
streamName = "StreamDAOD_BPHY16"

def BPHY16Cfg(flags):
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
    BPHY16JpsiFinder = CompFactory.Analysis.JpsiFinder(
          name                        = "BPHY16JpsiFinder",
          muAndMu                     = True,
          muAndTrack                  = False,
          TrackAndTrack               = False,
          assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
          invMassUpper                = 12000.0,
          invMassLower                = 8000.,
          Chi2Cut                     = 20.,
          oppChargesOnly              = True,
          atLeastOneComb              = True,
          useCombinedMeasurement      = False, # Only takes effect if combOnly=True
          muonCollectionKey           = "Muons",
          TrackParticleCollection     = "InDetTrackParticles",
          V0VertexFitterTool          = None, # V0 vertex fitter
          useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
          TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
          TrackSelectorTool           = trackselect,
          VertexPointEstimator        = vpest,
          useMCPCuts                  = False )
    acc.addPublicTool(BPHY16JpsiFinder)
    BPHY16_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
          name                   = "BPHY16_Reco_mumu",
          VertexSearchTool       = BPHY16JpsiFinder,
          OutputVtxContainerName = "BPHY16OniaCandidates",
          PVContainerName        = "PrimaryVertices",
          RefPVContainerName     = "BPHY16RefittedPrimaryVertices1",
          RefitPV                = True,
          V0Tools                = V0Tools,
          PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
          MaxPVrefit             = 100000,
          DoVertexType           = 7)

    BPHY16_Select_Upsi = CompFactory.DerivationFramework.Select_onia2mumu(
          name                  = "BPHY16_Select_Upsi",
          HypothesisName        = "Upsilon",
          InputVtxContainerName = "BPHY16OniaCandidates",
          V0Tools               = V0Tools,
          VtxMassHypo           = 9460.30,
          MassMin               = 8000.,
          MassMax               = 12000.,
          Chi2Max               = 200,
          DoVertexType          = 7)

    BPHY16Plus2Tracks = CompFactory.Analysis.JpsiPlus2Tracks(name = "BPHY16Plus2Tracks",
         kaonkaonHypothesis          = False,
         pionpionHypothesis          = False,
         kaonpionHypothesis          = False,
         ManualMassHypo              = [ 105.658, 105.658, 105.658, 105.658 ],
         trkThresholdPt              = 0.0,
         trkMaxEta                   = 3.0,
         BMassUpper                  = 50000.0,
         BMassLower                  = 0,
         oppChargesOnly              = False,
         Chi2Cut                     = 100.0,
         JpsiContainerKey            = "BPHY16OniaCandidates",
         TrackParticleCollection     = "InDetTrackParticles",
         MuonsUsedInJpsi             = "Muons",
         ExcludeJpsiMuonsOnly        = True,
         RequireNMuonTracks          = 1,
         TrkVertexFitterTool         = vkalvrt,
         TrackSelectorTool           = trackselect,
         UseMassConstraint           = False)

    BPHY16FourTrackSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name = "BPHY16FourTrackSelectAndWrite",
                                      VertexSearchTool         = BPHY16Plus2Tracks,
                                      OutputVtxContainerName   = "BPHY16FourTrack",
                                      PVContainerName          = "PrimaryVertices",
                                      RefPVContainerName       = "BPHY16RefittedPrimaryVertices2",
                                      RefitPV                  = True,
                                      V0Tools                  = V0Tools,
                                      PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                      MaxPVrefit               = 10000, DoVertexType = 7)

    BPHY16_Select_FourTrack  = CompFactory.DerivationFramework.Select_onia2mumu(
                                      name                       = "BPHY16_Select_FourTracks",
                                      HypothesisName             = "FourTracks",
                                      InputVtxContainerName      = "BPHY16FourTrack",
                                      V0Tools                    = V0Tools,
                                      TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
                                      VtxMassHypo                = 18100.0,
                                      MassMin                    = 0,
                                      MassMax                    = 500000,
                                      Chi2Max                    = BPHY16Plus2Tracks.Chi2Cut)
    BPHY16_Revertex   = CompFactory.DerivationFramework.ReVertex(
                                      name                       = "BPHY16_ReVertex",
                                      InputVtxContainerName      = "BPHY16FourTrack",
                                      V0Tools                    = V0Tools,
                                      TrackIndices               = [ 2, 3 ],
                                      PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                      TrkVertexFitterTool        = vkalvrt,
                                      OutputVtxContainerName     = "BPHY16TwoTrack")
    BPHY16_Select_TwoTrack  = CompFactory.DerivationFramework.Select_onia2mumu(
                                  name                       = "BPHY16_Select_TwoTracks",
                                  HypothesisName             = "TwoTracks",
                                  InputVtxContainerName      = "BPHY16TwoTrack",
                                  V0Tools                    = V0Tools,
                                  TrkMasses                  = [105.658, 105.658],
                                  VtxMassHypo                = 18100.0,
                                  MassMin                    = 1,
                                  MassMax                    = 500000,
                                  Chi2Max                    = 90)

    BPHY16_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY16_SelectEvent",
                                            expression = "count(BPHY16FourTrack.passed_FourTracks) > 0")


    augTools = [BPHY16_Reco_mumu, BPHY16_Select_Upsi, BPHY16FourTrackSelectAndWrite, BPHY16_Select_FourTrack, BPHY16_Revertex, BPHY16_Select_TwoTrack]
    skimTools = [BPHY16_SelectEvent]
    for t in  augTools +skimTools : acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY16Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = skimTools,
                                                    ThinningTools     = []))

    #====================================================================
    # Slimming 
    #====================================================================
    
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY16SlimmingHelper = SlimmingHelper("BPHY16SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent = []
    
    # Needed for trigger objects
    BPHY16SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY16SlimmingHelper.IncludeBPhysTriggerContent = True
    
    ## primary vertices
    AllVariables += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY16RefittedPrimaryVertices1"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY16RefittedPrimaryVertices1Aux."]
    StaticContent += ["xAOD::VertexContainer#BPHY16RefittedPrimaryVertices2"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY16RefittedPrimaryVertices2Aux."]
    
    ## ID track particles
    AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    AllVariables += ["CombinedMuonTrackParticles"]
    AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    AllVariables += ["Muons"]
    
    
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY16_Reco_mumu.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY16_Reco_mumu.OutputVtxContainerName]
    
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY16FourTrackSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY16FourTrackSelectAndWrite.OutputVtxContainerName]
    
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY16_Revertex.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY16_Revertex.OutputVtxContainerName]
    
    
    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]
    BPHY16SlimmingHelper.AllVariables = AllVariables
    BPHY16SlimmingHelper.StaticContent = StaticContent
    BPHY16ItemList = BPHY16SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY16", ItemList=BPHY16ItemList, AcceptAlgs=["BPHY16Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY16", AcceptAlgs=["BPHY16Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
