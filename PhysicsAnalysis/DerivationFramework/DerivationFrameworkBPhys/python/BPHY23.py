
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY23.py
# Contact: xin.chen@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY23"
streamName = "StreamDAOD_BPHY23"

def BPHY23Cfg(flags):
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName)) # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)
    PVrefit = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(PVrefit)

    # mass bounds and constants used in the following
    X_hi = 141000.0

    Jpsi_lo = 2600.0
    Jpsi_hi = 3500.0
    Zc_lo = 3650.0
    Zc_hi = 4150.0
    Psi_lo = 3350.0
    Psi_hi = 4200.0
    B_lo = 4850.0
    B_hi = 5700.0
    Bs0_lo = 4950.0
    Bs0_hi = 5800.0
    Upsi_lo = 8900.0
    Upsi_hi = 9900.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Jpsimass = 3096.916
    Psi2Smass = 3686.10
    X3872mass = 3871.65
    Zcmass = 3887.1
    Bpmmass = 5279.34
    B0mass = 5279.66
    Bs0mass = 5366.92
    Upsimass = 9460.30

    BPHY23JpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY23JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 2400.,
        invMassLower                = Jpsi_lo,
        invMassUpper                = Upsi_hi,
        Chi2Cut                     = 10.,
        oppChargesOnly            = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    acc.addPublicTool(BPHY23JpsiFinder)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY23_Reco_mumu",
        VertexSearchTool       = BPHY23JpsiFinder,
        OutputVtxContainerName = "BPHY23OniaCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = PVrefit,
        DoVertexType           = 1))


    # X(3872), Psi(2S) -> J/psi + pi pi
    BPHY23PsiX3872_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name = "BPHY23PsiX3872_Jpsi2Trk",
        kaonkaonHypothesis = False,
        pionpionHypothesis = True,
        kaonpionHypothesis = False,
        kaonprotonHypothesis = False,
        trkThresholdPt = 380.,
        trkMaxEta = 2.6,
        oppChargesOnly = False,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkQuadrupletMassLower = Psi_lo,
        TrkQuadrupletMassUpper = Psi_hi,
        Chi2Cut = 10.,
        JpsiContainerKey = "BPHY23OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = vkalvrt,
        TrackSelectorTool = trackselect,
        UseMassConstraint = False)
    acc.addPublicTool(BPHY23PsiX3872_Jpsi2Trk)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY23FourTrackReco_PsiX3872",
        VertexSearchTool = BPHY23PsiX3872_Jpsi2Trk,
        OutputVtxContainerName = "BPHY23FourTrack_PsiX3872",
        PVContainerName = "PrimaryVertices",
        V0Tools = V0Tools,
        PVRefitter = PVrefit,
        RefitPV = False,
        DoVertexType = 0))


    # Bs0 -> J/psi + K K
    BPHY23Bs0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name = "BPHY23Bs0_Jpsi2Trk",
        kaonkaonHypothesis = True,
        pionpionHypothesis = False,
        kaonpionHypothesis = False,
        kaonprotonHypothesis = False,
        trkThresholdPt = 380.,
        trkMaxEta = 2.6,
        oppChargesOnly = False,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkQuadrupletMassLower = Bs0_lo,
        TrkQuadrupletMassUpper = Bs0_hi,
        Chi2Cut = 10.,
        JpsiContainerKey = "BPHY23OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = vkalvrt,
        TrackSelectorTool = trackselect,
        UseMassConstraint = False)
    acc.addPublicTool(BPHY23Bs0_Jpsi2Trk)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY23FourTrackReco_Bs0",
        VertexSearchTool = BPHY23Bs0_Jpsi2Trk,
        OutputVtxContainerName = "BPHY23FourTrack_Bs0",
        PVContainerName = "PrimaryVertices",
        V0Tools = V0Tools,
        PVRefitter = PVrefit,
        RefitPV = False,
        DoVertexType = 0))


    # B0 -> J/psi + K pi
    BPHY23B0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name = "BPHY23B0_Jpsi2Trk",
        kaonkaonHypothesis = False,
        pionpionHypothesis = False,
        kaonpionHypothesis = True,
        kaonprotonHypothesis = False,
        trkThresholdPt = 380.,
        trkMaxEta = 2.6,
        oppChargesOnly = False,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkQuadrupletMassLower = B_lo,
        TrkQuadrupletMassUpper = B_hi,
        Chi2Cut = 10.,
        JpsiContainerKey = "BPHY23OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = vkalvrt,
        TrackSelectorTool = trackselect,
        UseMassConstraint = False)
    acc.addPublicTool(BPHY23B0_Jpsi2Trk)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY23FourTrackReco_B0",
        VertexSearchTool = BPHY23B0_Jpsi2Trk,
        OutputVtxContainerName = "BPHY23FourTrack_B0",
        PVContainerName = "PrimaryVertices",
        V0Tools = V0Tools,
        PVRefitter = PVrefit,
        RefitPV = False,
        DoVertexType = 0))


    # Zc(3900)+ -> J/psi pi
    BPHY23Zc3900_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name = "BPHY23Zc3900_Jpsi1Trk",
        pionHypothesis = True,
        kaonHypothesis = False,
        trkThresholdPt = 380.,
        trkMaxEta = 2.6,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkTrippletMassLower = Zc_lo,
        TrkTrippletMassUpper = Zc_hi,
        Chi2Cut = 10.0,
        JpsiContainerKey = "BPHY23OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = vkalvrt,
        TrackSelectorTool = trackselect,
        UseMassConstraint = False)
    acc.addPublicTool(BPHY23Zc3900_Jpsi1Trk)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY23ThreeTrackReco_Zc3900",
        VertexSearchTool = BPHY23Zc3900_Jpsi1Trk,
        OutputVtxContainerName = "BPHY23ThreeTrack_Zc3900",
        PVContainerName = "PrimaryVertices",
        V0Tools = V0Tools,
        PVRefitter = PVrefit,
        RefitPV = False,
        DoVertexType = 0))


    # B+ -> J/psi K
    BPHY23Bpm_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name = "BPHY23Bpm_Jpsi1Trk",
        pionHypothesis = False,
        kaonHypothesis = True,
        trkThresholdPt = 380.,
        trkMaxEta = 2.6,
        JpsiMassLower = Jpsi_lo,
        JpsiMassUpper = Jpsi_hi,
        TrkTrippletMassLower = B_lo,
        TrkTrippletMassUpper = B_hi,
        Chi2Cut = 10.0,
        JpsiContainerKey = "BPHY23OniaCandidates",
        TrackParticleCollection = "InDetTrackParticles",
        MuonsUsedInJpsi = "Muons",
        ExcludeJpsiMuonsOnly = True,
        TrkVertexFitterTool = vkalvrt,
        TrackSelectorTool = trackselect,
        UseMassConstraint = False)
    acc.addPublicTool(BPHY23Bpm_Jpsi1Trk)

    acc.addEventAlgo(CompFactory.DerivationFramework.Reco_Vertex(
        name = "BPHY23ThreeTrackReco_Bpm",
        VertexSearchTool = BPHY23Bpm_Jpsi1Trk,
        OutputVtxContainerName = "BPHY23ThreeTrack_Bpm",
        PVContainerName = "PrimaryVertices",
        V0Tools = V0Tools,
        PVRefitter = PVrefit,
        RefitPV = False,
        DoVertexType = 0))


    # revertex with mass constraints to reduce combinatorics
    # Psi(2S) -> J/psi pi pi
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Psi4Body",
        InputVtxContainerName = "BPHY23FourTrack_PsiX3872",
        TrackIndices = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Psi2Smass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Pimass, Pimass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Psi4Body"))


    # X(3872) -> J/psi pi pi
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_X3872",
        InputVtxContainerName = "BPHY23FourTrack_PsiX3872",
        TrackIndices = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = X3872mass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Pimass, Pimass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_X3872"))

    # Bs0 -> J/psi K K
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Bs0",
        InputVtxContainerName = "BPHY23FourTrack_Bs0",
        TrackIndices = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Bs0mass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Kmass, Kmass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Bs0"))

    # B0 -> J/psi K pi
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_B0Kpi",
        InputVtxContainerName = "BPHY23FourTrack_B0",
        TrackIndices = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = B0mass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Kmass, Pimass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_B0Kpi"))


    # B0 -> J/psi pi K
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_B0piK",
        InputVtxContainerName = "BPHY23FourTrack_B0",
        TrackIndices = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = B0mass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Pimass, Kmass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_B0piK"))


    # Zc3900 -> J/psi pi
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Zc3900",
        InputVtxContainerName = "BPHY23ThreeTrack_Zc3900",
        TrackIndices = [ 0, 1, 2 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Pimass],
        Chi2Cut = 25.,
        BMassLower = Zc_lo,
        BMassUpper = Zc_hi,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Zc3900"))

    # Bpm -> J/psi K
    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Bpm",
        InputVtxContainerName = "BPHY23ThreeTrack_Bpm",
        TrackIndices = [ 0, 1, 2 ],
        SubVertexTrackIndices = [ 1, 2 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Bpmmass,
        SubVertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass, Kmass],
        Chi2Cut = 25.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Bpm"))


    acc.addEventAlgo(CompFactory.DerivationFramework.Select_onia2mumu(
        name = "BPHY23Select_Jpsi",
        HypothesisName = "Jpsi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        V0Tools = V0Tools,
        TrkMasses = [Mumass, Mumass],
        MassMin = Jpsi_lo,
        MassMax = Jpsi_hi,
        DoVertexType = 0))

    acc.addEventAlgo(CompFactory.DerivationFramework.Select_onia2mumu(
        name = "BPHY23Select_Psi",
        HypothesisName = "Psi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        V0Tools = V0Tools,
        TrkMasses = [Mumass, Mumass],
        MassMin = Psi_lo,
        MassMax = Psi_hi,
        DoVertexType = 0))

    acc.addEventAlgo(CompFactory.DerivationFramework.Select_onia2mumu(
        name = "BPHY23Select_Upsi",
        HypothesisName = "Upsi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        V0Tools = V0Tools,
        TrkMasses = [Mumass, Mumass],
        MassMin = Upsi_lo,
        MassMax = Upsi_hi,
        DoVertexType = 0))

    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Jpsi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        HypothesisNames = [ "Jpsi" ],
        TrackIndices = [ 0, 1 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Jpsimass,
        MassInputParticles = [Mumass, Mumass],
        Chi2Cut = 50.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Jpsi"))

    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Psi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        HypothesisNames = [ "Psi" ],
        TrackIndices = [ 0, 1 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Psi2Smass,
        MassInputParticles = [Mumass, Mumass],
        Chi2Cut = 50.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Psi"))

    acc.addEventAlgo(CompFactory.DerivationFramework.ReVertex(
        name = "BPHY23Rev_Upsi",
        InputVtxContainerName = "BPHY23OniaCandidates",
        HypothesisNames = [ "Upsi" ],
        TrackIndices = [ 0, 1 ],
        RefitPV = False,
        UseMassConstraint = True,
        VertexMass = Upsimass,
        MassInputParticles = [Mumass, Mumass],
        Chi2Cut = 50.,
        TrkVertexFitterTool = vkalvrt,
        PVRefitter = PVrefit,
        V0Tools = V0Tools,
        OutputVtxContainerName = "BPHY23Revtx_Upsi"))


    OutputCollections = []
    RefPVContainers = []
    RefPVAuxContainers = []
    passedCandidates = []

    ########################
    ###  2 trks + 0 trk  ###
    ########################

    list_2trk0trk_hypo = ["Bs2KJpsi0", "Bs2KPsi0", "Bs2KUpsi0",
                          "B0KpiJpsi0", "B0KpiPsi0", "B0KpiUpsi0",
                          "B0piKJpsi0", "B0piKPsi0", "B0piKUpsi0"]
    list_2trk0trk_psiInput = ["BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                              "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi",
                              "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK"]
    list_2trk0trk_jpsiInput = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi",
                               "BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi",
                               "BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi"]
    list_2trk0trk_jpsiMass = [Jpsimass, Jpsimass, Jpsimass,
                              Jpsimass, Jpsimass, Jpsimass,
                              Jpsimass, Jpsimass, Jpsimass]
    list_2trk0trk_psiMass = [Bs0mass, Bs0mass, Bs0mass,
                             B0mass, B0mass, B0mass,
                             B0mass, B0mass, B0mass]
    list_2trk0trk_dau3Mass = [Kmass, Kmass, Kmass,
                              Kmass, Kmass, Kmass,
                              Pimass, Pimass, Pimass]
    list_2trk0trk_dau4Mass = [Kmass, Kmass, Kmass,
                              Pimass, Pimass, Pimass,
                              Kmass, Kmass, Kmass]
    list_2trk0trk_jpsi2Mass = [Jpsimass, Psi2Smass, Upsimass,
                               Jpsimass, Psi2Smass, Upsimass,
                               Jpsimass, Psi2Smass, Upsimass]

    for i in range(len(list_2trk0trk_hypo)):
        twoTrk0TrkArgs = {}
        twoTrk0TrkArgs.setdefault("HypothesisName" , list_2trk0trk_hypo[i])
        twoTrk0TrkArgs.setdefault("JpsiVertices" , list_2trk0trk_jpsiInput[i])
        twoTrk0TrkArgs.setdefault("PsiVertices" , list_2trk0trk_psiInput[i])
        twoTrk0TrkArgs.setdefault("NumberOfPsiDaughters" , 4)
        twoTrk0TrkArgs.setdefault("MassLowerCut" , 0.)
        twoTrk0TrkArgs.setdefault("MassUpperCut" , X_hi)
        twoTrk0TrkArgs.setdefault("Chi2Cut" , 30.)
        twoTrk0TrkArgs.setdefault("MaxCandidates" , 15)
        twoTrk0TrkArgs.setdefault("MaxnPV" , 100)
        twoTrk0TrkArgs.setdefault("PVRefitter" , PVrefit)
        twoTrk0TrkArgs.setdefault("TrkVertexFitterTool" , vkalvrt)
        twoTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrk0TrkArgs.setdefault("RefPVContainerName" , "BPHY23_"+list_2trk0trk_hypo[i]+"_RefPrimaryVertices")
        twoTrk0TrkArgs.setdefault("CascadeVertexCollections" , ["BPHY23_"+list_2trk0trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_2trk0trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_2trk0trk_hypo[i]+"_CascadeVtx3"])
        twoTrk0TrkArgs.setdefault("RefitPV" , True)
        twoTrk0TrkArgs.setdefault("Vtx1Daug3MassHypo" , list_2trk0trk_dau3Mass[i])
        twoTrk0TrkArgs.setdefault("Vtx1Daug4MassHypo" , list_2trk0trk_dau4Mass[i])
        twoTrk0TrkArgs.setdefault("ApplyJpsiMassConstraint" , True)
        twoTrk0TrkArgs.setdefault("JpsiMass" , list_2trk0trk_jpsiMass[i])
        twoTrk0TrkArgs.setdefault("ApplyPsiMassConstraint" , True)
        twoTrk0TrkArgs.setdefault("PsiMass" , list_2trk0trk_psiMass[i])
        twoTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint" , True)
        twoTrk0TrkArgs.setdefault("Jpsi2Mass" , list_2trk0trk_jpsi2Mass[i])
        OutputCollections += twoTrk0TrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_2trk0trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_2trk0trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_2trk0trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.JpsiPlusPsiCascade("BPHY23_"+list_2trk0trk_hypo[i], **twoTrk0TrkArgs) )


    #######################
    ###  1 trk + 0 trk  ###
    #######################

    list_1trk0trk_hypo = ["BpmJpsi0", "BpmPsi0", "BpmUpsi0"]
    list_1trk0trk_psiInput = ["BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm"]
    list_1trk0trk_jpsiInput = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi"]
    list_1trk0trk_jpsiMass = [Jpsimass, Jpsimass, Jpsimass]
    list_1trk0trk_psiMass = [Bpmmass, Bpmmass, Bpmmass]
    list_1trk0trk_dau3Mass = [Kmass, Kmass, Kmass]
    list_1trk0trk_jpsi2Mass = [Jpsimass, Psi2Smass, Upsimass]

    for i in range(len(list_1trk0trk_hypo)):
        oneTrk0TrkArgs = { }
        oneTrk0TrkArgs.setdefault("HypothesisName" , list_1trk0trk_hypo[i])
        oneTrk0TrkArgs.setdefault("JpsiVertices" , list_1trk0trk_jpsiInput[i])
        oneTrk0TrkArgs.setdefault("PsiVertices" , list_1trk0trk_psiInput[i])
        oneTrk0TrkArgs.setdefault("NumberOfPsiDaughters" , 3)
        oneTrk0TrkArgs.setdefault("MassLowerCut" , 0.)
        oneTrk0TrkArgs.setdefault("MassUpperCut" , X_hi)
        oneTrk0TrkArgs.setdefault("Chi2Cut" , 30.)
        oneTrk0TrkArgs.setdefault("MaxCandidates" , 15)
        oneTrk0TrkArgs.setdefault("MaxnPV" , 100)
        oneTrk0TrkArgs.setdefault("PVRefitter" , PVrefit)
        oneTrk0TrkArgs.setdefault("TrkVertexFitterTool" , vkalvrt)
        oneTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrk0TrkArgs.setdefault("RefPVContainerName" , "BPHY23_"+list_1trk0trk_hypo[i]+"_RefPrimaryVertices")
        oneTrk0TrkArgs.setdefault("CascadeVertexCollections" , ["BPHY23_"+list_1trk0trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_1trk0trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_1trk0trk_hypo[i]+"_CascadeVtx3"])
        oneTrk0TrkArgs.setdefault("RefitPV" , True)
        oneTrk0TrkArgs.setdefault("Vtx1Daug3MassHypo" , list_1trk0trk_dau3Mass[i])
        oneTrk0TrkArgs.setdefault("ApplyJpsiMassConstraint" , True)
        oneTrk0TrkArgs.setdefault("JpsiMass" , list_1trk0trk_jpsiMass[i])
        oneTrk0TrkArgs.setdefault("ApplyPsiMassConstraint" , True)
        oneTrk0TrkArgs.setdefault("PsiMass" , list_1trk0trk_psiMass[i])
        oneTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint" , True)
        oneTrk0TrkArgs.setdefault("Jpsi2Mass" , list_1trk0trk_jpsi2Mass[i])
        OutputCollections += oneTrk0TrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_1trk0trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_1trk0trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_1trk0trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.JpsiPlusPsiCascade("BPHY23_"+list_1trk0trk_hypo[i], **oneTrk0TrkArgs) )

    #######################
    ###  1 trk + 1 trk  ###
    #######################

    list_1trk1trk_hypo = ["BpmZc3900", "BpmBpm"]
    list_1trk1trk_psi1Input = ["BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm"]
    list_1trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm"]
    list_1trk1trk_jpsi1Mass = [Jpsimass, Jpsimass]
    list_1trk1trk_psi1Mass = [Bpmmass, Bpmmass]
    list_1trk1trk_p1dau3Mass = [Kmass, Kmass]
    list_1trk1trk_jpsi2Mass = [Jpsimass, Jpsimass]
    list_1trk1trk_psi2Mass = [Zcmass, Bpmmass]
    list_1trk1trk_p2dau3Mass = [Pimass, Kmass]

    for i in range(len(list_1trk1trk_hypo)):
        oneTrkOneTrkArgs = { }
        oneTrkOneTrkArgs("HypothesisName" , list_1trk1trk_hypo[i])
        oneTrkOneTrkArgs("Psi1Vertices" , list_1trk1trk_psi1Input[i])
        oneTrkOneTrkArgs("Psi2Vertices" , list_1trk1trk_psi2Input[i])
        oneTrkOneTrkArgs("NumberOfPsi1Daughters" , 3)
        oneTrkOneTrkArgs("NumberOfPsi2Daughters" , 3)
        oneTrkOneTrkArgs("MassLowerCut" , 0.)
        oneTrkOneTrkArgs("MassUpperCut" , X_hi)
        oneTrkOneTrkArgs("Chi2CutPsi1" , 5.)
        oneTrkOneTrkArgs("Chi2CutPsi2" , 5.)
        oneTrkOneTrkArgs("Chi2Cut" , 30.)
        oneTrkOneTrkArgs("MaxCandidates" , 15)
        oneTrkOneTrkArgs("RemoveDuplicatePairs" , True)
        oneTrkOneTrkArgs("MaxnPV" , 100)
        oneTrkOneTrkArgs("PVRefitter" , PVrefit)
        oneTrkOneTrkArgs("TrkVertexFitterTool" , vkalvrt)
        oneTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrkOneTrkArgs("RefPVContainerName" , "BPHY23_"+list_1trk1trk_hypo[i]+"_RefPrimaryVertices")
        oneTrkOneTrkArgs("CascadeVertexCollections" , ["BPHY23_"+list_1trk1trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_1trk1trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_1trk1trk_hypo[i]+"_CascadeVtx3"])
        oneTrkOneTrkArgs("RefitPV" , True)
        oneTrkOneTrkArgs("Vtx1Daug3MassHypo" , list_1trk1trk_p1dau3Mass[i])
        oneTrkOneTrkArgs("Vtx2Daug3MassHypo" , list_1trk1trk_p2dau3Mass[i])
        oneTrkOneTrkArgs("ApplyJpsi1MassConstraint" , True)
        oneTrkOneTrkArgs("Jpsi1Mass" , list_1trk1trk_jpsi1Mass[i])
        oneTrkOneTrkArgs("ApplyPsi1MassConstraint" , True)
        oneTrkOneTrkArgs("Psi1Mass" , list_1trk1trk_psi1Mass[i])
        oneTrkOneTrkArgs("ApplyJpsi2MassConstraint" , True)
        oneTrkOneTrkArgs("Jpsi2Mass" , list_1trk1trk_jpsi2Mass[i])
        if list_1trk1trk_psi2Mass[i] != Zcmass:
            oneTrkOneTrkArgs("ApplyPsi2MassConstraint" , True)
            oneTrkOneTrkArgs("Psi2Mass" , list_1trk1trk_psi2Mass[i])
        OutputCollections += oneTrkOneTrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_1trk1trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_1trk1trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_1trk1trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiCascade("BPHY23_"+list_1trk1trk_hypo[i], **oneTrkOneTrkArgs) )

    ########################
    ###  2 trks + 1 trk  ###
    ########################

    list_2trk1trk_hypo = ["Bs2KZc3900", "Bs2KBpm",
                          "B0KpiZc3900", "B0KpiBpm",
                          "B0piKZc3900", "B0piKBpm"]
    list_2trk1trk_psi1Input = ["BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                               "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi",
                               "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK"]
    list_2trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm",
                               "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm",
                               "BPHY23Revtx_Zc3900", "BPHY23Revtx_Bpm"]
    list_2trk1trk_jpsi1Mass = [Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass]
    list_2trk1trk_psi1Mass = [Bs0mass, Bs0mass,
                              B0mass, B0mass,
                              B0mass, B0mass]
    list_2trk1trk_p1dau3Mass = [Kmass, Kmass,
                                Kmass, Kmass,
                                Pimass, Pimass]
    list_2trk1trk_p1dau4Mass = [Kmass, Kmass,
                                Pimass, Pimass,
                                Kmass, Kmass]
    list_2trk1trk_jpsi2Mass = [Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass]
    list_2trk1trk_psi2Mass = [Zcmass, Bpmmass,
                              Zcmass, Bpmmass,
                              Zcmass, Bpmmass]
    list_2trk1trk_p2dau3Mass = [Pimass, Kmass,
                                Pimass, Kmass,
                                Pimass, Kmass]

    for i in range(len(list_2trk1trk_hypo)):
        twoTrkOneTrkArgs = { }
        twoTrkOneTrkArgs("HypothesisName" , list_2trk1trk_hypo[i])
        twoTrkOneTrkArgs("Psi1Vertices" , list_2trk1trk_psi1Input[i])
        twoTrkOneTrkArgs("Psi2Vertices" , list_2trk1trk_psi2Input[i])
        twoTrkOneTrkArgs("NumberOfPsi1Daughters" , 4)
        twoTrkOneTrkArgs("NumberOfPsi2Daughters" , 3)
        twoTrkOneTrkArgs("MassLowerCut" , 0.)
        twoTrkOneTrkArgs("MassUpperCut" , X_hi)
        twoTrkOneTrkArgs("Chi2CutPsi1" , 5.)
        twoTrkOneTrkArgs("Chi2CutPsi2" , 5.)
        twoTrkOneTrkArgs("Chi2Cut" , 30.)
        twoTrkOneTrkArgs("MaxCandidates" , 15)
        twoTrkOneTrkArgs("MaxnPV" , 100)
        twoTrkOneTrkArgs("PVRefitter" , PVrefit)
        twoTrkOneTrkArgs("TrkVertexFitterTool" , vkalvrt)
        twoTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkOneTrkArgs("RefPVContainerName" , "BPHY23_"+list_2trk1trk_hypo[i]+"_RefPrimaryVertices")
        twoTrkOneTrkArgs("CascadeVertexCollections" , ["BPHY23_"+list_2trk1trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_2trk1trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_2trk1trk_hypo[i]+"_CascadeVtx3"])
        twoTrkOneTrkArgs("RefitPV" , True)
        twoTrkOneTrkArgs("Vtx1Daug3MassHypo" , list_2trk1trk_p1dau3Mass[i])
        twoTrkOneTrkArgs("Vtx1Daug4MassHypo" , list_2trk1trk_p1dau4Mass[i])
        twoTrkOneTrkArgs("Vtx2Daug3MassHypo" , list_2trk1trk_p2dau3Mass[i])
        twoTrkOneTrkArgs("ApplyJpsi1MassConstraint" , True)
        twoTrkOneTrkArgs("Jpsi1Mass" , list_2trk1trk_jpsi1Mass[i])
        twoTrkOneTrkArgs("ApplyPsi1MassConstraint" , True)
        twoTrkOneTrkArgs("Psi1Mass" , list_2trk1trk_psi1Mass[i])
        twoTrkOneTrkArgs("ApplyJpsi2MassConstraint" , True)
        twoTrkOneTrkArgs("Jpsi2Mass" , list_2trk1trk_jpsi2Mass[i])
        if list_2trk1trk_psi2Mass[i] != Zcmass:
            twoTrkOneTrkArgs("ApplyPsi2MassConstraint" , True)
            twoTrkOneTrkArgs("Psi2Mass" , list_2trk1trk_psi2Mass[i])
        OutputCollections += twoTrkOneTrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_2trk1trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_2trk1trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_2trk1trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiCascade("BPHY23_"+list_2trk1trk_hypo[i], **twoTrkOneTrkArgs) )

    ########################
    ###  1 trks + 2 trk  ###
    ########################

    list_1trk2trk_hypo = ["BpmPsi2", "BpmX3872"]
    list_1trk2trk_psi1Input = ["BPHY23Revtx_Bpm", "BPHY23Revtx_Bpm"]
    list_1trk2trk_psi2Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872"]
    list_1trk2trk_jpsi1Mass = [Jpsimass, Jpsimass]
    list_1trk2trk_psi1Mass = [Bpmmass, Bpmmass]
    list_1trk2trk_p1dau3Mass = [Kmass, Kmass]
    list_1trk2trk_jpsi2Mass = [Jpsimass, Jpsimass]
    list_1trk2trk_psi2Mass = [Psi2Smass, X3872mass]
    list_1trk2trk_p2dau3Mass = [Pimass, Pimass]
    list_1trk2trk_p2dau4Mass = [Pimass, Pimass]

    for i in range(len(list_1trk2trk_hypo)):
        oneTrkTwoTrkArgs = { }
        oneTrkTwoTrkArgs("HypothesisName" , list_1trk2trk_hypo[i])
        oneTrkTwoTrkArgs("Psi1Vertices" , list_1trk2trk_psi1Input[i])
        oneTrkTwoTrkArgs("Psi2Vertices" , list_1trk2trk_psi2Input[i])
        oneTrkTwoTrkArgs("NumberOfPsi1Daughters" , 3)
        oneTrkTwoTrkArgs("NumberOfPsi2Daughters" , 4)
        oneTrkTwoTrkArgs("MassLowerCut" , 0.)
        oneTrkTwoTrkArgs("MassUpperCut" , X_hi)
        oneTrkTwoTrkArgs("Chi2CutPsi1" , 5.)
        oneTrkTwoTrkArgs("Chi2CutPsi2" , 5.)
        oneTrkTwoTrkArgs("Chi2Cut" , 30.)
        oneTrkTwoTrkArgs("MaxCandidates" , 15)
        oneTrkTwoTrkArgs("MaxnPV" , 100)
        oneTrkTwoTrkArgs("PVRefitter" , PVrefit)
        oneTrkTwoTrkArgs("TrkVertexFitterTool" , vkalvrt)
        oneTrkTwoTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrkTwoTrkArgs("RefPVContainerName" , "BPHY23_"+list_1trk2trk_hypo[i]+"_RefPrimaryVertices")
        oneTrkTwoTrkArgs("CascadeVertexCollections" , ["BPHY23_"+list_1trk2trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_1trk2trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_1trk2trk_hypo[i]+"_CascadeVtx3"])
        oneTrkTwoTrkArgs("RefitPV" , True)
        oneTrkTwoTrkArgs("Vtx1Daug3MassHypo" , list_1trk2trk_p1dau3Mass[i])
        oneTrkTwoTrkArgs("Vtx2Daug3MassHypo" , list_1trk2trk_p2dau3Mass[i])
        oneTrkTwoTrkArgs("Vtx2Daug4MassHypo" , list_1trk2trk_p2dau4Mass[i])
        oneTrkTwoTrkArgs("ApplyJpsi1MassConstraint" , True)
        oneTrkTwoTrkArgs("Jpsi1Mass" , list_1trk2trk_jpsi1Mass[i])
        oneTrkTwoTrkArgs("ApplyPsi1MassConstraint" , True)
        oneTrkTwoTrkArgs("Psi1Mass" , list_1trk2trk_psi1Mass[i])
        oneTrkTwoTrkArgs("ApplyJpsi2MassConstraint" , True)
        oneTrkTwoTrkArgs("Jpsi2Mass" , list_1trk2trk_jpsi2Mass[i])
        oneTrkTwoTrkArgs("ApplyPsi2MassConstraint" , True)
        oneTrkTwoTrkArgs("Psi2Mass" , list_1trk2trk_psi2Mass[i])
        OutputCollections += oneTrkTwoTrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_1trk2trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_1trk2trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_1trk2trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiCascade("BPHY23_"+list_1trk2trk_hypo[i], **oneTrkTwoTrkArgs) )

    #########################
    ###  2 trks + 2 trks  ###
    #########################

    list_2trk2trk_hypo = ["Bs2KPsi2", "B0KpiPsi2", "B0piKPsi2",
                          "Bs2KX3872", "B0KpiX3872", "B0piKX3872",
                          "Bs2KBs2K", "Bs2KB0Kpi", "Bs2KB0piK",
                          "B0KpiB0Kpi", "B0KpiB0piK", "B0piKB0piK"]
    list_2trk2trk_psi1Input = ["BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK",
                               "BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK",
                               "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0", "BPHY23Revtx_Bs0",
                               "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK"]
    list_2trk2trk_psi2Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body",
                               "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872",
                               "BPHY23Revtx_Bs0", "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK",
                               "BPHY23Revtx_B0Kpi", "BPHY23Revtx_B0piK", "BPHY23Revtx_B0piK"]
    list_2trk2trk_jpsi1Mass = [Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass]
    list_2trk2trk_psi1Mass = [Bs0mass, B0mass, B0mass,
                              Bs0mass, B0mass, B0mass,
                              Bs0mass, Bs0mass, Bs0mass,
                              B0mass, B0mass, B0mass]
    list_2trk2trk_p1dau3Mass = [Kmass, Kmass, Pimass,
                                Kmass, Kmass, Pimass,
                                Kmass, Kmass, Kmass,
                                Kmass, Kmass, Pimass]
    list_2trk2trk_p1dau4Mass = [Kmass, Pimass, Kmass,
                                Kmass, Pimass, Kmass,
                                Kmass, Kmass, Kmass,
                                Pimass, Pimass, Kmass]
    list_2trk2trk_jpsi2Mass = [Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass,
                               Jpsimass, Jpsimass, Jpsimass]
    list_2trk2trk_psi2Mass = [Psi2Smass, Psi2Smass, Psi2Smass,
                              X3872mass, X3872mass, X3872mass,
                              Bs0mass, B0mass, B0mass,
                              B0mass, B0mass, B0mass]
    list_2trk2trk_p2dau3Mass = [Pimass, Pimass, Pimass,
                                Pimass, Pimass, Pimass,
                                Kmass, Kmass, Pimass,
                                Kmass, Pimass, Pimass]
    list_2trk2trk_p2dau4Mass = [Pimass, Pimass, Pimass,
                                Pimass, Pimass, Pimass,
                                Kmass, Pimass, Kmass,
                                Pimass, Kmass, Kmass]

    for i in range(len(list_2trk2trk_hypo)):
        twoTrkTwoTrkArgs = { }
        twoTrkTwoTrkArgs("HypothesisName" , list_2trk2trk_hypo[i])
        twoTrkTwoTrkArgs("Psi1Vertices" , list_2trk2trk_psi1Input[i])
        twoTrkTwoTrkArgs("Psi2Vertices" , list_2trk2trk_psi2Input[i])
        twoTrkTwoTrkArgs("NumberOfPsi1Daughters" , 4)
        twoTrkTwoTrkArgs("NumberOfPsi2Daughters" , 4)
        twoTrkTwoTrkArgs("MassLowerCut" , 0.)
        twoTrkTwoTrkArgs("MassUpperCut" , X_hi)
        twoTrkTwoTrkArgs("Chi2CutPsi1" , 4.)
        twoTrkTwoTrkArgs("Chi2CutPsi2" , 4.)
        twoTrkTwoTrkArgs("Chi2Cut" , 30.)
        twoTrkTwoTrkArgs("MaxCandidates" , 15)
        twoTrkTwoTrkArgs("RemoveDuplicatePairs" , True)
        twoTrkTwoTrkArgs("MaxnPV" , 100)
        twoTrkTwoTrkArgs("PVRefitter" , PVrefit)
        twoTrkTwoTrkArgs("TrkVertexFitterTool" , vkalvrt)
        twoTrkTwoTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkTwoTrkArgs("RefPVContainerName" , "BPHY23_"+list_2trk2trk_hypo[i]+"_RefPrimaryVertices")
        twoTrkTwoTrkArgs("CascadeVertexCollections" , ["BPHY23_"+list_2trk2trk_hypo[i]+"_CascadeVtx1","BPHY23_"+list_2trk2trk_hypo[i]+"_CascadeVtx2","BPHY23_"+list_2trk2trk_hypo[i]+"_CascadeVtx3"])
        twoTrkTwoTrkArgs("RefitPV" , True)
        twoTrkTwoTrkArgs("Vtx1Daug3MassHypo" , list_2trk2trk_p1dau3Mass[i])
        twoTrkTwoTrkArgs("Vtx1Daug4MassHypo" , list_2trk2trk_p1dau4Mass[i])
        twoTrkTwoTrkArgs("Vtx2Daug3MassHypo" , list_2trk2trk_p2dau3Mass[i])
        twoTrkTwoTrkArgs("Vtx2Daug4MassHypo" , list_2trk2trk_p2dau4Mass[i])
        twoTrkTwoTrkArgs("ApplyJpsi1MassConstraint" , True)
        twoTrkTwoTrkArgs("Jpsi1Mass" , list_2trk2trk_jpsi1Mass[i])
        twoTrkTwoTrkArgs("ApplyPsi1MassConstraint" , True)
        twoTrkTwoTrkArgs("Psi1Mass" , list_2trk2trk_psi1Mass[i])
        twoTrkTwoTrkArgs("ApplyJpsi2MassConstraint" , True)
        twoTrkTwoTrkArgs("Jpsi2Mass" , list_2trk2trk_jpsi2Mass[i])
        twoTrkTwoTrkArgs("ApplyPsi2MassConstraint" , True)
        twoTrkTwoTrkArgs("Psi2Mass" , list_2trk2trk_psi2Mass[i])
        OutputCollections += twoTrkTwoTrkArgs.get("CascadeVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list_2trk2trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list_2trk2trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list_2trk2trk_hypo[i] + "_CascadeVtx3"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiCascade("BPHY23_"+list_2trk2trk_hypo[i], **twoTrkTwoTrkArgs) )

    list2_2trk0trk_hypo = ["Psi2Jpsi0", "Psi2Psi0", "Psi2Upsi0",
                           "X3872Jpsi0", "X3872Psi0", "X3872Upsi0"]
    list2_2trk0trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body",
                                "BPHY23Revtx_X3872", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872"]
    list2_2trk0trk_psi2Input = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi",
                                "BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi"]
    list2_2trk0trk_jpsi2lo = [Jpsi_lo, Psi_lo, Upsi_lo, Jpsi_lo, Psi_lo, Upsi_lo]
    list2_2trk0trk_jpsi2hi = [Jpsi_hi, Psi_hi, Upsi_hi, Jpsi_hi, Psi_hi, Upsi_hi]
    list2_2trk0trk_jpsi1mass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass, Jpsimass]
    list2_2trk0trk_psi1mass = [Psi2Smass, Psi2Smass, Psi2Smass, X3872mass, X3872mass, X3872mass]
    list2_2trk0trk_jpsi2mass = [Jpsimass, Psi2Smass, Upsimass, Jpsimass, Psi2Smass, Upsimass]

    for i in range(len(list2_2trk0trk_hypo)):
        twoTrk0TrkArgs = {}
        twoTrk0TrkArgs.setdefault("HypothesisName" , list2_2trk0trk_hypo[i])
        twoTrk0TrkArgs.setdefault("Psi1Vertices" , list2_2trk0trk_psi1Input[i])
        twoTrk0TrkArgs.setdefault("Psi2Vertices" , list2_2trk0trk_psi2Input[i])
        twoTrk0TrkArgs.setdefault("MaxCandidates" , 15)
        twoTrk0TrkArgs.setdefault("NumberOfPsi1Daughters" , 4)
        twoTrk0TrkArgs.setdefault("NumberOfPsi2Daughters" , 2)
        twoTrk0TrkArgs.setdefault("Jpsi1MassLowerCut" , Jpsi_lo)
        twoTrk0TrkArgs.setdefault("Jpsi1MassUpperCut" , Jpsi_hi)
        twoTrk0TrkArgs.setdefault("Psi1MassLowerCut" , Psi_lo)
        twoTrk0TrkArgs.setdefault("Psi1MassUpperCut" , Psi_hi)
        twoTrk0TrkArgs.setdefault("Jpsi2MassLowerCut" , list2_2trk0trk_jpsi2lo[i])
        twoTrk0TrkArgs.setdefault("Jpsi2MassUpperCut" , list2_2trk0trk_jpsi2hi[i])
        twoTrk0TrkArgs.setdefault("MassLowerCut" , 0.)
        twoTrk0TrkArgs.setdefault("MassUpperCut" , X_hi)
        twoTrk0TrkArgs.setdefault("Jpsi1Mass" , list2_2trk0trk_jpsi1mass[i])
        twoTrk0TrkArgs.setdefault("Psi1Mass" , list2_2trk0trk_psi1mass[i])
        twoTrk0TrkArgs.setdefault("Jpsi2Mass" , list2_2trk0trk_jpsi2mass[i])
        twoTrk0TrkArgs.setdefault("ApplyJpsi1MassConstraint" , True)
        twoTrk0TrkArgs.setdefault("ApplyPsi1MassConstraint" , True)
        twoTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint" , True)
        twoTrk0TrkArgs.setdefault("Chi2Cut" , 30.)
        twoTrk0TrkArgs.setdefault("PVRefitter" , PVrefit)
        twoTrk0TrkArgs.setdefault("TrkVertexFitterTool" , vkalvrt)
        twoTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrk0TrkArgs.setdefault("OutputVertexCollections" , ["BPHY23_"+list2_2trk0trk_hypo[i]+"_SubVtx1","BPHY23_"+list2_2trk0trk_hypo[i]+"_SubVtx2","BPHY23_"+list2_2trk0trk_hypo[i]+"_MainVtx"])
        twoTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrk0TrkArgs.setdefault("RefPVContainerName" , "BPHY23_"+list2_2trk0trk_hypo[i]+"_RefPrimaryVertices")
        twoTrk0TrkArgs.setdefault("RefitPV" , True)
        twoTrk0TrkArgs.setdefault("MaxnPV" , 100)
        OutputCollections += twoTrk0TrkArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list2_2trk0trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list2_2trk0trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list2_2trk0trk_hypo[i] + "_MainVtx"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY23_"+list2_2trk0trk_hypo[i], **twoTrk0TrkArgs) )

    list2_1trk0trk_hypo = ["Zc3900Jpsi0", "Zc3900Psi0", "Zc3900Upsi0"]
    list2_1trk0trk_psi1Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900"]
    list2_1trk0trk_psi2Input = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi"]
    list2_1trk0trk_jpsi2lo = [Jpsi_lo, Psi_lo, Upsi_lo]
    list2_1trk0trk_jpsi2hi = [Jpsi_hi, Psi_hi, Upsi_hi]
    list2_1trk0trk_jpsi1mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_1trk0trk_psi1mass = [Zcmass, Zcmass, Zcmass]
    list2_1trk0trk_jpsi2mass = [Jpsimass, Psi2Smass, Upsimass]

    for i in range(len(list2_1trk0trk_hypo)):
        oneTrk0TrkArgs = { }
        oneTrk0TrkArgs.setdefault("HypothesisName" , list2_1trk0trk_hypo[i])
        oneTrk0TrkArgs.setdefault("Psi1Vertices" , list2_1trk0trk_psi1Input[i])
        oneTrk0TrkArgs.setdefault("Psi2Vertices" , list2_1trk0trk_psi2Input[i])
        oneTrk0TrkArgs.setdefault("MaxCandidates" , 15)
        oneTrk0TrkArgs.setdefault("NumberOfPsi1Daughters" , 3)
        oneTrk0TrkArgs.setdefault("NumberOfPsi2Daughters" , 2)
        oneTrk0TrkArgs.setdefault("Jpsi1MassLowerCut" , Jpsi_lo)
        oneTrk0TrkArgs.setdefault("Jpsi1MassUpperCut" , Jpsi_hi)
        oneTrk0TrkArgs.setdefault("Psi1MassLowerCut" , Zc_lo)
        oneTrk0TrkArgs.setdefault("Psi1MassUpperCut" , Zc_hi)
        oneTrk0TrkArgs.setdefault("Jpsi2MassLowerCut" , list2_1trk0trk_jpsi2lo[i])
        oneTrk0TrkArgs.setdefault("Jpsi2MassUpperCut" , list2_1trk0trk_jpsi2hi[i])
        oneTrk0TrkArgs.setdefault("MassLowerCut" , 0.)
        oneTrk0TrkArgs.setdefault("MassUpperCut" , X_hi)
        oneTrk0TrkArgs.setdefault("Jpsi1Mass" , list2_1trk0trk_jpsi1mass[i])
        oneTrk0TrkArgs.setdefault("Psi1Mass" , list2_1trk0trk_psi1mass[i])
        oneTrk0TrkArgs.setdefault("Jpsi2Mass" , list2_1trk0trk_jpsi2mass[i])
        oneTrk0TrkArgs.setdefault("ApplyJpsi1MassConstraint" , True)
        oneTrk0TrkArgs.setdefault("ApplyPsi1MassConstraint" , True)
        oneTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint" , True)
        oneTrk0TrkArgs.setdefault("Chi2Cut" , 30.)
        oneTrk0TrkArgs.setdefault("PVRefitter" , PVrefit)
        oneTrk0TrkArgs.setdefault("TrkVertexFitterTool" , vkalvrt)
        oneTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrk0TrkArgs.setdefault("OutputVertexCollections" , ["BPHY23_"+list2_1trk0trk_hypo[i]+"_SubVtx1","BPHY23_"+list2_1trk0trk_hypo[i]+"_SubVtx2","BPHY23_"+list2_1trk0trk_hypo[i]+"_MainVtx"])
        oneTrk0TrkArgs.setdefault("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrk0TrkArgs.setdefault("RefPVContainerName" , "BPHY23_"+list2_1trk0trk_hypo[i]+"_RefPrimaryVertices")
        oneTrk0TrkArgs.setdefault("RefitPV" , True)
        oneTrk0TrkArgs.setdefault("MaxnPV" , 100)
        OutputCollections += oneTrk0TrkArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list2_1trk0trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list2_1trk0trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list2_1trk0trk_hypo[i] + "_MainVtx"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY23_"+list2_1trk0trk_hypo[i], **oneTrk0TrkArgs) )

    list2_1trk1trk_hypo = ["Zc3900Zc3900"]
    list2_1trk1trk_psi1Input = ["BPHY23Revtx_Zc3900"]
    list2_1trk1trk_psi2Input = ["BPHY23Revtx_Zc3900"]
    list2_1trk1trk_jpsi1mass = [Jpsimass]
    list2_1trk1trk_psi1mass = [Zcmass]
    list2_1trk1trk_jpsi2mass = [Jpsimass]
    list2_1trk1trk_psi2mass = [Zcmass]

    for i in range(len(list2_1trk1trk_hypo)):
        oneTrkOneTrkArgs = { }
        oneTrkOneTrkArgs("HypothesisName" , list2_1trk1trk_hypo[i])
        oneTrkOneTrkArgs("Psi1Vertices" , list2_1trk1trk_psi1Input[i])
        oneTrkOneTrkArgs("Psi2Vertices" , list2_1trk1trk_psi2Input[i])
        oneTrkOneTrkArgs("MaxCandidates" , 15)
        oneTrkOneTrkArgs("NumberOfPsi1Daughters" , 3)
        oneTrkOneTrkArgs("NumberOfPsi2Daughters" , 3)
        oneTrkOneTrkArgs("Jpsi1MassLowerCut" , Jpsi_lo)
        oneTrkOneTrkArgs("Jpsi1MassUpperCut" , Jpsi_hi)
        oneTrkOneTrkArgs("Psi1MassLowerCut" , Zc_lo)
        oneTrkOneTrkArgs("Psi1MassUpperCut" , Zc_hi)
        oneTrkOneTrkArgs("Jpsi2MassLowerCut" , Jpsi_lo)
        oneTrkOneTrkArgs("Jpsi2MassUpperCut" , Jpsi_hi)
        oneTrkOneTrkArgs("Psi2MassLowerCut" , Zc_lo)
        oneTrkOneTrkArgs("Psi2MassUpperCut" , Zc_hi)
        oneTrkOneTrkArgs("MassLowerCut" , 0.)
        oneTrkOneTrkArgs("MassUpperCut" , X_hi)
        oneTrkOneTrkArgs("Jpsi1Mass" , list2_1trk1trk_jpsi1mass[i])
        oneTrkOneTrkArgs("Psi1Mass" , list2_1trk1trk_psi1mass[i])
        oneTrkOneTrkArgs("Jpsi2Mass" , list2_1trk1trk_jpsi2mass[i])
        oneTrkOneTrkArgs("Psi2Mass" , list2_1trk1trk_psi2mass[i])
        oneTrkOneTrkArgs("ApplyJpsi1MassConstraint" , True)
        oneTrkOneTrkArgs("ApplyPsi1MassConstraint" , True)
        oneTrkOneTrkArgs("ApplyJpsi2MassConstraint" , True)
        oneTrkOneTrkArgs("ApplyPsi2MassConstraint" , True)
        oneTrkOneTrkArgs("Chi2Cut" , 30.)
        oneTrkOneTrkArgs("PVRefitter" , PVrefit)
        oneTrkOneTrkArgs("TrkVertexFitterTool" , vkalvrt)
        oneTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrkOneTrkArgs("OutputVertexCollections" , ["BPHY23_"+list2_1trk1trk_hypo[i]+"_SubVtx1","BPHY23_"+list2_1trk1trk_hypo[i]+"_SubVtx2","BPHY23_"+list2_1trk1trk_hypo[i]+"_MainVtx"])
        oneTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        oneTrkOneTrkArgs("RefPVContainerName" , "BPHY23_"+list2_1trk1trk_hypo[i]+"_RefPrimaryVertices")
        oneTrkOneTrkArgs("RefitPV" , True)
        oneTrkOneTrkArgs("MaxnPV" , 100)
        OutputCollections += oneTrkOneTrkArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list2_1trk1trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list2_1trk1trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list2_1trk1trk_hypo[i] + "_MainVtx"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY23_"+list2_1trk1trk_hypo[i], **oneTrkOneTrkArgs) )

    list2_2trk1trk_hypo = ["Psi2Zc3900", "X3872Zc3900"]
    list2_2trk1trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872"]
    list2_2trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900"]
    list2_2trk1trk_jpsi1mass = [Jpsimass, Jpsimass]
    list2_2trk1trk_psi1mass = [Psi2Smass, X3872mass]
    list2_2trk1trk_jpsi2mass = [Jpsimass, Jpsimass]
    list2_2trk1trk_psi2mass = [Zcmass, Zcmass]

    for i in range(len(list2_2trk1trk_hypo)):
        twoTrkOneTrkArgs = { }
        twoTrkOneTrkArgs("HypothesisName" , list2_2trk1trk_hypo[i])
        twoTrkOneTrkArgs("Psi1Vertices" , list2_2trk1trk_psi1Input[i])
        twoTrkOneTrkArgs("Psi2Vertices" , list2_2trk1trk_psi2Input[i])
        twoTrkOneTrkArgs("MaxCandidates" , 15)
        twoTrkOneTrkArgs("NumberOfPsi1Daughters" , 4)
        twoTrkOneTrkArgs("NumberOfPsi2Daughters" , 3)
        twoTrkOneTrkArgs("Jpsi1MassLowerCut" , Jpsi_lo)
        twoTrkOneTrkArgs("Jpsi1MassUpperCut" , Jpsi_hi)
        twoTrkOneTrkArgs("Psi1MassLowerCut" , Psi_lo)
        twoTrkOneTrkArgs("Psi1MassUpperCut" , Psi_hi)
        twoTrkOneTrkArgs("Jpsi2MassLowerCut" , Jpsi_lo)
        twoTrkOneTrkArgs("Jpsi2MassUpperCut" , Jpsi_hi)
        twoTrkOneTrkArgs("Psi2MassLowerCut" , Zc_lo)
        twoTrkOneTrkArgs("Psi2MassUpperCut" , Zc_hi)
        twoTrkOneTrkArgs("MassLowerCut" , 0.)
        twoTrkOneTrkArgs("MassUpperCut" , X_hi)
        twoTrkOneTrkArgs("Jpsi1Mass" , list2_2trk1trk_jpsi1mass[i])
        twoTrkOneTrkArgs("Psi1Mass" , list2_2trk1trk_psi1mass[i])
        twoTrkOneTrkArgs("Jpsi2Mass" , list2_2trk1trk_jpsi2mass[i])
        twoTrkOneTrkArgs("Psi2Mass" , list2_2trk1trk_psi2mass[i])
        twoTrkOneTrkArgs("ApplyJpsi1MassConstraint" , True)
        twoTrkOneTrkArgs("ApplyPsi1MassConstraint" , True)
        twoTrkOneTrkArgs("ApplyJpsi2MassConstraint" , True)
        twoTrkOneTrkArgs("ApplyPsi2MassConstraint" , True)
        twoTrkOneTrkArgs("Chi2Cut" , 30.)
        twoTrkOneTrkArgs("PVRefitter" , PVrefit)
        twoTrkOneTrkArgs("TrkVertexFitterTool" , vkalvrt)
        twoTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkOneTrkArgs("OutputVertexCollections" , ["BPHY23_"+list2_2trk1trk_hypo[i]+"_SubVtx1","BPHY23_"+list2_2trk1trk_hypo[i]+"_SubVtx2","BPHY23_"+list2_2trk1trk_hypo[i]+"_MainVtx"])
        twoTrkOneTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkOneTrkArgs("RefPVContainerName" , "BPHY23_"+list2_2trk1trk_hypo[i]+"_RefPrimaryVertices")
        twoTrkOneTrkArgs("RefitPV" , True)
        twoTrkOneTrkArgs("MaxnPV" , 100)
        OutputCollections += twoTrkOneTrkArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list2_2trk1trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list2_2trk1trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list2_2trk1trk_hypo[i] + "_MainVtx"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY23_"+list2_2trk1trk_hypo[i], **twoTrkOneTrkArgs) )

    list2_2trk2trk_hypo = ["Psi2Psi2", "Psi2X3872", "X3872X3872"]
    list2_2trk2trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872"]
    list2_2trk2trk_psi2Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872"]
    list2_2trk2trk_jpsi1mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_2trk2trk_psi1mass = [Psi2Smass, Psi2Smass, X3872mass]
    list2_2trk2trk_jpsi2mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_2trk2trk_psi2mass = [Psi2Smass, X3872mass, X3872mass]

    for i in range(len(list2_2trk2trk_hypo)):
        twoTrkTwoTrkArgs = { }
        twoTrkTwoTrkArgs("HypothesisName" , list2_2trk2trk_hypo[i])
        twoTrkTwoTrkArgs("Psi1Vertices" , list2_2trk2trk_psi1Input[i])
        twoTrkTwoTrkArgs("Psi2Vertices" , list2_2trk2trk_psi2Input[i])
        twoTrkTwoTrkArgs("MaxCandidates" , 15)
        twoTrkTwoTrkArgs("NumberOfPsi1Daughters" , 4)
        twoTrkTwoTrkArgs("NumberOfPsi2Daughters" , 4)
        twoTrkTwoTrkArgs("Jpsi1MassLowerCut" , Jpsi_lo)
        twoTrkTwoTrkArgs("Jpsi1MassUpperCut" , Jpsi_hi)
        twoTrkTwoTrkArgs("Psi1MassLowerCut" , Psi_lo)
        twoTrkTwoTrkArgs("Psi1MassUpperCut" , Psi_hi)
        twoTrkTwoTrkArgs("Jpsi2MassLowerCut" , Jpsi_lo)
        twoTrkTwoTrkArgs("Jpsi2MassUpperCut" , Jpsi_hi)
        twoTrkTwoTrkArgs("Psi2MassLowerCut" , Psi_lo)
        twoTrkTwoTrkArgs("Psi2MassUpperCut" , Psi_hi)
        twoTrkTwoTrkArgs("MassLowerCut" , 0.)
        twoTrkTwoTrkArgs("MassUpperCut" , X_hi)
        twoTrkTwoTrkArgs("Jpsi1Mass" , list2_2trk2trk_jpsi1mass[i])
        twoTrkTwoTrkArgs("Psi1Mass" , list2_2trk2trk_psi1mass[i])
        twoTrkTwoTrkArgs("Jpsi2Mass" , list2_2trk2trk_jpsi2mass[i])
        twoTrkTwoTrkArgs("Psi2Mass" , list2_2trk2trk_psi2mass[i])
        twoTrkTwoTrkArgs("ApplyJpsi1MassConstraint" , True)
        twoTrkTwoTrkArgs("ApplyPsi1MassConstraint" , True)
        twoTrkTwoTrkArgs("ApplyJpsi2MassConstraint" , True)
        twoTrkTwoTrkArgs("ApplyPsi2MassConstraint" , True)
        twoTrkTwoTrkArgs("Chi2Cut" , 30.)
        twoTrkTwoTrkArgs("PVRefitter" , PVrefit)
        twoTrkTwoTrkArgs("TrkVertexFitterTool" , vkalvrt)
        twoTrkTwoTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkTwoTrkArgs("OutputVertexCollections" , ["BPHY23_"+list2_2trk2trk_hypo[i]+"_SubVtx1","BPHY23_"+list2_2trk2trk_hypo[i]+"_SubVtx2","BPHY23_"+list2_2trk2trk_hypo[i]+"_MainVtx"])
        twoTrkTwoTrkArgs("VxPrimaryCandidateName" , "PrimaryVertices")
        twoTrkTwoTrkArgs("RefPVContainerName" , "BPHY23_"+list2_2trk2trk_hypo[i]+"_RefPrimaryVertices")
        twoTrkTwoTrkArgs("RefitPV" , True)
        twoTrkTwoTrkArgs("MaxnPV" , 100)
        OutputCollections += twoTrkTwoTrkArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + list2_2trk2trk_hypo[i] + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + list2_2trk2trk_hypo[i] + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + list2_2trk2trk_hypo[i] + "_MainVtx"]
        acc.addEventAlgo( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY23_"+list2_2trk2trk_hypo[i], **twoTrkTwoTrkArgs) )

    BPHY23_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY23_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY23_SelectEvent)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY23Kernel",
        SkimmingTools     = [BPHY23_SelectEvent]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY23SlimmingHelper = SlimmingHelper("BPHY23SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY23_AllVariables  = getDefaultAllVariables()
    BPHY23_StaticContent = []

    # Needed for trigger objects
    BPHY23SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY23SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    BPHY23_AllVariables += ["PrimaryVertices"]
    BPHY23_StaticContent += RefPVContainers
    BPHY23_StaticContent += RefPVAuxContainers

    ## ID track particles
    BPHY23_AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are stored in InDetTrackParticles collection)
    BPHY23_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    ## muon container
    BPHY23_AllVariables += ["Muons", "MuonSegments"]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for output in OutputCollections:
        BPHY23_StaticContent += ["xAOD::VertexContainer#%s" % output]
        BPHY23_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % output]

    # Truth information for MC only
    if isSimulation:
        BPHY23_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY23SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
    BPHY23SlimmingHelper.AllVariables = BPHY23_AllVariables
    BPHY23SlimmingHelper.StaticContent = BPHY23_StaticContent

    BPHY23ItemList = BPHY23SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY23", ItemList=BPHY23ItemList, AcceptAlgs=["BPHY23Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY23", AcceptAlgs=["BPHY23Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
