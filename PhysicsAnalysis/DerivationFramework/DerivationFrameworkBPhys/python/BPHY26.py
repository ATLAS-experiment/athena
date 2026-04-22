# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#====================================================================
# BPHY26.py (Based on BPHY8, BPHY16, and BPHY13)
# Contact: yue.xu@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.Logging import logging
log_BPHY26 = logging.getLogger('BPHY26')

BPHYDerivationName = "BPHY26"
streamName = "StreamDAOD_BPHY26"

def BPHY26KernelCfg(flags, **kwargs):
    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    acc.getPrimaryAndMerge(PartPropSvcCfg(flags))

    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    BPHY26TriggerListsHelper = TriggerListsHelper(flags)
    
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(V0Tools)
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName)) # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    pvrefitter = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(pvrefitter)
    
    # mass limits and constants used in the following     
    Phimass = 1019.461
    Jpsimass = 3096.916
    Upsimass = 9460.30
    Wmass = 80377.
    
    Jpsimass_lower = 770.0
    Jpsimass_upper = 10000.0 #Three channels, Phi + pi, Jpsi + pi and Upsilon +pi, are actually considered
    Wmass_lower = 40000.
    Wmass_upper = 13000000
    
    BPHY26JpsiFinder = CompFactory.Analysis.JpsiFinder( # phi, psi, upsilon
        name                        = "BPHY26JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 2500.,
        invMassUpper                = Jpsimass_upper,
        invMassLower                = Jpsimass_lower,
        Chi2Cut                     = 30.,
        oppChargesOnly	            = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    
    acc.addPublicTool(BPHY26JpsiFinder)
        
    BPHY26_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY26_Reco_mumu",
        VertexSearchTool       = BPHY26JpsiFinder,
        OutputVtxContainerName = "BPHY26OniaCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        RefitPV                = True,
        PVRefitter             = pvrefitter,
        DoVertexType           = 1)
    #https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/BPhysPVTools.cxx#L259
    # bit pattern: doZ0BA|doZ0|doA0|doPt
    
      
    BPHY26Plus1Track = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY26Plus1Track",
        pionHypothesis                      = True,
        kaonHypothesis                      = False,
        trkThresholdPt                      = 3000,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsimass_lower,
        JpsiMassUpper                       = Jpsimass_upper,
        TrkTrippletMassLower                = Wmass_lower,
        TrkTrippletMassUpper                = Wmass_upper,
        Chi2Cut                             = 30.0,
        JpsiContainerKey                    = "BPHY26OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = False)
    
    acc.addPublicTool(BPHY26Plus1Track)

    BPHY26ThreeTrackSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY26ThreeTrackSelectAndWrite",
        VertexSearchTool         = BPHY26Plus1Track,
        OutputVtxContainerName   = "BPHY26ThreeTrack",
        PVContainerName          = "PrimaryVertices",
        RefPVContainerName       = "BPHY26RefPrimaryVertices1",
        RefitPV                  = True,
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        MaxPVrefit               = 10000,
        DoVertexType             = 7)
        
        
    BPHY26_Select_ThreeTrack      = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY26_Select_ThreeTrack",
        HypothesisName             = "ThreeTracks",
        InputVtxContainerName      = "BPHY26ThreeTrack",
        V0Tools                    = V0Tools,
        TrkMasses                  = [105.658, 105.658, 139.570],
        VtxMassHypo                = Wmass, 
        MassMin                    = Wmass_lower,
        MassMax                    = Wmass_upper,
        Chi2Max                    = 30.)

    #====================================================================
    # Isolation
    #====================================================================
    #Track isolation for candidates

    BPHY26TrackIsolationDecorator = CompFactory.DerivationFramework.VertexTrackIsolation(
            name                            = "BPHY26TrackIsolationDecorator",
            TrackIsoTool                    = "xAOD::TrackIsolationTool",
            TrackContainer                  = "InDetTrackParticles",
            InputVertexContainer            = "BPHY26ThreeTrack",
            PassFlags                       = ["passed_ThreeTracks"],
            DoIsoPerTrk                     = True,
            RemoveDuplicate                 = 2
            )

    #====================================================================
    # Revertex with mass constraint
    #====================================================================
    
    BPHY26_Revertex_phipi          = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY26_Revertex_phipi",
        InputVtxContainerName      = "BPHY26ThreeTrack",
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ], # Track indices start from 1 (not 0)! (https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Tracking/TrkVertexFitter/TrkVKalVrtCore/src/PrCFit.cxx#L170)
        RefitPV                    = True,
        RefPVContainerName         = "BPHY26phipiRefPrimaryVertices", # use existing refitted PVs
        UseMassConstraint          = False,
        SubVertexMass              = Phimass,
        MassInputParticles         = [105.658, 105.658, 139.570],
        TrkVertexFitterTool	       = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY26Revtx_phipi")
        
    BPHY26_Select_phipi          = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY26_Select_phipi",
        HypothesisName             = "phipi",
        InputVtxContainerName      = "BPHY26Revtx_phipi",
        TrkMasses                  = [105.658, 105.658, 139.570],
        VtxMassHypo                = Wmass,
        MassMin                    = Wmass_lower,
        MassMax                    = Wmass_upper,
        Chi2Max                    = 30.)
    
    BPHY26_Revertex_Jpsipi          = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY26_Revertex_Jpsipi",
        InputVtxContainerName      = "BPHY26ThreeTrack",
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = True,
        RefPVContainerName         = "BPHY26JpsipiRefPrimaryVertices",
        UseMassConstraint          = True,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [105.658, 105.658, 139.570],
        TrkVertexFitterTool	       = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY26Revtx_Jpsipi")
    
    BPHY26_Select_Jpsipi          = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY26_Select_Jpsipi",
        HypothesisName             = "Jpsipi",
        InputVtxContainerName      = "BPHY26Revtx_Jpsipi",
        TrkMasses                  = [105.658, 105.658, 139.570],
        VtxMassHypo                = Wmass,
        MassMin                    = Wmass_lower,
        MassMax                    = Wmass_upper,
        Chi2Max                    = 30.)
        
    BPHY26_Revertex_Upsipi          = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY26_Revertex_Upsipi",
        InputVtxContainerName      = "BPHY26ThreeTrack",
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = True,
        RefPVContainerName         = "BPHY26UpsipiRefPrimaryVertices",
        UseMassConstraint          = True,
        SubVertexMass              = Upsimass,
        MassInputParticles         = [105.658, 105.658, 139.570],
        TrkVertexFitterTool	       = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY26Revtx_Upsipi")
        
    BPHY26_Select_Upsipi          = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY26_Select_Upsipi",
        HypothesisName             = "Upsipi",
        InputVtxContainerName      = "BPHY26Revtx_Upsipi",
        TrkMasses                  = [105.658, 105.658, 139.570],
        VtxMassHypo                = Wmass,
        MassMin                    = Wmass_lower,
        MassMax                    = Wmass_upper,
        Chi2Max                    = 30.)    
    
    
    #--------------------------------------------------------------------
    ## 7/ select the event. We only want to keep events that contain certain vertices which passed certain selection.
    ##    This is specified by the "SelectionExpression" property, which contains the expression in the following format:
    ##
    ##       "ContainerName.passed_HypoName > count"
    ##
    ##    where "ContainerName" is output container from some Reco_* tool, "HypoName" is the hypothesis name setup in some "Select_*"
    ##    tool and "count" is the number of candidates passing the selection you want to keep. 

    # Common augmentations
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    acc.merge(PhysCommonAugmentationsCfg(
        flags,
        TriggerListsHelper = BPHY26TriggerListsHelper
    ))

    
    expression = "( count(BPHY26ThreeTrack.passed_ThreeTracks) > 0 && ( count(BPHY26Revtx_Jpsipi.passed_Jpsipi) + count(BPHY26Revtx_Upsipi.passed_Upsipi) + count(BPHY26Revtx_phipi.passed_phipi) ) > 0)"
    BPHY26_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY26_SelectEvent", expression = expression)
    
    augTools = [BPHY26_Reco_mumu, BPHY26ThreeTrackSelectAndWrite, BPHY26_Select_ThreeTrack]
    augTools += [BPHY26TrackIsolationDecorator, BPHY26_Revertex_phipi , BPHY26_Select_phipi, BPHY26_Revertex_Jpsipi, BPHY26_Select_Jpsipi , BPHY26_Revertex_Upsipi , BPHY26_Select_Upsipi]
    skimTools = [BPHY26_SelectEvent]
    for t in  augTools +skimTools : acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY26Kernel",
                                                    AugmentationTools = augTools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = skimTools,
                                                    ThinningTools     = []))
    
    
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg
    acc.merge(DerivationTrackIsoCfg(flags, object_types=("Muons")))
    
    # IFF augmentation - Adding Lepton Taggers
    #from LeptonTaggers.LeptonTaggersConfig import DecoratePLITAlgsCfg
    #acc.merge(DecoratePLITAlgsCfg(flags))
    
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByAlgsCfg
    contNames = [ "Muons"]
    acc.merge(IsoCloseByAlgsCfg(flags, isPhysLite = False, containerNames = contNames, useSelTools = True, stream_name = "StreamDAOD_BPHY26"))

    ## FTAG augmentations - run b-tagging on PFlow jets
    from BTagging.FlavorTaggingConfig import FlavorTaggingCfg
    acc.merge(FlavorTaggingCfg(flags, "AntiKt4EMPFlowJets"), sequenceName="BPHY26Sequence")
    
    
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    BPHY26SlimmingHelper = SlimmingHelper("BPHY26SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)

    BPHY26SlimmingHelper.SmartCollections = ["EventInfo",
                                           "Electrons",
                                           "Photons",
                                           "Muons",
                                           "PrimaryVertices",
                                           "InDetTrackParticles",
                                           "AntiKt4EMTopoJets",
                                           "AntiKt4EMPFlowJets",
                                           "MET_Baseline_AntiKt4EMTopo",
                                           "MET_Baseline_AntiKt4EMPFlow",
                                           "TauJets",
                                           "TauJets_MuonRM",
                                           "DiTauJets",
                                           "DiTauJetsLowPt",
                                           "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
                                           "AntiKtVR30Rmax4Rmin02PV0TrackJets",
                                          ]

    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY26_AllVariables  = getDefaultAllVariables()
    BPHY26_StaticContent = []
    
    
    ## primary vertices
    BPHY26_AllVariables += ["PrimaryVertices"]
    BPHY26_StaticContent += ["xAOD::VertexContainer#BPHY26RefPrimaryVertices1"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#BPHY26RefPrimaryVertices1Aux."]
    BPHY26_StaticContent += ["xAOD::VertexContainer#BPHY26phipiRefPrimaryVertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#BPHY26phipiRefPrimaryVerticesAux."]
    BPHY26_StaticContent += ["xAOD::VertexContainer#BPHY26JpsipiRefPrimaryVertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#BPHY26JpsipiRefPrimaryVerticesAux."]
    BPHY26_StaticContent += ["xAOD::VertexContainer#BPHY26UpsipiRefPrimaryVertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#BPHY26UpsipiRefPrimaryVerticesAux."]
    
    ## ID track particles
    BPHY26_AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    BPHY26_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    BPHY26_AllVariables += ["Muons","MuonSegments"]
    
    BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Reco_mumu.OutputVtxContainerName]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Reco_mumu.OutputVtxContainerName]
    
    BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26ThreeTrackSelectAndWrite.OutputVtxContainerName]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26ThreeTrackSelectAndWrite.OutputVtxContainerName]
    
    BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_phipi.OutputVtxContainerName]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_phipi.OutputVtxContainerName]
    
    BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_Jpsipi.OutputVtxContainerName]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_Jpsipi.OutputVtxContainerName]
    
    BPHY26_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY26_Revertex_Upsipi.OutputVtxContainerName]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY26_Revertex_Upsipi.OutputVtxContainerName]

    excludedVertexAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
    BPHY26_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Tight_Vertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Tight_VerticesAux." + excludedVertexAuxData]
    BPHY26_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Medium_Vertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Medium_VerticesAux." + excludedVertexAuxData]
    BPHY26_StaticContent += ["xAOD::VertexContainer#SoftBVrtClusterTool_Loose_Vertices"]
    BPHY26_StaticContent += ["xAOD::VertexAuxContainer#SoftBVrtClusterTool_Loose_VerticesAux." + excludedVertexAuxData]

    
    
    # Truth content
    if flags.Input.isMC:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import addTruth3ContentToSlimmerTool
        addTruth3ContentToSlimmerTool(BPHY26SlimmingHelper)
        BPHY26SlimmingHelper.ExtraVariables += ["Electrons.TruthLink","Muons.TruthLink","Photons.TruthLink","AntiKt4TruthDressedWZJets.IsoFixedCone5Pt"]

        BPHY26_AllVariables += ["TruthLHEParticles","TruthHFWithDecayParticles","TruthHFWithDecayVertices","TruthCharm","TruthPileupParticles","InTimeAntiKt4TruthJets","OutOfTimeAntiKt4TruthJets",
                         "TruthPrimaryVertices","TruthEvents","TruthParticles","TruthVertices","TruthElectrons","TruthMuons","TruthTaus"]

        from DerivationFrameworkMCTruth.MCTruthCommonConfig import AddTauAndDownstreamParticlesCfg
        acc.merge(AddTauAndDownstreamParticlesCfg(flags))
        BPHY26_AllVariables += ["TruthTausWithDecayParticles","TruthTausWithDecayVertices"]

        BPHY26SlimmingHelper.SmartCollections += [
            "AntiKt4TruthDressedWZJets",
            "AntiKt4TruthWZJets",
            "AntiKt4TruthJets"
        ]

    
    MuonsExtraContent = [ ".".join( [
        "Muons",
        "MeasEnergyLoss.MeasEnergyLossSigma.EnergyLossSigma.ParamEnergyLoss",
        "ParamEnergyLossSigmaMinus.ParamEnergyLossSigmaPlus.clusterLink.scatteringCurvatureSignificance",
        "deltaPhiRescaled2.deltaPhiFromLastMeasurement.scatteringNeighbourSignificance",
        "ptcone20.ptcone30.ptcone40.ptvarcone20.ptvarcone30.ptvarcone40.topoetcone30",
        "neflowisol20.neflowisol30.neflowisol40.ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptvarcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptvarcone40_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone20_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone30_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptcone30_Nonprompt_All_MaxWeightTTVA_pt1000.ptcone40_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptcone40_Nonprompt_All_MaxWeightTTVA_pt1000",
        "msInnerMatchChi2", "isoSelIsOK", "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr", "neflowisol20_CloseByCorr", "topoetcone20_CloseByCorr"
    ] ) ]
    
    BPHY26SlimmingHelper.ExtraVariables += ["AntiKt4EMTopoJets.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.IsoFixedCone5PtPUsub",
                                              "AntiKt4EMPFlowJets.QGTransformer_ConstScore.DFCommonJets_QGTagger_truthjet_nCharged.DFCommonJets_QGTagger_truthjet_pt.DFCommonJets_QGTagger_truthjet_eta.DFCommonJets_QGTagger_NTracks.DFCommonJets_QGTagger_TracksWidth.DFCommonJets_QGTagger_TracksC1.ConeExclBHadronsFinal.ConeExclCHadronsFinal.GhostBHadronsFinal.GhostCHadronsFinal.GhostBHadronsFinalCount.GhostBHadronsFinalPt.GhostCHadronsFinalCount.GhostCHadronsFinalPt.isJvtHS.isJvtPU.IsoFixedCone5PtPUsub",
                                              "TruthPrimaryVertices.t.x.y.z",
                                              "InDetTrackParticles.TTVA_AMVFVertices.TTVA_AMVFWeights.eProbabilityHT.numberOfTRTHits.numberOfTRTOutliers",
                                              "EventInfo.GenFiltHT.GenFiltMET.GenFiltHTinclNu.GenFiltPTZ.GenFiltFatJ.HF_Classification.HF_SimpleClassification",
                                              "TauJets.dRmax.etOverPtLeadTrk",
                                              "TauJets_MuonRM.dRmax.etOverPtLeadTrk",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET.ex.ey",
                                              "HLT_xAOD__TrigMissingETContainer_TrigEFMissingET_mht.ex.ey"]

    BPHY26SlimmingHelper.ExtraVariables += MuonsExtraContent

    from IsolationSelection.IsolationSelectionConfig import setupIsoCloseBySlimmingVariables
    setupIsoCloseBySlimmingVariables(BPHY26SlimmingHelper)

    BPHY26SlimmingHelper.AllVariables = BPHY26_AllVariables
    BPHY26SlimmingHelper.StaticContent = BPHY26_StaticContent

    # Needed for trigger objects
    BPHY26SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY26SlimmingHelper.IncludeBPhysTriggerContent = True

    # Trigger matching
    # Run 2
    if flags.Trigger.EDMVersion == 2:
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddRun2TriggerMatchingToSlimmingHelper
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = BPHY26SlimmingHelper,
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = BPHY26TriggerListsHelper.Run2TriggerNamesTau)
        AddRun2TriggerMatchingToSlimmingHelper(SlimmingHelper = BPHY26SlimmingHelper,
                                               OutputContainerPrefix = "TrigMatch_",
                                               TriggerList = BPHY26TriggerListsHelper.Run2TriggerNamesNoTau)

    # Run 3, or Run 2 with navigation conversion
    if flags.Trigger.EDMVersion == 3 or (flags.Trigger.EDMVersion == 2 and flags.Trigger.doEDMVersionConversion):
        from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import AddRun3TrigNavSlimmingCollectionsToSlimmingHelper
        AddRun3TrigNavSlimmingCollectionsToSlimmingHelper(BPHY26SlimmingHelper)

    # L1 trigger objects
    from Campaigns.Utils import getDataYear
    if getDataYear(flags) >= 2024:
        # Run 3 with Phase I jet RoIs.
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddjFexRoIsToSlimmingHelper
        AddjFexRoIsToSlimmingHelper(SlimmingHelper = BPHY26SlimmingHelper)
    elif getDataYear(flags) >= 2015:
        # Run 2 and early Run 3, legacy L1 RoIs
        from DerivationFrameworkPhys.TriggerMatchingCommonConfig import AddLegacyL1JetRoIsToSlimmingHelper
        AddLegacyL1JetRoIsToSlimmingHelper(SlimmingHelper = BPHY26SlimmingHelper)

    
    BPHY26ItemList = BPHY26SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY26", ItemList=BPHY26ItemList, AcceptAlgs=["BPHY26Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY26", AcceptAlgs=["BPHY26Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc

def BPHY26Cfg(flags):
    log_BPHY26.info('****************** STARTING BPHY26 ******************')
    acc = ComponentAccumulator()
    # Get the lists of triggers needed for trigger matching.
    # This is needed at this scope (for the slimming) and further down in the config chain
    # for actually configuring the matching, so we create it here and pass it down
    # TODO: this should ideally be called higher up to avoid it being run multiple times in a train

    # dedicated augmentations
    acc.merge(BPHY26KernelCfg(flags, name="BPHY26Kernel", StreamName = streamName))

    return acc

