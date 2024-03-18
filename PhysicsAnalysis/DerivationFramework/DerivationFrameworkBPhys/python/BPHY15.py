# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY15.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY15"
streamName = "StreamDAOD_BPHY15"

def BPHY15Cfg(flags):
   from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
   from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
   acc = ComponentAccumulator()
   V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
   vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
   acc.addPublicTool(vkalvrt)
   acc.addPublicTool(V0Tools)
   trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
   acc.addPublicTool(trackselect)
   vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
   acc.addPublicTool(vpest)
   PVrefit = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
   acc.addPublicTool(PVrefit)
   isSimulation = flags.Input.isMC
   BPHY15_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
                                          name = "BPHY15_AugOriginalCounts",
                                          VertexContainer = "PrimaryVertices",
                                          TrackContainer = "InDetTrackParticles" )
   BPHY15JpsiFinder = CompFactory.Analysis.JpsiFinder(
       name                       = "BPHY15JpsiFinder",
       muAndMu                    = True,
       muAndTrack                 = False,
       TrackAndTrack              = False,
       assumeDiMuons              = True, 
       muonThresholdPt            = 2700,
       invMassUpper               = 3400.0,
       invMassLower               = 2800.0,
       Chi2Cut                    = 10.,
       oppChargesOnly             = True,
       allMuons                   = True,
       combOnly                   = False,
       atLeastOneComb             = False,
       useCombinedMeasurement     = False, # Only takes effect if combOnly=True  
       muonCollectionKey          = "Muons",
       TrackParticleCollection    = "InDetTrackParticles",
       V0VertexFitterTool         = None,             # V0 vertex fitter
       useV0Fitter                = False,
       TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
       TrackSelectorTool          = trackselect,
       VertexPointEstimator       = vpest,
       useMCPCuts                 = False)
   acc.addPublicTool(BPHY15JpsiFinder)
   BPHY15JpsiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
       name                   = "BPHY15JpsiSelectAndWrite",
       VertexSearchTool       = BPHY15JpsiFinder,
       OutputVtxContainerName = "BPHY15JpsiCandidates",
       V0Tools                = V0Tools,
       PVRefitter             = PVrefit,
       PVContainerName        = "PrimaryVertices",
       RefPVContainerName     = "SHOULDNOTBEUSED",
       DoVertexType           = 1)

   BPHY15_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_Jpsi2mumu",
       HypothesisName        = "Jpsi",
       InputVtxContainerName = "BPHY15JpsiCandidates",
       V0Tools               = V0Tools,
       VtxMassHypo           = 3096.900,
       MassMin               = 2600.0,
       MassMax               = 3600.0,
       Chi2Max               = 200,
       LxyMin                = 0.1,
       DoVertexType          = 1)

   BPHY15BcJpsipi = CompFactory.Analysis.JpsiPlus1Track(
       name                    = "BPHY15BcJpsipi",
       pionHypothesis          = True,
       kaonHypothesis       = False,
       trkThresholdPt       = 2700,
       trkMaxEta               = 2.7,
       BThresholdPt            = 100.0,
       BMassUpper             = 6900.0,
       BMassLower             = 5600.0,
       JpsiContainerKey        = "BPHY15JpsiCandidates",
       TrackParticleCollection = "InDetTrackParticles",
       MuonsUsedInJpsi         = "Muons",
       TrkVertexFitterTool     = vkalvrt,
       TrackSelectorTool       = trackselect,
       UseMassConstraint       = True, 
       Chi2Cut                = 5,
       TrkTrippletMassUpper    = 6900,
       TrkTrippletMassLower    = 5600)
   acc.addPublicTool(BPHY15BcJpsipi)
   BPHY15BcJpsipiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
      name                   = "BPHY15BcJpsipiSelectAndWrite",
      VertexSearchTool       = BPHY15BcJpsipi,
      V0Tools                = V0Tools,
      PVRefitter             = PVrefit,
      OutputVtxContainerName = "BPHY15BcJpsipiCandidates",
      PVContainerName        = "PrimaryVertices",
      RefPVContainerName     = "BPHY15RefittedPrimaryVertices1",
      RefitPV                = True,
      MaxPVrefit             = 1000)
   BPHY15_Select_Bc2Jpsipi = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY15_Select_Bc2Jpsipi",
      HypothesisName        = "Bc",
      InputVtxContainerName = "BPHY15BcJpsipiCandidates",
      V0Tools               = V0Tools,
      TrkMasses             = [105.658, 105.658, 139.571],
      VtxMassHypo           = 6274.9,
      MassMin               = 5600.0,
      MassMax               = 6900.0,
      Chi2Max               = 200)
   BPHY15JpsipiFinder = CompFactory.Analysis.JpsiPlus1Track(
      name                    = "BPHY15JpsipiFinder",
      pionHypothesis          = True,
      kaonHypothesis       = False,
      trkThresholdPt       = 350.0,
      trkMaxEta               = 2.7,
      BThresholdPt            = 5000.0,
      BMassUpper             = 3600.0,
      BMassLower             = 3200.0,
      TrkDeltaZ               = 20.,
      TrkQuadrupletPt         = 5000,
      JpsiContainerKey        = "BPHY15JpsiCandidates",
      TrackParticleCollection = "InDetTrackParticles",
      MuonsUsedInJpsi         = "Muons",
      TrkVertexFitterTool     = vkalvrt,
      TrackSelectorTool       = trackselect,
      UseMassConstraint       = True, 
      Chi2Cut                = 5,
      TrkTrippletMassUpper    = 3600,
      TrkTrippletMassLower    = 3200)
   acc.addPublicTool(BPHY15JpsipiFinder)

   BPHY15JpsipiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
      name                   = "BPHY15JpsipiSelectAndWrite",
      VertexSearchTool       = BPHY15JpsipiFinder,
      PVRefitter             = PVrefit,
      V0Tools                = V0Tools,
      OutputVtxContainerName = "BPHY15JpsipiCandidates",
      PVContainerName        = "PrimaryVertices",
      RefPVContainerName     = "SHOULDNOTBEUSED",
      MaxPVrefit       = 1000)

   BPHY15_Select_Jpsipi = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_Jpsipi",
       HypothesisName        = "Jpsipi",
       V0Tools               = V0Tools,
       TrkMasses             = [105.658, 105.658, 139.571],
       InputVtxContainerName = "BPHY15JpsipiCandidates",
       VtxMassHypo           = 3396.900,
       MassMin               = 3200.0,
       MassMax               = 3600.0,
       Chi2Max               = 200,
       LxyMin                = 0.1,
       DoVertexType          = 1)

   BPHY15DiTrkFinder = CompFactory.Analysis.JpsiFinder(
       name                       = "BPHY15DiTrkFinder",
       muAndMu                    = False,
       muAndTrack                 = False,
       TrackAndTrack              = True,
       assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
       trackThresholdPt           = 900,
       invMassUpper               = 1900.0,
       invMassLower               = 280.0,
       Chi2Cut                    = 10.,
       oppChargesOnly          = True,
       atLeastOneComb             = False,
       useCombinedMeasurement     = False, # Only takes effect if combOnly=True  
       muonCollectionKey          = "Muons",
       TrackParticleCollection    = "InDetTrackParticles",
       V0VertexFitterTool         = None,
       useV0Fitter                = False,
       TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
       TrackSelectorTool          = trackselect,
       VertexPointEstimator       = vpest,
       useMCPCuts                 = False,
       track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
       track2Mass                 = 139.571)
   acc.addPublicTool(BPHY15DiTrkFinder)
   BPHY15DiTrkSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
       name                   = "BPHY15DiTrkSelectAndWrite",
       VertexSearchTool       = BPHY15DiTrkFinder,
       OutputVtxContainerName = "BPHY15DiTrkCandidates",
       PVContainerName        = "PrimaryVertices",
       V0Tools                = V0Tools,
       PVRefitter             = PVrefit,
       RefPVContainerName     = "SHOULDNOTBEUSED",
       CheckCollections       = True,
       CheckVertexContainers  = ['BPHY15JpsiCandidates'],
       DoVertexType           = 1)

   BPHY15_Select_D0 = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_D0",
       HypothesisName        = "D0",
       InputVtxContainerName = "BPHY15DiTrkCandidates",
       V0Tools               = V0Tools,
       TrkMasses             = [139.571, 493.677],
       VtxMassHypo           = 1864.83,
       MassMin               = 1864.83-170,
       MassMax               = 1864.83+170,
       LxyMin                = 0.15,
       Chi2Max               = 200)

   BPHY15_Select_D0b = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_D0b",
       HypothesisName        = "D0b",
       InputVtxContainerName = "BPHY15DiTrkCandidates",
       V0Tools               = V0Tools,
       TrkMasses             = [493.677, 139.571],
       VtxMassHypo           = 1864.83,
       MassMin               = 1864.83-170,
       MassMax               = 1864.83+170,
       LxyMin                = 0.15,
       Chi2Max               = 200)

   BPHY15Dh3Finder = CompFactory.Analysis.JpsiPlus1Track(
        name                    = "BPHY15Dh3Finder",
        pionHypothesis          = True,
        kaonHypothesis          = False,
        trkThresholdPt          = 900.0,
        trkMaxEta               = 2.7, # is this value fine?? default would be 102.5
        BThresholdPt            = 2000.0,
        BMassUpper              = 1800.0, # What is this??
        BMassLower              = 500.0,
        TrkDeltaZ               = 20.,
        TrkTrippletMassUpper    = 1800,
        TrkTrippletMassLower    = 500,
        TrkQuadrupletPt         = 2000,
        JpsiContainerKey        = "BPHY15DiTrkCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi         = "NONE", # ?
        ExcludeCrossJpsiTracks  = False,
        TrkVertexFitterTool     = vkalvrt,
        TrackSelectorTool       = trackselect,
        UseMassConstraint       = False, 
        Chi2Cut                 = 5) #Cut on chi2/Ndeg_of_freedom
   acc.addPublicTool(BPHY15Dh3Finder)
   BPHY15Dh3SelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
       name                   = "BPHY15Dh3SelectAndWrite",
       VertexSearchTool       = BPHY15Dh3Finder,
       V0Tools                = V0Tools,
       PVRefitter             = PVrefit,
       OutputVtxContainerName = "BPHY15Dh3Candidates",
       PVContainerName        = "PrimaryVertices",
       RefPVContainerName     = "SHOULDNOTBEUSED",
       MaxPVrefit             = 1000)

   BPHY15_Select_Ds = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_Ds",
       HypothesisName        = "Ds",
       V0Tools               = V0Tools,
       TrkMasses             = [493.677, 493.677, 139.571],
       InputVtxContainerName = "BPHY15Dh3Candidates",
       VtxMassHypo           = 1968.28,
       MassMin               = 1968.28-200,
       MassMax               = 1968.28+200,
       Chi2Max               = 200,
       LxyMin                = 0.1,
       DoVertexType          = 1)

   BPHY15_Select_Dp = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_Dp",
       HypothesisName        = "Dp",
       V0Tools               = V0Tools,
       TrkMasses             = [139.571, 493.677, 139.571],
       InputVtxContainerName = "BPHY15Dh3Candidates",
       VtxMassHypo           = 1869.59,
       MassMin               = 1869.59-200,
       MassMax               = 1869.59+200,
       Chi2Max               = 200,
       LxyMin                = 0.1,
       DoVertexType          = 1)

   BPHY15_Select_Dm = CompFactory.DerivationFramework.Select_onia2mumu(
      name                  = "BPHY15_Select_Dm",
      HypothesisName        = "Dm",
      V0Tools               = V0Tools,
      TrkMasses             = [493.677, 139.571, 139.571],
      InputVtxContainerName = "BPHY15Dh3Candidates",
      VtxMassHypo           = 1869.59,
      MassMin               = 1869.59-200,
      MassMax               = 1869.59+200,
      Chi2Max               = 200,
      LxyMin                = 0.1,
      DoVertexType          = 1)

   BcJpsiDxVertexFit = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName + "BcJpsiDx", CascadeCnstPrecision = 1e-6))
   acc.addPublicTool(BcJpsiDxVertexFit)
   BPHY15JpsiDs = CompFactory.DerivationFramework.JpsiPlusDsCascade(
       name                     = "BPHY15JpsiDs",
       HypothesisName           = "Bc",
       V0Tools                  = V0Tools,
       TrkVertexFitterTool      = BcJpsiDxVertexFit,
       PVRefitter               = PVrefit,
       DxHypothesis             = 431,
       ApplyDxMassConstraint    = True,
       ApplyJpsiMassConstraint  = True,
       JpsiMassLowerCut         = 2600.,
       JpsiMassUpperCut         = 3600.,
       DxMassLowerCut           = 1968.28 - 200.,
       DxMassUpperCut           = 1968.28 + 200.,
       MassLowerCut             = 6274.90 - 600.,
       MassUpperCut             = 6274.90 + 600.,
       Chi2Cut                  = 10,
       RefitPV                  = True,
       RefPVContainerName       = "BPHY15RefittedPrimaryVertices2",
       JpsiVertices             = "BPHY15JpsiCandidates",
       CascadeVertexCollections = ["BcJpsiDsCascadeSV2", "BcJpsiDsCascadeSV1"],
       DxVertices               = "BPHY15Dh3Candidates")

   BPHY15JpsiDp = CompFactory.DerivationFramework.JpsiPlusDsCascade(
       name                     = "BPHY15JpsiDp",
       HypothesisName           = "Bc",
       V0Tools                  = V0Tools,
       TrkVertexFitterTool      = BcJpsiDxVertexFit,
       PVRefitter               = PVrefit,
       DxHypothesis             = 411,
       ApplyDxMassConstraint    = True,
       ApplyJpsiMassConstraint  = True,
       JpsiMassLowerCut         = 2600.,
       JpsiMassUpperCut         = 3600.,
       DxMassLowerCut           = 1869.59 - 180.,
       DxMassUpperCut           = 1869.59 + 180.,
       MassLowerCut             = 6274.90 - 600.,
       MassUpperCut             = 6274.90 + 600.,
       Chi2Cut                  = 10,
       RefitPV                  = True,
       RefPVContainerName       = "BPHY15RefittedPrimaryVertices3",
       JpsiVertices             = "BPHY15JpsiCandidates",
       CascadeVertexCollections = ["BcJpsiDpCascadeSV2", "BcJpsiDpCascadeSV1"],
       DxVertices               = "BPHY15Dh3Candidates")

   BcJpsiDstVertexFit = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName + "BcJpsiDst", CascadeCnstPrecision = 1e-6))
   acc.addPublicTool(BcJpsiDstVertexFit)

   BPHY15JpsiDpst = CompFactory.DerivationFramework.JpsiPlusDpstCascade(
       name                     = "BPHY15JpsiDpst",
       HypothesisName           = "Bc",
       V0Tools                  = V0Tools,
       TrkVertexFitterTool      = BcJpsiDstVertexFit,
       PVRefitter               = PVrefit,
       DxHypothesis             = 421,
       ApplyD0MassConstraint    = True,
       ApplyJpsiMassConstraint  = True,
       JpsiMassLowerCut         = 2600.,
       JpsiMassUpperCut         = 3600.,
       JpsipiMassLowerCut       = 2600.,
       JpsipiMassUpperCut       = 6800.,
       D0MassLowerCut           = 1864.83 - 200.,
       D0MassUpperCut           = 1864.83 + 200.,
       DstMassLowerCut          = 2010.26 - 300.,
       DstMassUpperCut          = 2010.26 + 300.,
       MassLowerCut             = 5400,
       MassUpperCut             = 6274.90 + 600.,
       Chi2Cut                 = 10,
       RefitPV                  = True,
       RefPVContainerName       = "BPHY15RefittedPrimaryVertices4",
       JpsipiVertices           = "BPHY15JpsipiCandidates",
       CascadeVertexCollections = ["BcJpsiDpstCascadeSV2", "BcJpsiDpstCascadeSV1"],
       D0Vertices               = "BPHY15DiTrkCandidates")

   from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
   v0Vertexfit = acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))
   acc.addPublicTool(v0Vertexfit)
   BPHY15K0Finder = CompFactory.Analysis.JpsiFinder(
       name                       = "BPHY15K0Finder",
       muAndMu                    = False,
       muAndTrack                 = False,
       TrackAndTrack              = True,
       assumeDiMuons              = False,    # If true, will assume dimu hypothesis and use PDG value for mu mass
       trackThresholdPt           = 400,
       invMassUpper               = 600.0,
       invMassLower               = 400.0,
       Chi2Cut                    = 20,
       oppChargesOnly             = True,
       atLeastOneComb             = False,
       useCombinedMeasurement     = False, # Only takes effect if combOnly=True
       muonCollectionKey          = "Muons",
       TrackParticleCollection    = "InDetTrackParticles",
       V0VertexFitterTool         = v0Vertexfit,             # V0 vertex fitter
       useV0Fitter                = True,                   # if False a TrkVertexFitterTool will be used
       TrkVertexFitterTool        = vkalvrt,        # VKalVrt vertex fitter
       TrackSelectorTool          = trackselect,
       VertexPointEstimator       = vpest,
       useMCPCuts                 = False,
       track1Mass                 = 139.571, # Not very important, only used to calculate inv. mass cut, leave it loose here
       track2Mass                 = 139.571)
   acc.addPublicTool(BPHY15K0Finder)
   BPHY15K0SelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
       name                   = "BPHY15K0SelectAndWrite",
       VertexSearchTool       = BPHY15K0Finder,
       OutputVtxContainerName = "BPHY15K0Candidates",
       PVContainerName        = "PrimaryVertices",
       RefPVContainerName     = "SHOULDNOTBEUSED",
       V0Tools                = V0Tools,
       PVRefitter             = PVrefit,
       CheckCollections       = True,
       CheckVertexContainers  = ['BPHY15JpsipiCandidates','BPHY15DiTrkCandidates','BcJpsiDpstCascadeSV1'],
       DoVertexType           = 1)

   BPHY15_Select_K0 = CompFactory.DerivationFramework.Select_onia2mumu(
       name                  = "BPHY15_Select_K0",
       HypothesisName        = "K0",
       InputVtxContainerName = "BPHY15K0Candidates",
       V0Tools               = V0Tools,
       TrkMasses             = [139.571, 139.571],
       VtxMassHypo           = 497.672,
       MassMin               = 400,
       MassMax               = 600,
       LxyMin                = 0.2,
       Chi2Max               = 200)

   BcJpsiDs1VertexFit = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName + "BcJpsiDs1", CascadeCnstPrecision = 1e-6))
   acc.addPublicTool(BcJpsiDs1VertexFit)

   BPHY15JpsiDps1 = CompFactory.DerivationFramework.JpsiPlusDs1Cascade(
       name                     = "BPHY15JpsiDps1",
       HypothesisName           = "Bc",
       TrkVertexFitterTool      = BcJpsiDs1VertexFit,
       PVRefitter               = PVrefit,
       V0Tools                  = V0Tools,
       DxHypothesis             = 421,
       ApplyD0MassConstraint    = True,
       ApplyK0MassConstraint    = True,
       ApplyJpsiMassConstraint  = True,
       JpsiMassLowerCut         = 2600.,
       JpsiMassUpperCut         = 3600.,
       JpsipiMassLowerCut       = 2600.,
       JpsipiMassUpperCut       = 6800.,
       D0MassLowerCut           = 1864.83 - 180.,
       D0MassUpperCut           = 1864.83 + 180.,
       K0MassLowerCut           = 400.,
       K0MassUpperCut           = 600.,
       DstMassLowerCut          = 2010.26 - 300.,
       DstMassUpperCut          = 2010.26 + 300.,
       MassLowerCut             = 6274.90 - 600,
       MassUpperCut             = 6274.90 + 600.,
       Chi2Cut                 = 10,
       RefitPV                  = True,
       RefPVContainerName       = "BPHY15RefittedPrimaryVertices5",
       JpsipiVertices           = "BPHY15JpsipiCandidates",
       CascadeVertexCollections = ["BcJpsiDps1CascadeSV3", "BcJpsiDps1CascadeSV2", "BcJpsiDps1CascadeSV1"],
       K0Vertices               = "BPHY15K0Candidates",
       D0Vertices               = "BPHY15DiTrkCandidates")

   #--------------------------------------------------------------------
   
   CascadeCollections = []
   
   CascadeCollections += BPHY15JpsiDs.CascadeVertexCollections
   CascadeCollections += BPHY15JpsiDp.CascadeVertexCollections
   
   CascadeCollections += BPHY15JpsiDpst.CascadeVertexCollections
   CascadeCollections += BPHY15JpsiDps1.CascadeVertexCollections
   
   #--------------------------------------------------------------------
   if not isSimulation: #Only Skim Data
      BPHY15_SelectBcJpsipiEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
                           name = "BPHY15_SelectBcJpsipiEvent",
                           expression = "( count(BPHY15BcJpsipiCandidates.passed_Bc) > 0)")
      acc.addPublicTool(BPHY15_SelectBcJpsipiEvent)
      BPHY15_AnyVertexSkimmingTool = CompFactory.DerivationFramework.AnyVertexSkimmingTool("BPHY15_AnyVertexSkimmingTool", UseHandles = True,
                                                                        VertexContainerNames =CascadeCollections )
      acc.addPublicTool(BPHY15_AnyVertexSkimmingTool)
      #====================================================================
      # Make event selection based on an OR of the input skimming tools
      #====================================================================
         
      BPHY15SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR(
                           "BPHY15SkimmingOR",
                           FilterList = [BPHY15_SelectBcJpsipiEvent, BPHY15_AnyVertexSkimmingTool] )
      acc.addPublicTool(BPHY15SkimmingOR)

   augTools = [BPHY15JpsiSelectAndWrite, BPHY15_Select_Jpsi2mumu,
                         BPHY15BcJpsipiSelectAndWrite, BPHY15_Select_Bc2Jpsipi,
                         BPHY15JpsipiSelectAndWrite, BPHY15_Select_Jpsipi,
                         BPHY15DiTrkSelectAndWrite, BPHY15_Select_D0, BPHY15_Select_D0b,
                         BPHY15Dh3SelectAndWrite, BPHY15_Select_Ds, BPHY15_Select_Dp, BPHY15_Select_Dm,
                         BPHY15JpsiDs,
                         BPHY15JpsiDp,
                         BPHY15JpsiDpst,
                         BPHY15K0SelectAndWrite, BPHY15_Select_K0,
                         BPHY15JpsiDps1,
                         BPHY15_AugOriginalCounts]
   for t in  augTools : acc.addPublicTool(t)
   acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY15Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = [BPHY15SkimmingOR] if not isSimulation else [],
                                                    ThinningTools     = []))

   from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
   BPHY15SlimmingHelper = SlimmingHelper("BPHY15SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
   from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
   AllVariables  = getDefaultAllVariables()
   StaticContent = []

   # Needed for trigger objects
   BPHY15SlimmingHelper.IncludeMuonTriggerContent  = True
   BPHY15SlimmingHelper.IncludeBPhysTriggerContent = True

   ## primary vertices
   AllVariables  += ["PrimaryVertices"]
   for x in range(1,6):
      StaticContent += ["xAOD::VertexContainer#BPHY15RefittedPrimaryVertices%s"   %     str(x)]
      StaticContent += ["xAOD::VertexAuxContainer#BPHY15RefittedPrimaryVertices%sAux." % str(x)]

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
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY15JpsiSelectAndWrite.OutputVtxContainerName]
   ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY15JpsiSelectAndWrite.OutputVtxContainerName]

   ## Bc+>J/psi pi+ candidates
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY15BcJpsipiSelectAndWrite.OutputVtxContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY15BcJpsipiSelectAndWrite.OutputVtxContainerName]


   ## Bc+>J/psi D_(s)+/-, J/psi D*+/- and J/psi D_s1+/- candidates
   for cascades in CascadeCollections:
      StaticContent += ["xAOD::VertexContainer#%s"   %     cascades]
      StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascades]

   # Tagging information (in addition to that already requested by usual algorithms)
   AllVariables += ["GSFTrackParticles", "MuonSpectrometerTrackParticles" ]

   # Truth information for MC only
   if isSimulation:
       AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

   AllVariables = list(set(AllVariables)) # remove duplicates

   BPHY15SlimmingHelper.AllVariables = AllVariables
   BPHY15SlimmingHelper.StaticContent = StaticContent

   BPHY15ItemList = BPHY15SlimmingHelper.GetItemList()
   acc.merge(OutputStreamCfg(flags, "DAOD_BPHY15", ItemList=BPHY15ItemList, AcceptAlgs=["BPHY15Kernel"]))
   acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY15", AcceptAlgs=["BPHY15Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
   acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
   return acc
