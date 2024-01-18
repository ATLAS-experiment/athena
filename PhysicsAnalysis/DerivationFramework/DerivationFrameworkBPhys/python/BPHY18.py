# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY18.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY18"
streamName = "StreamDAOD_BPHY18"

def BPHY18Cfg(ConfigFlags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    from DerivationFrameworkEGamma.EGammaCommonConfig import EGammaCommonCfg
    acc = ComponentAccumulator()
    acc.merge(EGammaCommonCfg(ConfigFlags))
    isSimulation = ConfigFlags.Input.isMC
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(ConfigFlags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(ConfigFlags, BPHYDerivationName))        # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(ConfigFlags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(ConfigFlags, BPHYDerivationName))
    acc.addPublicTool(vpest)

    #BPHY18TriggerSkim = CompFactory.DerivationFramework.TriggerSkimmingTool(name = "BPHY18TriggerSkim",
#                                                             TriggerListOR = triggerList,
#                                                            TriggerListORHLTOnly = triggerList_unseeded )
    
    ElectronLHSelectorLHvloose_nod0 = CompFactory.AsgElectronLikelihoodTool("ElectronLHSelectorLHvloosenod0", 
            primaryVertexContainer = "PrimaryVertices",
            ConfigFile="ElectronPhotonSelectorTools/offline/mc20_20230321/ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth_NoD0_NoPix.conf")

    # decorate electrons with the output of LH
       
    ElectronPassLHvloosenod0 = CompFactory.DerivationFramework.EGElectronLikelihoodToolWrapper(name = "ElectronPassLHvloosenod0",
                                            EGammaElectronLikelihoodTool = ElectronLHSelectorLHvloose_nod0,
                                            EGammaFudgeMCTool = "",
                                            CutType = "",
                                            StoreGateEntryName = "DFCommonElectronsLHVeryLoosenod0",
                                            ContainerName = "Electrons",
                                            StoreTResult=False)
    
    BPHY18DiElectronFinder = CompFactory.Analysis.JpsiFinder_ee(
                             name                        = "BPHY18DiElectronFinder",
                             elAndEl                     = True,
                             elAndTrack                  = False,
                             TrackAndTrack               = False,
                             assumeDiElectrons           = True,
                             elThresholdPt               = 4000.0,
                             invMassUpper                = 7000.0,
                             invMassLower                = 1.0,
                             Chi2Cut                     = 30.,
                             oppChargesOnly              = False,
                             allChargeCombinations       = True,
                             useElectronTrackMeasurement = True,
                             electronCollectionKey       = "Electrons",
                             TrackParticleCollection     = "GSFTrackParticles",
                             useEgammaCuts               = True,
                             V0VertexFitterTool          = None,
                             useV0Fitter                 = False,
                             TrkVertexFitterTool         = vkalvrt,
                             TrackSelectorTool           = trackselect,
                             VertexPointEstimator        = vpest,
                             ElectronSelection             = "d0_or_nod0"
                             )
    extraTools = [BPHY18DiElectronFinder]
    BPHY18DiElectronSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
                            name                   = "BPHY18DiElectronSelectAndWrite",
                            VertexSearchTool       = BPHY18DiElectronFinder,
                            OutputVtxContainerName = "BPHY18DiElectronCandidates",
                            PVContainerName        = "PrimaryVertices",
                            V0Tools                = V0Tools,
                            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(ConfigFlags)),
                            RefPVContainerName     = "SHOULDNOTBEUSED",
                            DoVertexType           = 7
                            )

    BPHY18_Select_DiElectrons = CompFactory.DerivationFramework.Select_onia2mumu(
                            name                  = "BPHY18_Select_DiElectrons",
                            HypothesisName        = "Jpsi",
                            InputVtxContainerName = "BPHY18DiElectronCandidates",
                            V0Tools               = V0Tools,
                            VtxMassHypo           = 3096.916,
                            MassMin               = 1.0,
                            MassMax               = 7000.0,
                            Chi2Max               = 30,
                            DoVertexType          = 7
                            )

    BPHY18BeeKst = CompFactory.Analysis.JpsiPlus2Tracks(
                            name                    = "BPHY18BeeKstFinder",
                            kaonkaonHypothesis      = False,
                            pionpionHypothesis      = False,
                            kaonpionHypothesis      = True,
                            oppChargesOnly          = False,
                            SameChargesOnly         = False,
                            trkThresholdPt          = 500.0,
                            trkMaxEta                   = 3.0,
                            BThresholdPt            = 1000.,
                            BMassLower              = 3000.0,
                            BMassUpper                = 6500.0,
                            JpsiContainerKey          = "BPHY18DiElectronCandidates",
                            TrackParticleCollection = "InDetTrackParticles",
                            ExcludeCrossJpsiTracks  = False,
                            TrkVertexFitterTool     = vkalvrt,
                            TrackSelectorTool         = trackselect,
                            UseMassConstraint         = False,
                            DiTrackMassUpper        = 1110.,
                            DiTrackMassLower        = 690.,
                            Chi2Cut                 = 15.0,
                            DiTrackPt               = 500.,
                            TrkQuadrupletMassLower  = 1000.0,
                            TrkQuadrupletMassUpper  = 10000.0,
                            FinalDiTrackPt          = 500.,
                            UseGSFTrackIndices      = [0,1]
                            )
    BPHY18BeeKstSelectAndWrite  = CompFactory.DerivationFramework.Reco_Vertex(
                            name                   = "BPHY18BeeKstSelectAndWrite",
                            VertexSearchTool       = BPHY18BeeKst,
                            OutputVtxContainerName = "BeeKstCandidates",
                            PVContainerName        = "PrimaryVertices",
                            RefPVContainerName     = "BPHY18RefittedPrimaryVertices",
                            RefitPV                = True,
                            V0Tools                = V0Tools,
                            PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(ConfigFlags)),
                            MaxPVrefit             = 10000,
                            DoVertexType           = 7
                            )

    BPHY18_Select_BeeKst = CompFactory.DerivationFramework.Select_onia2mumu(
                           name                  = "BPHY18_Select_BeeKst",
                           HypothesisName        = "Bd",
                           InputVtxContainerName = "BeeKstCandidates",
                           V0Tools               = V0Tools,
                           TrkMasses             = [0.511, 0.511, 493.677, 139.570],
                           VtxMassHypo           = 5279.6,
                           MassMin               = 1.0,
                           MassMax               = 10000.0,
                           Chi2Max               = 30.0
                           )

    BPHY18_Select_BeeKstbar = CompFactory.DerivationFramework.Select_onia2mumu(
                           name                  = "BPHY18_Select_Bd2JpsiKstbar",
                           HypothesisName        = "Bdbar",
                           InputVtxContainerName = "BeeKstCandidates",
                           V0Tools               = V0Tools,
                           TrkMasses             = [0.511, 0.511, 139.570, 493.677],
                           VtxMassHypo           = 5279.6,
                           MassMin               = 1.0,
                           MassMax               = 10000.0,
                           Chi2Max               = 30.0
                           )


    BPHY18_diMeson_revertex = CompFactory.DerivationFramework.ReVertex(
                           name                   = "BPHY18_diMeson_revertex",
                           InputVtxContainerName  = "BeeKstCandidates",
                           V0Tools                = V0Tools,
                           PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(ConfigFlags)),
                           TrackIndices           = [ 2, 3 ],
                           TrkVertexFitterTool    = vkalvrt,
                           OutputVtxContainerName = "BPHY18DiMeson"
                           )

    BPHY18_Select_Kpi = CompFactory.DerivationFramework.Select_onia2mumu(
                          name                  = "BPHY18_Select_Kpi",
                          HypothesisName        = "Kpi",
                          InputVtxContainerName = "BPHY18DiMeson",
                          V0Tools               = V0Tools,
                          TrkMasses             = [ 493.677, 139.570 ],
                          VtxMassHypo           = 891.66,
                          MassMin               = 1.0,
                          MassMax               = 100000.0,
                          Chi2Max               = 100.0
                          )

    BPHY18_Select_piK = CompFactory.DerivationFramework.Select_onia2mumu(
                       name                  = "BPHY18_Select_piK",
                       HypothesisName        = "piK",
                       InputVtxContainerName = "BPHY18DiMeson",
                       V0Tools               = V0Tools,
                       TrkMasses             = [ 139.570, 493.677 ],
                       VtxMassHypo           = 891.66,
                       MassMin               = 1.0,
                       MassMax               = 100000.0,
                       Chi2Max               = 100.0
                       )

    BPHY18_SelectBeeKstEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(
                       name = "BPHY18_SelectBeeKstEvent",
                       expression = "(count(BeeKstCandidates.passed_Bd > 0) + count(BeeKstCandidates.passed_Bdbar > 0)) > 0")

    BPHY18SkimmingAND = CompFactory.DerivationFramework.FilterCombinationAND(
        "BPHY18SkimmingAND",
        #FilterList = [BPHY18_SelectBeeKstEvent, BPHY18TriggerSkim])   # TODO: Need to update the trigger names
        FilterList = [BPHY18_SelectBeeKstEvent])
    extraTools += [BPHY18_SelectBeeKstEvent, BPHY18SkimmingAND]
    BPHY18_thinningTool_Tracks = CompFactory.DerivationFramework.Thin_vtxTrk(
                                name                       = "BPHY18_thinningTool_Tracks",
                                TrackParticleContainerName = "InDetTrackParticles",
                                StreamName = streamName,
                                VertexContainerNames       = ["BeeKstCandidates"],
                                PassFlags                  = ["passed_Bd", "passed_Bdbar"] )

    BPHY18_thinningTool_GSFTracks = CompFactory.DerivationFramework.Thin_vtxTrk(
                                name                       = "BPHY18_thinningTool_GSFTracks",
                                TrackParticleContainerName = "GSFTrackParticles",
                                StreamName = streamName,
                                VertexContainerNames       = ["BeeKstCandidates"],
                                PassFlags                  = ["passed_Bd", "passed_Bdbar"] )

    BPHY18_thinningTool_PV = CompFactory.DerivationFramework.BPhysPVThinningTool(
                                name                 = "BPHY18_thinningTool_PV",
                                StreamName = streamName,
                                CandidateCollections = ["BeeKstCandidates"],
                                KeepPVTracks         = True
                                )

    BPHY18MuonTPThinningTool = CompFactory.DerivationFramework.MuonTrackParticleThinning(
                               name                    = "BPHY18MuonTPThinningTool",
                               StreamName = streamName,
                               MuonKey                 = "Muons",
                               InDetTrackParticlesKey  = "InDetTrackParticles")

    BPHY18EgammaTPThinningTool = CompFactory.DerivationFramework.EgammaTrackParticleThinning(
                              name                   = "BPHY18EgammaTPThinningTool",
                              StreamName = streamName,
                              SGKey                  = "Electrons",
                              InDetTrackParticlesKey = "InDetTrackParticles")

    BPHY18TruthThinTool = CompFactory.DerivationFramework.GenericTruthThinning(name     = "BPHY18TruthThinTool",
                                                                ParticleSelectionString = "abs(TruthParticles.pdgId) == 11 || abs(TruthParticles.pdgId) == 13 || abs(TruthParticles.pdgId) == 10311 || abs(TruthParticles.pdgId) == 521 || abs(TruthParticles.pdgId) == 523 || TruthParticles.pdgId == 511 || TruthParticles.pdgId == 513",
                                                                PreserveDescendants     = True,
                                                                StreamName = streamName,
                                                                PreserveAncestors       = True)

    BPHY18TruthThinNoChainTool = CompFactory.DerivationFramework.GenericTruthThinning(name = "BPHY18TruthThinNoChainTool",
                                                           ParticleSelectionString = "abs(TruthParticles.pdgId) == 5 || abs(TruthParticles.pdgId) == 12 || abs(TruthParticles.pdgId) == 14",
                                                           PreserveDescendants     = False,
                                                           StreamName = streamName,
                                                           PreserveAncestors       = False)

    thinningCollection = [ BPHY18_thinningTool_Tracks,  BPHY18_thinningTool_GSFTracks,
                       BPHY18_thinningTool_PV, #BPHY18_thinningTool_PV_GSF, 
                       BPHY18EgammaTPThinningTool, BPHY18MuonTPThinningTool
                     ]
    

    augTools = [ElectronPassLHvloosenod0,
                BPHY18DiElectronSelectAndWrite, BPHY18_Select_DiElectrons,
                BPHY18BeeKstSelectAndWrite, BPHY18_Select_BeeKst, BPHY18_Select_BeeKstbar,
                BPHY18_diMeson_revertex, BPHY18_Select_Kpi, BPHY18_Select_piK]
    skimTools = [BPHY18SkimmingAND]
    
    if isSimulation:
        thinningCollection += [BPHY18TruthThinTool,BPHY18TruthThinNoChainTool]        

    for t in augTools + skimTools + thinningCollection + extraTools: acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY18Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = skimTools,
                                                    ThinningTools     = thinningCollection))
    
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg
    acc.merge(DerivationTrackIsoCfg(ConfigFlags, object_types=("Electrons", "Muons")))

    
    #====================================================================
    # Slimming 
    #====================================================================
    
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY18SlimmingHelper = SlimmingHelper("BPHY18SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    AllVariables  = getDefaultAllVariables()
    StaticContent  = []
    ExtraVariables = []
    BPHY18SlimmingHelper.SmartCollections = ["Electrons", "Muons", "InDetTrackParticles" ] 
    
    # Needed for trigger objects
    BPHY18SlimmingHelper.IncludeMuonTriggerContent   = False
    BPHY18SlimmingHelper.IncludeBPhysTriggerContent  = False
    BPHY18SlimmingHelper.IncludeEGammaTriggerContent = True
    ## primary vertices
    AllVariables  += ["PrimaryVertices"]
    StaticContent += ["xAOD::VertexContainer#BPHY18RefittedPrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#BPHY18RefittedPrimaryVerticesAux."]
    
    ExtraVariables += ["Muons.etaLayer1Hits.etaLayer2Hits.etaLayer3Hits.etaLayer4Hits.phiLayer1Hits.phiLayer2Hits.phiLayer3Hits.phiLayer4Hits",
                       "Muons.numberOfTriggerEtaLayers.numberOfPhiLayers",
                       "InDetTrackParticles.numberOfTRTHits.numberOfTRTHighThresholdHits.vx.vy.vz.pixeldEdx",
                       "PrimaryVertices.chiSquared.covariance", 
                       "Electrons.deltaEta1.DFCommonElectronsLHVeryLoosenod0.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500.ptvarcone40_Nonprompt_All_MaxWeightTTVALooseCone_pt1000.ptvarcone20_Nonprompt_All_MaxWeightTTVALooseCone_pt1000",
                       "egammaClusters.calE.calEta.calPhi.e_sampl.eta_sampl.etaCalo.phiCalo.ETACALOFRAME.PHICALOFRAME",
                       "HLT_xAOD__ElectronContainer_egamma_ElectronsAuxDyn.charge"]
    
    ## Jpsi candidates 
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY18DiElectronSelectAndWrite.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY18DiElectronSelectAndWrite.OutputVtxContainerName]
    
    StaticContent += ["xAOD::VertexContainer#%s"        %                 BPHY18BeeKstSelectAndWrite.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY18BeeKstSelectAndWrite.OutputVtxContainerName]
    
    StaticContent += ["xAOD::VertexContainer#%s"        % BPHY18_diMeson_revertex.OutputVtxContainerName]
    StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY18_diMeson_revertex.OutputVtxContainerName]
    
    AllVariables += [ "GSFTrackParticles"] 


    # Truth information for MC only
    if isSimulation:
        AllVariables += ["TruthEvents","TruthParticles","TruthVertices", "ElectronTruthParticles"]
    
    AllVariables = list(set(AllVariables)) # remove duplicates
    
    BPHY18SlimmingHelper.AllVariables = AllVariables
    BPHY18SlimmingHelper.ExtraVariables = ExtraVariables
    
    BPHY18SlimmingHelper.StaticContent = StaticContent
    
    from DerivationFrameworkEGamma.ElectronsCPDetailedContent import ElectronsCPDetailedContent, GSFTracksCPDetailedContent
    BPHY18SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent
    BPHY18SlimmingHelper.ExtraVariables += GSFTracksCPDetailedContent
    
    BPHY18ItemList = BPHY18SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_BPHY18", ItemList=BPHY18ItemList, AcceptAlgs=["BPHY18Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(ConfigFlags, "DAOD_BPHY18", AcceptAlgs=["BPHY18Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
