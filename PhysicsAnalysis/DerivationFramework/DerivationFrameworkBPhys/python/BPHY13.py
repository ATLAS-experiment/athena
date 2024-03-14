# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY13.py
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory


BPHYDerivationName = "BPHY13"
streamName = "StreamDAOD_BPHY13"

def BPHY13Cfg(flags):
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

    BPHY13JpsiFinder = CompFactory.Analysis.JpsiFinder(
     name                        = "BPHY13JpsiFinder",
     muAndMu                     = True,
     muAndTrack                  = False,
     TrackAndTrack               = False,
     assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
     trackThresholdPt            = 2500.,
     invMassUpper                = 12500.,
     invMassLower                = 2000.,
     Chi2Cut                     = 200.,
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
    acc.addPublicTool(BPHY13JpsiFinder)
    BPHY13_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_mumu",
        VertexSearchTool       = BPHY13JpsiFinder,
        OutputVtxContainerName = "BPHY13OniaCandidates",
        PVContainerName        = "PrimaryVertices",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        RefPVContainerName     = "SHOULDNOTBEUSED",
    #https://gitlab.cern.ch/atlas/athena/-/blob/21.2/PhysicsAnalysis/DerivationFramework/DerivationFrameworkBPhys/src/BPhysPVTools.cxx#L259
    # bit pattern: doZ0BA|doZ0|doA0|doPt
        DoVertexType           = 1)
    
    BPHY13Plus2Tracks = CompFactory.Analysis.JpsiPlus2Tracks(
       name = "BPHY13Plus2Tracks",
       kaonkaonHypothesis              = False,
       pionpionHypothesis              = False,
       kaonpionHypothesis              = False,
       ManualMassHypo                  = [ 105.658, 105.658, 105.658, 105.658 ],
       trkThresholdPt                  = 1500.,
       trkMaxEta                       = 2.5,
       oppChargesOnly                  = False,
       DiTrackMassUpper                = 12500.,
       DiTrackMassLower                = 2000.,
       TrkQuadrupletMassUpper          = 25000.,
       TrkQuadrupletMassLower          = 0.,
       Chi2Cut                         = 200.,
       JpsiContainerKey                = "BPHY13OniaCandidates",
       TrackParticleCollection         = "InDetTrackParticles",
       MuonsUsedInJpsi                 = "Muons",
       ExcludeJpsiMuonsOnly            = True,
       RequireNMuonTracks              = 1,
       TrkVertexFitterTool             = vkalvrt,
       TrackSelectorTool               = trackselect,
       UseMassConstraint               = False)
    acc.addPublicTool(BPHY13Plus2Tracks)
    BPHY13FourTrackSelectAndWrite = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY13FourTrackSelectAndWrite",
        VertexSearchTool         = BPHY13Plus2Tracks,
        OutputVtxContainerName   = "BPHY13FourTrack",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        RefPVContainerName       = "BPHY13RefittedPrimaryVertices1",
        RefitPV                  = True,
        MaxPVrefit               = 10000,
        DoVertexType             = 7)
    raise Exception("Fix passing of blinding information")
    doBlinding = False
    doUnblinding1 = False
    doUnblinding2 = False
    do_blinding = 'doBlinding' in vars() and doBlinding is True and not isSimulation
    do_unblinding1 = not do_blinding and 'doUnblinding1' in vars() and doUnblinding1 is True and not isSimulation
    do_unblinding2 = not do_blinding and 'doUnblinding2' in vars() and doUnblinding2 is True and not isSimulation

    if do_blinding:
        #
        # select 4 regions (before unblinding)
        #
        BPHY13_Select1_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
            name                       = "BPHY13_Select1_FourTrack",
            HypothesisName             = "FourTracks1",
            InputVtxContainerName      = "BPHY13FourTrack",
            V0Tools                  = V0Tools,
            TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
            VtxMassHypo                = 6900.0, # for decay time
            MassMin                    = 7500.,
            MassMax                    = 9000.,
            Chi2Max                    = 200.)
        
       
        BPHY13_Select2_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
            name                       = "BPHY13_Select2_FourTrack",
            HypothesisName             = "FourTracks2",
            InputVtxContainerName      = "BPHY13FourTrack",
            V0Tools                  = V0Tools,
            TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
            VtxMassHypo                = 6900.0, # for decay time
            MassMin                    = 10000.,
            MassMax                    = 12000.,
            Chi2Max                    = 200.)
       
        BPHY13_Select3_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
            name                       = "BPHY13_Select3_FourTrack",
            HypothesisName             = "FourTracks3",
            InputVtxContainerName      = "BPHY13FourTrack",
            V0Tools                  = V0Tools,
            TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
            VtxMassHypo                = 6900.0,  #for decay time
            MassMin                    = 14000.,
            MassMax                    = 17500.,
            Chi2Max                    = 200.)
       
        BPHY13_Select4_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
            name                       = "BPHY13_Select4_FourTrack",
            HypothesisName             = "FourTracks4",
            InputVtxContainerName      = "BPHY13FourTrack",
            V0Tools                  = V0Tools,
            TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
            VtxMassHypo                = 6900.0,  #for decay time
            MassMin                    = 19500.,
            MassMax                    = 25000.,
            Chi2Max                    = 200.)
       
    elif do_unblinding1 or do_unblinding2:
     if do_unblinding1:
         #
         # select 2 regions (unblinding part 1)
         #
         BPHY13_Select1_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
             name                       = "BPHY13_Select1_FourTrack",
             HypothesisName             = "FourTracks1",
             InputVtxContainerName      = "BPHY13FourTrack",
             V0Tools                    = V0Tools,
             TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
             VtxMassHypo                = 6900.0, # for decay time
             MassMin                    = 0.,
             MassMax                    = 7500.,
             Chi2Max                    = 200.)
     
    
         BPHY13_Select2_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
             name                       = "BPHY13_Select2_FourTrack",
             HypothesisName             = "FourTracks2",
             InputVtxContainerName      = "BPHY13FourTrack",
             V0Tools                    = V0Tools,
             TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
             VtxMassHypo                = 6900.0, # for decay time
             MassMin                    = 9000.,
             MassMax                    = 10000.,
             Chi2Max                    = 200.)
    
     if do_unblinding2:
         #
         # select 2 regions (unblinding part 2)
         #
         BPHY13_Select3_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
             name                       = "BPHY13_Select3_FourTrack",
             HypothesisName             = "FourTracks3",
             InputVtxContainerName      = "BPHY13FourTrack",
             V0Tools                    = V0Tools,
             TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
             VtxMassHypo                = 6900.0,  #for decay time
             MassMin                    = 12000.,
             MassMax                    = 14000.,
             Chi2Max                    = 200.)
    
    
         BPHY13_Select4_FourTrack     = CompFactory.DerivationFramework.Select_onia2mumu(
             name                       = "BPHY13_Select4_FourTrack",
             HypothesisName             = "FourTracks4",
             InputVtxContainerName      = "BPHY13FourTrack",
             V0Tools                    = V0Tools,
             TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
             VtxMassHypo                = 6900.0,  #for decay time
             MassMin                    = 17500.,
             MassMax                    = 19500.,
             Chi2Max                    = 200.)
    
    else:
        BPHY13_Select_FourTrack      = CompFactory.DerivationFramework.Select_onia2mumu(
         name                       = "BPHY13_Select_FourTrack",
         HypothesisName             = "FourTracks",
         InputVtxContainerName      = "BPHY13FourTrack",
         V0Tools                    = V0Tools,
         TrkMasses                  = [105.658, 105.658, 105.658, 105.658],
         VtxMassHypo                = 6900.0, # for decay time
         MassMin                    = 0.,
         MassMax                    = 25000.,
         Chi2Max                    = 200.)

    BPHY13TrackIsolationDecorator = CompFactory.DerivationFramework.VertexTrackIsolation(
      name                            = "BPHY13TrackIsolationDecorator",
      TrackIsoTool                    = "xAOD::TrackIsolationTool",
      TrackContainer                  = "InDetTrackParticles",
      InputVertexContainer            = "BPHY13FourTrack",
      PassFlags                       = ["passed_FourTracks","passed_FourTracks1","passed_FourTracks2","passed_FourTracks3","passed_FourTracks4"],
      DoIsoPerTrk                     = True,
      RemoveDuplicate                 = 2
    )

    BPHY13_Revertex_2mu            = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2mu",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = True,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices2", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        VertexMass                 = 3096.916,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoMuon")

    BPHY13_Select_TwoMuon          = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoMuon",
        HypothesisName             = "TwoMuons",
        InputVtxContainerName      = "BPHY13TwoMuon",
        TrkMasses                  = [105.658, 105.658],
        V0Tools                    = V0Tools,
        VtxMassHypo                = 3096.916,
        MassMin                    = 2000.,
        MassMax                    = 3600.,
        Chi2Max                    = 200)

    BPHY13_Revertex_2trk           = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2trk",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 2, 3 ],
        RefitPV                    = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices3", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        VertexMass                 = 3096.916,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoTrack")

    BPHY13_Select_TwoTrack         = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoTrack",
        HypothesisName             = "TwoTracks",
        InputVtxContainerName      = "BPHY13TwoTrack",
        TrkMasses                  = [105.658, 105.658],
        V0Tools                    = V0Tools,
        VtxMassHypo                = 3096.916,
        MassMin                    = 2000.,
        MassMax                    = 3600.,
        Chi2Max                    = 200)

    BPHY13_Revertex_2muHi          = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2muHi",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices4", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        VertexMass                 = 9460.30,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoMuonHi")

    BPHY13_Select_TwoMuonHi        = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoMuonHi",
        HypothesisName             = "TwoMuonsHi",
        InputVtxContainerName      = "BPHY13TwoMuonHi",
        V0Tools                    = V0Tools,
        TrkMasses                  = [105.658, 105.658],
        VtxMassHypo                = 9460.30,
        MassMin                    = 8500.,
        MassMax                    = 11000.,
        Chi2Max                    = 200)

    BPHY13_Revertex_2trkHi         = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2trkHi",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 2, 3 ],
        RefitPV                    = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices5", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        VertexMass                 = 9460.30,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoTrackHi")

    BPHY13_Select_TwoTrackHi       = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoTrackHi",
        HypothesisName             = "TwoTracksHi",
        V0Tools                    = V0Tools,
        InputVtxContainerName      = "BPHY13TwoTrackHi",
        TrkMasses                  = [105.658, 105.658],
        VtxMassHypo                = 9460.30,
        MassMin                    = 8500.,
        MassMax                    = 11000.,
        Chi2Max                    = 200)

    BPHY13_Revertex_2muMed         = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2muMed",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices6", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        VertexMass                 = 3686.10,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoMuonMed")

    BPHY13_Select_TwoMuonMed       = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoMuonMed",
        HypothesisName             = "TwoMuonsMed",
        InputVtxContainerName      = "BPHY13TwoMuonMed",
        V0Tools                    = V0Tools,
        TrkMasses                  = [105.658, 105.658],
        VtxMassHypo                = 3686.10,
        MassMin                    = 3300.0,
        MassMax                    = 4500.0,
        Chi2Max                    = 200)

    BPHY13_Revertex_2trkMed        = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Revertex_2trkMed",
        InputVtxContainerName      = "BPHY13FourTrack",
        TrackIndices               = [ 2, 3 ],
        RefitPV                    = True,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        RefPVContainerName         = "BPHY13RefittedPrimaryVertices7", # cannot use existing refitted PVs
        UseMassConstraint          = True,
        VertexMass                 = 3686.10,
        MassInputParticles         = [105.658, 105.658],
        TrkVertexFitterTool        = vkalvrt,
        OutputVtxContainerName     = "BPHY13TwoTrackMed")

    BPHY13_Select_TwoTrackMed      = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY13_Select_TwoTrackMed",
        HypothesisName             = "TwoTracksMed",
        InputVtxContainerName      = "BPHY13TwoTrackMed",
        V0Tools                    = V0Tools,
        TrkMasses                  = [105.658, 105.658],
        VtxMassHypo                = 3686.10,
        MassMin                    = 3300.,
        MassMax                    = 4500.,
        Chi2Max                    = 200)

    expression = "( count(BPHY13TwoMuon.passed_TwoMuons) + count(BPHY13TwoTrack.passed_TwoTracks) > 1 || count(BPHY13TwoMuonMed.passed_TwoMuonsMed) + count(BPHY13TwoTrackMed.passed_TwoTracksMed) > 1 || count(BPHY13TwoMuon.passed_TwoMuons) + count(BPHY13TwoTrackMed.passed_TwoTracksMed) > 1 || count(BPHY13TwoMuonMed.passed_TwoMuonsMed) + count(BPHY13TwoTrack.passed_TwoTracks) > 1 || count(BPHY13TwoMuonHi.passed_TwoMuonsHi) + count(BPHY13TwoTrackHi.passed_TwoTracksHi) > 0 )"
    
    if do_blinding or (do_unblinding1 and do_unblinding2):
        expression = expression + " && count(BPHY13FourTrack.passed_FourTracks1)+count(BPHY13FourTrack.passed_FourTracks2)+count(BPHY13FourTrack.passed_FourTracks3)+count(BPHY13FourTrack.passed_FourTracks4) > 0"
    elif do_unblinding1:
        expression = expression + " && count(BPHY13FourTrack.passed_FourTracks1)+count(BPHY13FourTrack.passed_FourTracks2) > 0"
    elif do_unblinding2:
        expression = expression + " && count(BPHY13FourTrack.passed_FourTracks3)+count(BPHY13FourTrack.passed_FourTracks4) > 0"
    else:
        expression = expression + " && count(BPHY13FourTrack.passed_FourTracks) > 0"

    BPHY13_SelectEvent = CompFactory.DerivationFramework.xAODStringSkimmingTool(name = "BPHY13_SelectEvent", expression = expression)

    augmentation_tools = [BPHY13_Reco_mumu, BPHY13FourTrackSelectAndWrite]
    if do_blinding or (do_unblinding1 and do_unblinding2):
        augmentation_tools += [BPHY13_Select1_FourTrack, BPHY13_Select2_FourTrack, BPHY13_Select3_FourTrack, BPHY13_Select4_FourTrack]
    elif do_unblinding1:
        augmentation_tools += [BPHY13_Select1_FourTrack, BPHY13_Select2_FourTrack]
    elif do_unblinding2:
        augmentation_tools += [BPHY13_Select3_FourTrack, BPHY13_Select4_FourTrack]
    else:
        augmentation_tools += [BPHY13_Select_FourTrack]
    
    augmentation_tools += [BPHY13TrackIsolationDecorator, BPHY13_Revertex_2mu, BPHY13_Select_TwoMuon,
               BPHY13_Revertex_2trk, BPHY13_Select_TwoTrack, BPHY13_Revertex_2muHi, BPHY13_Select_TwoMuonHi,
               BPHY13_Revertex_2trkHi, BPHY13_Select_TwoTrackHi, BPHY13_Revertex_2muMed, BPHY13_Select_TwoMuonMed, 
               BPHY13_Revertex_2trkMed, BPHY13_Select_TwoTrackMed]
    for t in  augmentation_tools + [BPHY13_SelectEvent] : acc.addPublicTool(t)
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel("BPHY13Kernel",
                                                    AugmentationTools = augmentation_tools,
                                                    #Only skim if not MC
                                                    SkimmingTools     = [BPHY13_SelectEvent],
                                                    ThinningTools     = []))
    
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY13SlimmingHelper = SlimmingHelper("BPHY13SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY13_AllVariables  = getDefaultAllVariables()
    BPHY13_StaticContent = []
    
    # Needed for trigger objects
    BPHY13SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY13SlimmingHelper.IncludeBPhysTriggerContent = True
    
    ## primary vertices
    BPHY13_AllVariables += ["PrimaryVertices"]
    for i in range(1,8):
        BPHY13_StaticContent += ["xAOD::VertexContainer#BPHY13RefittedPrimaryVertices%s" %str(i) ]
        BPHY13_StaticContent += ["xAOD::VertexAuxContainer#BPHY13RefittedPrimaryVertices%sAux." %str(i)]
    
    ## ID track particles
    BPHY13_AllVariables += ["InDetTrackParticles"]
    
    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    BPHY13_AllVariables += ["CombinedMuonTrackParticles"]
    BPHY13_AllVariables += ["ExtrapolatedMuonTrackParticles"]
    
    ## muon container
    BPHY13_AllVariables += ["Muons"]
    
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13FourTrackSelectAndWrite.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13FourTrackSelectAndWrite.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2mu.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2mu.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2trk.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2trk.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2muHi.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2muHi.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2trkHi.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2trkHi.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2muMed.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2muMed.OutputVtxContainerName]
    
    BPHY13_StaticContent += ["xAOD::VertexContainer#%s"        % BPHY13_Revertex_2trkMed.OutputVtxContainerName]
    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % BPHY13_Revertex_2trkMed.OutputVtxContainerName]
    
    
    # Truth information for MC only
    if isSimulation:
        BPHY13_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]
    BPHY13SlimmingHelper.AllVariables = BPHY13_AllVariables
    BPHY13SlimmingHelper.StaticContent = BPHY13_StaticContent

    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY13", ItemList=BPHY13SlimmingHelper.GetItemList(), AcceptAlgs=["BPHY13Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY13", AcceptAlgs=["BPHY13Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
