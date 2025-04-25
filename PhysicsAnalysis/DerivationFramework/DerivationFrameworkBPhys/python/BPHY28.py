# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY28.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY28"
streamName = "StreamDAOD_BPHY28"

MuMuContainerName = "BPHY28MuMuCandidates"
BsPhiMuMuContainerName   = "BPHY28BsKKMuMuCandidates"


def BPHY28Kernel(flags):

   # Lists for better code organization
   augList          = [] # List of active augmentation tools
   skimList          = [] # List of active skimming algorithms
   thinList          = [] # List of active thinning algorithms
   thinTrkVtxList    = [] # List of reconstructed candidates to use for the thinning of tracks from vertices


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
   PVrefit = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
   acc.addPublicTool(PVrefit)

   # LRT
   doLRT = flags.Tracking.doLargeD0
   if not doLRT : print("BPHY28: LRT tracks disabled")
   mainMuonInput = "StdWithLRTMuons" if doLRT else "Muons"
   mainIDInput   = "InDetWithLRTTrackParticles" if doLRT else "InDetTrackParticles"
   if doLRT:
      from DerivationFrameworkLLP.LLPToolsConfig import LRTMuonMergerAlg
      from AthenaConfiguration.Enums import LHCPeriod
      acc.merge(LRTMuonMergerAlg( flags,
                                 PromptMuonLocation    = "Muons",
                                 LRTMuonLocation       = "MuonsLRT",
                                 OutputMuonLocation    = mainMuonInput,
                                 CreateViewCollection  = True,
                                 UseRun3WP = flags.GeoModel.Run == LHCPeriod.Run3))
      from DerivationFrameworkInDet.InDetToolsConfig import InDetLRTMergeCfg
      acc.merge(InDetLRTMergeCfg(flags))
   toRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else []
   MuonReLink = [ "Muons", "MuonsLRT" ] if doLRT else []


   BPHY28_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
      name = "BPHY28_AugOriginalCounts",
      VertexContainer = "PrimaryVertices",
      TrackContainer = "InDetTrackParticles",
      TrackLRTContainer = "InDetLargeD0TrackParticles" if doLRT else "")
   augList += [ BPHY28_AugOriginalCounts ]

   BPHY28MuMuFinder = CompFactory.Analysis.JpsiFinder(
       name                        = "BPHY28MuMuFinder",
       muAndMu                     = True,
       muAndTrack                  = False,
       TrackAndTrack               = False,
       assumeDiMuons               = True,
       invMassUpper                = 100000.0,
       invMassLower                = 0.0,
       Chi2Cut                     = 100.,
       oppChargesOnly              = True,
       combOnly                    = True,
       atLeastOneComb              = False,
       useCombinedMeasurement      = False, # Only takes effect if combOnly=True
       muonCollectionKey           = mainMuonInput,
       TrackParticleCollection     = mainIDInput,
       useV0Fitter                 = False,                   # if False a TrkVertexFitterTool will be used
       TrkVertexFitterTool         = vkalvrt,
       V0VertexFitterTool          = None,
       TrackSelectorTool           = trackselect,
       VertexPointEstimator        = vpest,
       useMCPCuts                  = False )

   BPHY28MuMuSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name = "BPHY28MuMuSelectAndWrite",
                                                       VertexSearchTool       = BPHY28MuMuFinder,
                                                       OutputVtxContainerName = MuMuContainerName,
                                                       PVContainerName        = "PrimaryVertices",
                                                       V0Tools                = V0Tools,
                                                       PVRefitter             = PVrefit,
                                                       RefPVContainerName     = "SHOULDNOTBEUSED",
                                                       RelinkTracks           =  toRelink,
                                                       RelinkMuons            =  MuonReLink,
                                                       DoVertexType           = 7)
   augList += [ BPHY28MuMuSelectAndWrite ]
   thinTrkVtxList += [ MuMuContainerName ]
   
   ## a/ augment and select Jpsi->mumu candidates
   BPHY28_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
         name                  = "BPHY28_Select_Jpsi2mumu",
         HypothesisName        = "Jpsi",
         InputVtxContainerName = MuMuContainerName,
         V0Tools               = V0Tools,
         VtxMassHypo           = 3096.916,
         MassMin               = 2000.0,
         MassMax               = 3600.0,
         Chi2Max               = 100, Do3d = False,
         DoVertexType          = 7)
   augList += [ BPHY28_Select_Jpsi2mumu ]


   BPHY28BsKKMuMu = CompFactory.Analysis.JpsiPlus2Tracks(name = "BPHY28BsKKMuMu",
         kaonkaonHypothesis          = True,
         pionpionHypothesis          = False,
         kaonpionHypothesis          = False,
         trkThresholdPt              = 500.0,
         trkMaxEta                   = 3.0,
         BMassUpper                  = 5900.0,
         BMassLower                  = 4900.0,
         DiTrackMassUpper            = 1220,
         DiTrackMassLower            = 820,
         Chi2Cut                     = 100.0, # this is chi2/ndf cut
         TrkQuadrupletMassUpper      = 6000.0,
         TrkQuadrupletMassLower      = 4800.0,
         JpsiContainerKey            = MuMuContainerName,
         TrackParticleCollection     = mainIDInput,
         MuonsUsedInJpsi             = mainMuonInput,
         TrkVertexFitterTool         = vkalvrt,
         TrackSelectorTool           = trackselect,
         UseMassConstraint           = False)

   
   BPHY28BsKKSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name = "BPHY28BsKKSelectAndWrite",
                        VertexSearchTool         = BPHY28BsKKMuMu,
                        OutputVtxContainerName   = BsPhiMuMuContainerName,
                        PVContainerName          = "PrimaryVertices",
                        V0Tools                  = V0Tools,
                        PVRefitter               = PVrefit,
                        RefPVContainerName       = "BPHY28RefittedPrimaryVertices",
                        RefitPV                  = True, Do3d = False,
                        RelinkTracks  =  toRelink,
                        MaxPVrefit               = 10000, DoVertexType = 7)
   augList += [ BPHY28BsKKSelectAndWrite ]
   thinTrkVtxList += [ BsPhiMuMuContainerName ]


   BPHY28_Select_Bs2KKMuMu = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY28_Select_Bs2KKMuMu",
                                 HypothesisName             = "Bs",
                                 InputVtxContainerName      = BsPhiMuMuContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 493.677, 493.677],
                                 VtxMassHypo                = 5366.3,
                                 MassMin                    = 4900.0,
                                 MassMax                    = 5900.0, Do3d = False,
                                 Chi2Max                    = 100)
   augList += [ BPHY28_Select_Bs2KKMuMu ]


   if not isSimulation: #Only Skim Data
      BPHY28_SelectBsKKMuMuEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
         name = "BPHY28_SelectBsKKMuMuEvent",
         expression = "count(BPHY28BsKKMuMuCandidates.passed_Bs) > 0")

      skimList += [ BPHY28_SelectBsKKMuMuEvent ]


   ## Thinning
   # ID tracks
   BPHY28_Thin_VtxTracks = CompFactory.DerivationFramework.Thin_vtxTrk( name     = "BPHY28_Thin_VtxTracks",
                                                               StreamName = streamName,
                                                               TrackParticleContainerName = "InDetTrackParticles",
                                                               VertexContainerNames       = thinTrkVtxList,
                                                               IgnoreFlags                = True )
   thinList += [ BPHY28_Thin_VtxTracks ]

   # LRT ID tracks
   if doLRT:
      BPHY28_Thin_VtxTracks_LRT = CompFactory.DerivationFramework.Thin_vtxTrk( name     = "BPHY28_Thin_VtxTracks_LRT",
                                                                  StreamName = streamName,
                                                                  TrackParticleContainerName = "InDetLargeD0TrackParticles",
                                                                  VertexContainerNames       = thinTrkVtxList,
                                                                  IgnoreFlags                = True )
      thinList += [ BPHY28_Thin_VtxTracks_LRT ]


   for t in  augList + skimList + thinList : acc.addPublicTool(t)
   acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY28Kernel",
                                                    AugmentationTools = augList,
                                                    #Only skim if not MC
                                                    SkimmingTools     = skimList,
                                                    ThinningTools     = thinList))

   return acc


