# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY5.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY5"
streamName = "StreamDAOD_BPHY5"

JpsiContainerName = "BPHY5JpsiCandidates"
BsJpsiPhiContainerName   = "BPHY5BsJpsiKKCandidates"
BPlusContainerName   = "BPHY5BpmJpsiKpmCandidates"
BpipiContainerName   = "BPHY5BJpsipipiXCandidates"
BdJpsiKstContainerName = "BPHY5BdJpsiKstCandidates"

def BPHY5Kernel(flags, Decays="BsB+BdKstBpipiX"):
   from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
   from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
   acc = ComponentAccumulator()
   isSimulation = flags.Input.isMC

   doLRT = flags.Tracking.doLargeD0
   if not doLRT : print("BPHY5: LRT tracks disabled")
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

   BPHY5_AugOriginalCounts = CompFactory.DerivationFramework.AugOriginalCounts(
       name = "BPHY5_AugOriginalCounts",
       VertexContainer = "PrimaryVertices",
       TrackContainer = "InDetTrackParticles",
       TrackLRTContainer = "InDetLargeD0TrackParticles" if doLRT else "")
   toRelink = ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else []
   MuonReLink = [ "Muons", "MuonsLRT" ] if doLRT else []

   V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
   vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))        # VKalVrt vertex fitter
   acc.addPublicTool(vkalvrt)
   acc.addPublicTool(V0Tools)
   trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
   acc.addPublicTool(trackselect)
   vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
   acc.addPublicTool(vpest)
   BPHY5JpsiFinder = CompFactory.Analysis.JpsiFinder(
       name                        = "BPHY5JpsiFinder",
       muAndMu                     = True,
       muAndTrack                  = False,
       TrackAndTrack               = False,
       assumeDiMuons               = True,
       invMassUpper                = 3600.0,
       invMassLower                = 2600.0,
       Chi2Cut                     = 30.,
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
   acc.addPublicTool(BPHY5JpsiFinder )

   BPHY5JpsiSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name = "BPHY5JpsiSelectAndWrite",
                                                       VertexSearchTool       = BPHY5JpsiFinder,
                                                       OutputVtxContainerName = JpsiContainerName,
                                                       PVContainerName        = "PrimaryVertices",
                                                       V0Tools                = V0Tools,
                                                       PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                                       RefPVContainerName     = "SHOULDNOTBEUSED",
                                                       RelinkTracks  =  toRelink,
                                                       RelinkMuons   =  MuonReLink,
                                                       DoVertexType           =1)
   
   ## a/ augment and select Jpsi->mumu candidates
   BPHY5_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
         name                  = "BPHY5_Select_Jpsi2mumu",
         HypothesisName        = "Jpsi",
         InputVtxContainerName = JpsiContainerName,
         V0Tools               = V0Tools,
         VtxMassHypo           = 3096.916,
         MassMin               = 2000.0,
         MassMax               = 3600.0,
         Chi2Max               = 200, Do3d = False,
         DoVertexType =1)

   BPHY5BsJpsiKK = CompFactory.Analysis.JpsiPlus2Tracks(name = "BPHY5BsJpsiKK",
         kaonkaonHypothesis          = True,
         pionpionHypothesis          = False,
         kaonpionHypothesis          = False,
         trkThresholdPt              = 800.0,
         trkMaxEta                   = 3.0,
         BMassUpper                  = 5800.0,
         BMassLower                  = 5000.0,
         #DiTrackMassUpper = 1019.445 + 100.,
         #DiTrackMassLower = 1019.445 - 100.,
         Chi2Cut                     = 15.0,
         TrkQuadrupletMassUpper      = 6000.0,
         TrkQuadrupletMassLower      = 4800.0,
         JpsiContainerKey            = JpsiContainerName,
         TrackParticleCollection     = mainIDInput,
         MuonsUsedInJpsi             = mainMuonInput,
         TrkVertexFitterTool         = vkalvrt,
         TrackSelectorTool           = trackselect,
         UseMassConstraint           = True)
   acc.addPublicTool(BPHY5BsJpsiKK )
   BPHY5BdJpsiKst = CompFactory.Analysis.JpsiPlus2Tracks(
         name                    = "BPHY5BdJpsiKst",
         kaonkaonHypothesis      = False,
         pionpionHypothesis      = False,
         kaonpionHypothesis      = True,
         trkThresholdPt          = 800.0,
         trkMaxEta               = 3.0,
         BThresholdPt            = 5000.,
         BMassLower              = 4300.0,
         BMassUpper              = 6300.0,
         JpsiContainerKey        = JpsiContainerName,
         TrackParticleCollection = mainIDInput,
         #MuonsUsedInJpsi      = "Muons", #Don't remove all muons, just those in J/psi candidate (see the following cut)
         ExcludeCrossJpsiTracks  = False,   #setting this to False rejects the muons from J/psi candidate
         TrkVertexFitterTool     = vkalvrt,
         TrackSelectorTool     = trackselect,
         UseMassConstraint     = True,
         Chi2Cut                 = 15.0,
         TrkQuadrupletMassLower  = 3500.0,
         TrkQuadrupletMassUpper  = 6800.0,
         )
   acc.addPublicTool(BPHY5BdJpsiKst )
   BPHY5BplJpsiKpl = CompFactory.Analysis.JpsiPlus1Track(name = "BPHY5BplJpsiKpl",
         pionHypothesis          = True,
         kaonHypothesis          = True,
         trkThresholdPt          = 750.0,
         trkMaxEta           = 3.0,
         BThresholdPt            = 4000.0,
         BMassUpper          = 7000.0,
         BMassLower          = 4500.0,
         Chi2Cut                         = 15.0,
         TrkTrippletMassUpper            = 8000,
         TrkTrippletMassLower            = 4000,
         JpsiContainerKey        = JpsiContainerName,
         TrackParticleCollection         = mainIDInput,
         MuonsUsedInJpsi         = mainMuonInput,
         TrkVertexFitterTool     = vkalvrt,
         TrackSelectorTool       = trackselect,
         UseMassConstraint       = True,
         ExcludeCrossJpsiTracks              = False,
         ExcludeJpsiMuonsOnly                = True)
   acc.addPublicTool(BPHY5BplJpsiKpl )
   BPHY5BJpsipipiX = CompFactory.Analysis.JpsiPlus2Tracks(name = "BPHY5BJpsipipiX",
         kaonkaonHypothesis          = False,
         pionpionHypothesis          = True,
         kaonpionHypothesis          = False,
         trkThresholdPt              = 800.0,
         trkMaxEta                   = 3.0,
         BMassUpper                  = 5800.0,
         BMassLower                  = 3400.0,
         #DiTrackMassUpper = 1019.445 + 100.,
         #DiTrackMassLower = 1019.445 - 100.,
         Chi2Cut                     = 15.0,
         TrkQuadrupletMassUpper      = 5800.0,
         TrkQuadrupletMassLower      = 3400.0,
         JpsiContainerKey            = JpsiContainerName,
         TrackParticleCollection     = mainIDInput,
         MuonsUsedInJpsi             = mainMuonInput,
         TrkVertexFitterTool     = vkalvrt,
         TrackSelectorTool       = trackselect,
         UseMassConstraint       = True,
         ExcludeCrossJpsiTracks  = False,
         ExcludeJpsiMuonsOnly    = True)
   acc.addPublicTool(BPHY5BJpsipipiX )
   BPHY5BsKKSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name = "BPHY5BsKKSelectAndWrite",
                        VertexSearchTool         = BPHY5BsJpsiKK,
                        OutputVtxContainerName   = BsJpsiPhiContainerName,
                        PVContainerName          = "PrimaryVertices",
                        V0Tools                  = V0Tools,
                        PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                        RefPVContainerName       = "BPHY5RefittedPrimaryVertices",
                        RefitPV                  = True, Do3d = False,
                        RelinkTracks  =  toRelink,
                        MaxPVrefit               = 10000, DoVertexType = 7)

   BPHY5BplKplSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name    = "BPHY5BplKplSelectAndWrite",
                                                              VertexSearchTool      =  BPHY5BplJpsiKpl,
                                                              OutputVtxContainerName    = BPlusContainerName,
                                                              PVContainerName           = "PrimaryVertices",
                                                              V0Tools                   = V0Tools,
                                                              PVRefitter                = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                                              RefPVContainerName        = "BPHY5RefBplJpsiKplPrimaryVertices",                                                              
                                                              RefitPV                   = True,
                                                              RelinkTracks  =  toRelink,
                                                              MaxPVrefit                = 10000 )

   BPHY5BpipiXSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(name  = "BPHY5BpipiXSelectAndWrite",
                                                           VertexSearchTool       = BPHY5BJpsipipiX,
                                                           OutputVtxContainerName   = BpipiContainerName,
                                                           PVContainerName          = "PrimaryVertices",
                                                           V0Tools                  = V0Tools,
                                                           PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                                           RefPVContainerName       = "BPHY5RefittedBPipiPrimaryVertices",
                                                           RefitPV                  = True, Do3d = False,
                                                           RelinkTracks  =  toRelink,
                                                           MaxPVrefit               = 10000, DoVertexType = 7)

   BPHY5BdKstSelectAndWrite  = CompFactory.DerivationFramework.Reco_Vertex(
                                 name                   = "BPHY5BdKstSelectAndWrite",
                                 VertexSearchTool       = BPHY5BdJpsiKst,
                                 OutputVtxContainerName = BdJpsiKstContainerName,
                                 V0Tools                = V0Tools,
                                 PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
                                 PVContainerName        = "PrimaryVertices",
                                 RefPVContainerName     = "BPHY5RefittedKstPrimaryVertices",
                                 RefitPV                = True,
                                 RelinkTracks  =  toRelink,
                                 MaxPVrefit             = 10000,
                                 DoVertexType = 7)

   BPHY5_Select_Bd2JpsiKst = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY5_Select_Bd2JpsiKst",
                                 HypothesisName             = "Bd",
                                 InputVtxContainerName      = BdJpsiKstContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 493.677, 139.570],
                                 VtxMassHypo                = 5279.6,
                                 MassMin                    = 100.0,      #no mass cuts here
                                 MassMax                    = 100000.0,   #no mass cuts here
                                 Chi2Max                    = 200)

   BPHY5_Select_Bd2JpsiKstbar = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY5_Select_Bd2JpsiKstbar",
                                 HypothesisName             = "Bdbar",
                                 InputVtxContainerName      = BdJpsiKstContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 139.570, 493.677],
                                 VtxMassHypo                = 5279.6,
                                 MassMin                    = 100.0,      #no mass cuts here
                                 MassMax                    = 100000.0,   #no mass cuts here
                                 Chi2Max                    = 200)

   BPHY5_Select_Bs2JpsiKK = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY5_Select_Bs2JpsiKK",
                                 HypothesisName             = "Bs",
                                 InputVtxContainerName      = BsJpsiPhiContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 493.677, 493.677],
                                 VtxMassHypo                = 5366.3,
                                 MassMin                    = 5000.0,
                                 MassMax                    = 5800.0, Do3d = False,
                                 Chi2Max                    = 200)

   BPHY5_Select_Bpl2JpsiKpl  = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY5_Select_Bpl2JpsiKpl",
                                 HypothesisName             = "Bplus",
                                 InputVtxContainerName      = BPlusContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 493.677],
                                 VtxMassHypo                = 5279.26,
                                 MassMin                    = 5279.26 - 500, Do3d = False,
                                 MassMax                    = 5279.26 + 500,
                                 Chi2Max                    = 200 )

   BPHY5_Select_Bpl2JpsiPi  = CompFactory.DerivationFramework.Select_onia2mumu(
                                 name                       = "BPHY5_Select_Bpl2JpsiPi",
                                 HypothesisName             = "Bc",
                                 InputVtxContainerName      = BPlusContainerName,
                                 V0Tools                    = V0Tools,
                                 TrkMasses                  = [105.658, 105.658, 139.570],
                                 VtxMassHypo                = 6275.1, Do3d = False,
                                 MassMin                    = 6275.1 - 500,
                                 MassMax                    = 6275.1 + 500,
                                 Chi2Max                    = 200 )

   BPHY5_Select_B2JpsipipiX = CompFactory.DerivationFramework.Select_onia2mumu(
                                name                       = "BPHY5_Select_B2JpsipipiX",
                                HypothesisName             = "pipiJpsi",
                                InputVtxContainerName      = BpipiContainerName,
                                V0Tools                    = V0Tools,
                                TrkMasses                  = [105.658, 105.658, 139.570, 139.570],
                                VtxMassHypo                = 4260,
                                MassMin                    = 3400.0,
                                MassMax                    = 5800.0, Do3d = False,
                                Chi2Max                    = 200)

   #Do vertices without constraint for systematic reference
   BPHY5_Revertex_Bs_NoMassConst  = CompFactory.DerivationFramework.ReVertex(
       name                       = "BPHY5_Revertex_Bs_NoMassConst",
       InputVtxContainerName      = BsJpsiPhiContainerName,
       TrackIndices               = [ 0, 1, 2, 3 ],
       RefitPV                    = False,
       UseMassConstraint          = False,
       TrackContainerName         = mainIDInput,
       RelinkTracks  =  toRelink,
       PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
       TrkVertexFitterTool        = vkalvrt,
       OutputVtxContainerName     = "BPHY5BsJpsiKKCandidatesNoConstraint")

   BPHY5_Revertex_Bd_NoMassConst  = CompFactory.DerivationFramework.ReVertex(
       name                       = "BPHY5_Revertex_Bd_NoMassConst",
       InputVtxContainerName      = "BPHY5BdJpsiKstCandidates",
       TrackIndices               = [ 0, 1, 2, 3 ],
       RefitPV                    = False,
       UseMassConstraint          = False,
       TrackContainerName         = mainIDInput,
       RelinkTracks  =  toRelink,
       PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
       TrkVertexFitterTool        = vkalvrt,
       OutputVtxContainerName     = "BPHY5BdJpsiKstCandidatesNoConstraint")

   BPHY5_Select_Bs2JpsiKKNoConstraint = CompFactory.DerivationFramework.Select_onia2mumu(
     name                       = "BPHY5_Select_Bs2JpsiKKNoConstraint",
     HypothesisName             = "Bs",
     V0Tools                    = V0Tools,
     InputVtxContainerName      = "BPHY5BsJpsiKKCandidatesNoConstraint",
     TrkMasses                  = [105.658, 105.658, 493.677, 493.677],
     VtxMassHypo                = 5366.3,
     MassMin                    = 5000.0,
     MassMax                    = 5800.0, Do3d = False,
     Chi2Max                    = 200)

   BPHY5_Select_BdBar2JpsiKpiNoConstraint = CompFactory.DerivationFramework.Select_onia2mumu(
     name                       = "BPHY5_Select_BdBar2JpsiKpiNoConstraint",
     HypothesisName             = "BdBar",
     V0Tools                    = V0Tools,
     InputVtxContainerName      = "BPHY5BdJpsiKstCandidatesNoConstraint",
     TrkMasses                  = [105.658, 105.658, 139.570, 493.677],
     VtxMassHypo                = 5279.6,
     MassMin                    = 100.0,      #no mass cuts here
     MassMax                    = 100000.0,   #no mass cuts here
     Chi2Max                    = 200)

   BPHY5_Select_Bd2JpsiKpiNoConstraint = CompFactory.DerivationFramework.Select_onia2mumu(
     name                       = "BPHY5_Select_Bd2JpsiKpiNoConstraint",
     HypothesisName             = "Bd",
     V0Tools                    = V0Tools,
     InputVtxContainerName      = "BPHY5BdJpsiKstCandidatesNoConstraint",
     TrkMasses                  = [105.658, 105.658, 493.677, 139.570],
     VtxMassHypo                = 5279.6,
     MassMin                    = 100.0,      #no mass cuts here
     MassMax                    = 100000.0,   #no mass cuts here
     Chi2Max                    = 200)


   if not isSimulation: #Only Skim Data
        BPHY5_SelectBsJpsiKKEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
          name = "BPHY5_SelectBsJpsiKKEvent",
          expression = f"count({BsJpsiPhiContainerName}.passed_Bs > 0) > 0")

        BPHY5_SelectBplJpsiKplEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY5_SelectBplJpsiKplEvent",
                                       expression = f"count({BPlusContainerName}.passed_Bplus>0) > 0")
     
        BPHY5_SelectBplJpsiKplEventBc = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY5_SelectBplJpsiKplEventBc",
                                       expression = f"count({BPlusContainerName}.passed_Bc>0) > 0")
        
        BPHY5_SelectBdKstarEventBd = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY5_SelectBdKstarEventBd",
                                       expression = f"count({BdJpsiKstContainerName}.passed_Bd>0) > 0")
     
        BPHY5_SelectBdKstarEventBdBar = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY5_SelectBdKstarEventBdbar",
                                       expression = f"count({BdJpsiKstContainerName}.passed_Bdbar>0) > 0")
        #====================================================================
        # Make event selection based on an OR of the input skimming tools
        #====================================================================
        filterlist = []
        if "Bs" in Decays : filterlist.append(BPHY5_SelectBsJpsiKKEvent)
        if "B+" in Decays : filterlist += [ BPHY5_SelectBplJpsiKplEvent,BPHY5_SelectBplJpsiKplEventBc]
        if "BdKst" in Decays : filterlist += [BPHY5_SelectBdKstarEventBd, BPHY5_SelectBdKstarEventBdBar]
        
        BPHY5SkimmingOR = CompFactory.DerivationFramework.FilterCombinationOR("BPHY5SkimmingOR",
                                              FilterList = filterlist)
        for t in filterlist +[BPHY5SkimmingOR]: acc.addPublicTool(t)

   augTools =   [BPHY5JpsiSelectAndWrite,  BPHY5_Select_Jpsi2mumu, BPHY5_AugOriginalCounts]
   if "Bs" in Decays : augTools += [BPHY5BsKKSelectAndWrite,  BPHY5_Select_Bs2JpsiKK, BPHY5_Revertex_Bs_NoMassConst, BPHY5_Select_Bs2JpsiKKNoConstraint]
   if "B+" in Decays : augTools += [BPHY5BplKplSelectAndWrite, BPHY5_Select_Bpl2JpsiKpl, BPHY5_Select_Bpl2JpsiPi]
   if "BdKst" in Decays : augTools += [ BPHY5BdKstSelectAndWrite, BPHY5_Select_Bd2JpsiKst, BPHY5_Select_Bd2JpsiKstbar, BPHY5_Revertex_Bd_NoMassConst, BPHY5_Select_BdBar2JpsiKpiNoConstraint, BPHY5_Select_Bd2JpsiKpiNoConstraint]
   if "BpipiX" in Decays : augTools+= [ BPHY5BpipiXSelectAndWrite, BPHY5_Select_B2JpsipipiX]
   acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY5Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = [BPHY5SkimmingOR] if not isSimulation else [],
                                                    ThinningTools     = []))
   for t in  augTools : acc.addPublicTool(t)
   return acc


