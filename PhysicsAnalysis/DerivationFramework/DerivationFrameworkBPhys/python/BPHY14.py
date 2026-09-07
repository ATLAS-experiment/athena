# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY13.py for di-μ + γ
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg

BPHYDerivationName = "BPHY14"
streamName = "StreamDAOD_BPHY14"

def BPHY14Cfg(flags):
       from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (
          BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg,
          BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg,
          AugOriginalCountsCfg)
       from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
       acc = ComponentAccumulator()
       isSimulation = flags.Input.isMC
       doLRT = flags.Tracking.doLargeD0
       if not doLRT : print("BPHY14: LRT tracks disabled")
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
       BPHY14JpsiFinder = CompFactory.Analysis.JpsiFinder(
                name                        = "BPHY14JpsiFinder",
                muAndMu                     = True,
                muAndTrack                  = False,
                TrackAndTrack               = False,
                assumeDiMuons               = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                invMassUpper                = 15000.0,
                invMassLower                = 2000.,
                Chi2Cut                     = 200.,
                muonThresholdPt             = 2500.,
                oppChargesOnly              = True,
                atLeastOneComb              = False,
                combOnly                    = True,
                useCombinedMeasurement      = False, # Only takes effect if combOnly=True
                muonCollectionKey           = "Muons",
                TrackParticleCollection     = mainIDInput,
                TrkVertexFitterTool         = vkalvrt,
                TrackSelectorTool           = trackselect,
                VertexPointEstimator        = vpest,
                useMCPCuts                  = False
        )
       acc.addPublicTool(BPHY14JpsiFinder )

       BPHY14_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
                                name                   = "BPHY14_Reco_mumu",
                                VertexSearchTool       = BPHY14JpsiFinder,
                                OutputVtxContainerName = "BPHY14OniaCandidates",
                                PVContainerName        = "PrimaryVertices",
                                V0Tools                = V0Tools,
                                PVRefitter             = PVrefit,
                                RelinkTracks  =  toRelink,
                                RelinkMuons   =  MuonReLink,
                                RefPVContainerName     = "BPHY14RefittedPrimaryVertices",
                                RefitPV                = True,
                                MaxPVrefit             = 100000,
                                DoVertexType           = 7)
        
       BPHY14_AugOriginalCounts = acc.popToolsAndMerge(AugOriginalCountsCfg(flags, name = "BPHY14_AugOriginalCounts"))

       BPHY14_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                                                                name                  = "BPHY14_Select_Jpsi2mumu",
                                                                HypothesisName        = "Jpsi",
                                                                InputVtxContainerName = "BPHY14OniaCandidates",
                                                                V0Tools               = V0Tools,
                                                                VtxMassHypo           = 3096.916,
                                                                MassMin               = 2000.0,
                                                                MassMax               = 4000.0,
                                                                Chi2Max               = 200,
                                                                DoVertexType          = 7)
       
       BPHY14_Select_Psi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                                                               name                  = "BPHY14_Select_Psi2mumu",
                                                               HypothesisName        = "Psi",
                                                               V0Tools               = V0Tools,
                                                               InputVtxContainerName = "BPHY14OniaCandidates",
                                                               VtxMassHypo           = 3686.09,
                                                               MassMin               = 3300.0,
                                                               MassMax               = 7500.0,
                                                               Chi2Max               = 200,
                                                               DoVertexType          = 7)

       BPHY14_Select_Upsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(
                                                                name                  = "BPHY14_Select_Upsi2mumu",
                                                                HypothesisName        = "Upsi",
                                                                InputVtxContainerName = "BPHY14OniaCandidates",
                                                                V0Tools               = V0Tools,
                                                                VtxMassHypo           = 9460.30,
                                                                MassMin               = 7000.0,
                                                                MassMax               = 15000.0,
                                                                Chi2Max               = 200,
                                                                DoVertexType          = 7)

       acc.merge(EGammaCommonCfg(flags))
       photonRequirements = 'Photons.Tight'
       expression = "(count(BPHY14OniaCandidates.passed_Jpsi) > 0 || count(BPHY14OniaCandidates.passed_Psi) > 0 || count(BPHY14OniaCandidates.passed_Upsi) > 0) && count("+photonRequirements+") >0"
       from DerivationFrameworkTools.DerivationFrameworkToolsConfig import   xAODStringSkimmingToolCfg
       BPHY14_SelectEvent = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags, name = "BPHY14_SelectEvent",
                                                                 expression = expression))

       BPHY14ThinningTools = []
       BPHY14Thin_vtxTrk = CompFactory.DerivationFramework.Thin_vtxTrk(
                                     name                       = "BPHY14Thin_vtxTrk",
                                     StreamName = streamName,
                                     TrackParticleContainerName = mainIDInput,
                                     VertexContainerNames       = ["BPHY14OniaCandidates"],
                                     PassFlags                  = ["passed_Jpsi", "passed_Psi", "passed_Upsi"] )

       BPHY14MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(name  = "BPHY14MuonTPThinningTool",
                                                                          StreamName = streamName,
                                                                          MuonKey                 = "Muons",
                                                                          InDetTrackParticlesKey  = mainIDInput)
       BPHY14ThinningTools.append(BPHY14MuonTPThinningTool)

       BPHY14PhotonTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(name = "BPHY14PhotonTPThinningTool",
                                                                              StreamName = streamName,
                                                                              SGKey                  = "Photons",
                                                                              GSFTrackParticlesKey   = "GSFTrackParticles",
                                                                              GSFConversionVerticesKey="GSFConversionVertices",
                                                                              InDetTrackParticlesKey = mainIDInput,
                                                                              SelectionString        = photonRequirements,
                                                                              BestMatchOnly          = False,
                                                                              ConeSize               = 0.6,
                                                                              #ApplyAnd               = False Has been removed?
                                                                              )
       BPHY14ThinningTools.append(BPHY14PhotonTPThinningTool)
       BPHY14TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name  = "BPHY14TruthThinTool",
                                                                StreamName = streamName,
                                                                ParticleSelectionString = "TruthParticles.isPhoton || TruthParticles.pdgId == 443 || TruthParticles.pdgId == 100443 || TruthParticles.pdgId == 553 || TruthParticles.pdgId == 100553 || TruthParticles.pdgId == 200553",
                                                                PreserveDescendants     = True,
                                                                PreserveAncestors      = True)
       if isSimulation:
           BPHY14ThinningTools.append(BPHY14TruthThinTool)

       BPHY14ThinningTools += [BPHY14PhotonTPThinningTool,  BPHY14MuonTPThinningTool, BPHY14Thin_vtxTrk]
       from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
       BPHY14SlimTools     = [ BPHY14_SelectEvent ]
       BPHY14AugTools      = [BPHY14_Reco_mumu, BPHY14_AugOriginalCounts, BPHY14_Select_Jpsi2mumu, BPHY14_Select_Jpsi2mumu,
                              BPHY14_Select_Psi2mumu, BPHY14_Select_Upsi2mumu]
       for t in BPHY14ThinningTools + BPHY14SlimTools + BPHY14AugTools: acc.addPublicTool(t)
       acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY14Kernel", 
                    AugmentationTools= BPHY14AugTools,  SkimmingTools     = BPHY14SlimTools,  ThinningTools  = BPHY14ThinningTools  ))
       from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
       from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
       BPHY14SlimmingHelper = SlimmingHelper("BPHY14SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
       BPHY14SlimmingHelper.SmartCollections = []
    
       # Needed for trigger objects
       BPHY14SlimmingHelper.IncludeMuonTriggerContent   = True
       BPHY14SlimmingHelper.IncludeBPhysTriggerContent  = True
       BPHY14SlimmingHelper.IncludeEGammaTriggerContent = True

       ## primary vertices
       BPHY14_SmartCollections  = ["PrimaryVertices"]
       BPHY14_StaticContent = ["xAOD::VertexContainer#BPHY14RefittedPrimaryVertices"]
       BPHY14_StaticContent += ["xAOD::VertexAuxContainer#BPHY14RefittedPrimaryVerticesAux."]
        
       ## ID track particles
       #Does not work with LRT
       BPHY14_ExtraVariables = []
       #BPHY14_SmartCollections += [mainIDInput]
       #BPHY14_ExtraVariables = ["%s.vx.vy.vz" %  mainIDInput]
       
        
       ## combined / extrapolated muon track particles
       ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
       ##        are store in InDetTrackParticles collection)
       from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
       BPHY14_AllVariables   = getDefaultAllVariables()
       BPHY14_AllVariables += ["CombinedMuonTrackParticles"]
       BPHY14_AllVariables += ["ExtrapolatedMuonTrackParticles"]
        
       ## muon container
       BPHY14_AllVariables += ["Muons"]

       BPHY14_ExtraVariables   += ["%s.etcone30.etcone40" %  "Muons"
                                    +".momentumBalanceSignificance"
                                    +".scatteringCurvatureSignificance"
                                    +".scatteringNeighbourSignificance"
                                    +".msInnerMatchDOF.msInnerMatchChi2"
                                    +".msOuterMatchDOF.msOuterMatchChi2"
                                    +".EnergyLoss.ParamEnergyLoss.MeasEnergyLoss"
                                    +".ET_Core" ]
       #BPHY14_AllVariables += ["Muons"]
        
       ## Jpsi candidates
       BPHY14_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY14_Reco_mumu.OutputVtxContainerName]
       BPHY14_StaticContent += ["xAOD::VertexAuxContainer#%sAux" % BPHY14_Reco_mumu.OutputVtxContainerName]
       ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
       BPHY14_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY14_Reco_mumu.OutputVtxContainerName]
        
       # Truth information for MC only
       if isSimulation:
           BPHY14_StaticContent += ["xAOD::TruthParticleContainer#TruthMuons","xAOD::TruthParticleAuxContainer#TruthMuonsAux."]
           BPHY14_StaticContent += ["xAOD::TruthParticleContainer#TruthPhotons","xAOD::TruthParticleAuxContainer#TruthPhotonsAux."]
           BPHY14_AllVariables  += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

       BPHY14_AllVariables += ["Photons"] #,"Muons","InDetTrackParticles","PrimaryVertices"]

       BPHY14_SmartCollections += ["InDetTrackParticles"]
       BPHY14_AllVariables += ["GSFTrackParticles"] 
       BPHY14_ExtraVariables += ["%s.vx.vy.vz" % "InDetTrackParticles"] 
       # conversion vertices 
       BPHY14_ExtraVariables += [ 
                    "GSFConversionVertices.x.y.z.px.py.pz.pt1.pt2.etaAtCalo.phiAtCalo", 
                    "GSFConversionVertices.trackParticleLinks"
       ] 

       BPHY14_AllVariables = list(set(BPHY14_AllVariables)) # remove duplicates
   
       BPHY14SlimmingHelper.AllVariables = BPHY14_AllVariables
       BPHY14SlimmingHelper.StaticContent = BPHY14_StaticContent
       BPHY14SlimmingHelper.SmartCollections = BPHY14_SmartCollections
       BPHY14SlimmingHelper.ExtraVariables = BPHY14_ExtraVariables
       BPHY14ItemList = BPHY14SlimmingHelper.GetItemList()
       acc.merge(OutputStreamCfg(flags, "DAOD_BPHY14", ItemList=BPHY14ItemList, AcceptAlgs=["BPHY14Kernel"]))
       acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY14", AcceptAlgs=["BPHY14Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
       acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
       return acc