def BPHY28Cfg(flags):
   doLRT = flags.Tracking.doLargeD0
   isSimulation = flags.Input.isMC
   acc = BPHY28Kernel(flags)
   from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
   BPHY28SlimmingHelper = SlimmingHelper("BPHY28SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
   from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
   AllVariables  = getDefaultAllVariables()
   StaticContent = []
    
   # Needed for trigger objects
   BPHY28SlimmingHelper.IncludeMuonTriggerContent  = True
   BPHY28SlimmingHelper.IncludeBPhysTriggerContent = True
    
   ## primary vertices
   AllVariables  += ["PrimaryVertices"]
   StaticContent += ["xAOD::VertexContainer#BPHY28RefittedPrimaryVertices"]
   StaticContent += ["xAOD::VertexAuxContainer#BPHY28RefittedPrimaryVerticesAux."]

    
   ## ID track particles
   AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]
    
   ## combined / extrapolated muon track particles 
   ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
   ##        are store in InDetTrackParticles collection)
   AllVariables += ["CombinedMuonTrackParticles"]
   AllVariables += ["ExtrapolatedMuonTrackParticles"]
   
   ## muon container
   AllVariables += ["Muons", "MuonsLRT"] if doLRT else ["Muons"]
    
    
   ## PhiMuMu candidates 
   StaticContent += ["xAOD::VertexContainer#%s"        %                 MuMuContainerName]
   ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % MuMuContainerName]
    
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BsPhiMuMuContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BsPhiMuMuContainerName]
   
   
   # Tagging information (in addition to that already requested by usual algorithms)
   AllVariables += [ "MuonSpectrometerTrackParticles" ]
   SmartVar = ["Photons", "Electrons", "LRTElectrons"] if doLRT else ["Photons", "Electrons"] #[ tagJetCollections ]
   
   
   # Truth information for MC only
   if isSimulation:
       AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles", "egammaTruthParticles" ]
   
   
   AllVariables = list(set(AllVariables)) # remove duplicates
   
   BPHY28SlimmingHelper.AllVariables = AllVariables
   BPHY28SlimmingHelper.StaticContent = StaticContent
   BPHY28SlimmingHelper.SmartCollections = SmartVar
   BPHY28ItemList = BPHY28SlimmingHelper.GetItemList()
   acc.merge(OutputStreamCfg(flags, "DAOD_BPHY28", ItemList=BPHY28ItemList, AcceptAlgs=["BPHY28Kernel"]))
   acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY28", AcceptAlgs=["BPHY28Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
   acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
   return acc