def BPHY5Cfg(flags):
   doLRT = flags.Tracking.doLargeD0
   isSimulation = flags.Input.isMC
   acc = BPHY5Kernel(flags)
   from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
   BPHY5SlimmingHelper = SlimmingHelper("BPHY5SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
   from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
   AllVariables  = getDefaultAllVariables()
   StaticContent = []
    
   # Needed for trigger objects
   BPHY5SlimmingHelper.IncludeMuonTriggerContent  = True
   BPHY5SlimmingHelper.IncludeBPhysTriggerContent = True
    
   ## primary vertices
   AllVariables  += ["PrimaryVertices"]
   StaticContent += ["xAOD::VertexContainer#BPHY5RefittedPrimaryVertices"]
   StaticContent += ["xAOD::VertexAuxContainer#BPHY5RefittedPrimaryVerticesAux."]
   StaticContent += ["xAOD::VertexContainer#BPHY5RefBplJpsiKplPrimaryVertices"]
   StaticContent += ["xAOD::VertexAuxContainer#BPHY5RefBplJpsiKplPrimaryVerticesAux."]
   StaticContent += ["xAOD::VertexContainer#BPHY5RefittedBPipiPrimaryVertices"]
   StaticContent += ["xAOD::VertexAuxContainer#BPHY5RefittedBPipiPrimaryVerticesAux."]
   StaticContent += ["xAOD::VertexContainer#BPHY5RefittedKstPrimaryVertices"]
   StaticContent += ["xAOD::VertexAuxContainer#BPHY5RefittedKstPrimaryVerticesAux."]
    
   ## ID track particles
   AllVariables += ["InDetTrackParticles", "InDetLargeD0TrackParticles"] if doLRT else ["InDetTrackParticles"]
    
   ## combined / extrapolated muon track particles 
   ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
   ##        are store in InDetTrackParticles collection)
   AllVariables += ["CombinedMuonTrackParticles"]
   AllVariables += ["ExtrapolatedMuonTrackParticles"]
   
   ## muon container
   AllVariables += ["Muons", "MuonsLRT"] if doLRT else ["Muons"]
    
    
   ## Jpsi candidates 
   StaticContent += ["xAOD::VertexContainer#%s"        %                 JpsiContainerName]
   ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % JpsiContainerName]
    
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BsJpsiPhiContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BsJpsiPhiContainerName]
   
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BPlusContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPlusContainerName]
   
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BpipiContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BpipiContainerName]
   
   StaticContent += ["xAOD::VertexContainer#%s"        %                 BdJpsiKstContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BdJpsiKstContainerName]

   StaticContent += ["xAOD::VertexContainer#%sNoConstraint"        %                 BsJpsiPhiContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sNoConstraintAux.-vxTrackAtVertex" % BsJpsiPhiContainerName]

   StaticContent += ["xAOD::VertexContainer#%sNoConstraint"        %                 BdJpsiKstContainerName]
   StaticContent += ["xAOD::VertexAuxContainer#%sNoConstraintAux.-vxTrackAtVertex" % BdJpsiKstContainerName]

   # Tagging information (in addition to that already requested by usual algorithms)
   AllVariables += ["GSFTrackParticles",  "MuonSpectrometerTrackParticles" ]
   tagJetCollections = ['AntiKt4LCTopoJets', 'AntiKt4EMTopoJets', 'AntiKt4PV0TrackJets']
   
   AllVariables += [ "Kt4LCTopoOriginEventShape", "Kt4EMTopoOriginEventShape" ]
   SmartVar = ["Photons", "Electrons", "LRTElectrons"] #[ tagJetCollections ]
   
   for jet_collection in tagJetCollections:
       AllVariables   += [jet_collection]

   
   
   # Truth information for MC only
   if isSimulation:
       AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles", "egammaTruthParticles" ]
       AllVariables += ["AntiKt4TruthJets", "AntiKt4TruthWZJets" ]
       tagJetCollections += [ "AntiKt4TruthJets", "AntiKt4TruthWZJets"  ]
   
   
   AllVariables = list(set(AllVariables)) # remove duplicates
   
   BPHY5SlimmingHelper.AllVariables = AllVariables
   BPHY5SlimmingHelper.StaticContent = StaticContent
   BPHY5SlimmingHelper.SmartCollections = SmartVar
   BPHY5ItemList = BPHY5SlimmingHelper.GetItemList()
   acc.merge(OutputStreamCfg(flags, "DAOD_BPHY5", ItemList=BPHY5ItemList, AcceptAlgs=["BPHY5Kernel"]))
   acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY5", AcceptAlgs=["BPHY5Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
   acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
   return acc
