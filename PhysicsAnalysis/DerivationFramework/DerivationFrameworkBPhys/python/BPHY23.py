
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import (BPHY_V0ToolCfg,  BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg, Reco_VertexCfg, ReVertexCfg, JpsiPlusPsiCascadeCfg, PsiPlusPsiSingleVertexCfg, PsiPlusPsiCascadeCfg, Select_onia2mumuCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC

    # Configure common Public Tools
    V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName)))
    vkalvrt = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName))) # VKalVrt vertex fitter
    trackselect = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName)))
    PVrefit = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)))

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

    augmentation_tools = []

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23_Reco_mumu",
                       VertexSearchTool = CompFactory.Analysis.JpsiFinder(
                           name = "BPHY23JpsiFinder",
                           muAndMu = True,
                           muAndTrack = False,
                           TrackAndTrack = False,
                           assumeDiMuons = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
                           trackThresholdPt = 2400.,
                           invMassLower = Jpsi_lo,
                           invMassUpper = Upsi_hi,
                           Chi2Cut = 10.,
                           oppChargesOnly = True,
                           atLeastOneComb = True,
                           useCombinedMeasurement = False, # Only takes effect if combOnly=True
                           muonCollectionKey = "Muons",
                           TrackParticleCollection = "InDetTrackParticles",
                           TrkVertexFitterTool = vkalvrt, # PublicToolHandle
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           VertexPointEstimator = acc.addPublicTool(acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))), # PublicToolHandle
                           useMCPCuts = False ),
                       OutputVtxContainerName = "BPHY23OniaCandidates",
                       PVContainerName = "PrimaryVertices",
                       RefPVContainerName = "SHOULDNOTBEUSED",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       DoVertexType = 1)
    )))


    # X(3872), Psi(2S) -> J/psi + pi pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23FourTrackReco_PsiX3872",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus2Tracks(
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
                           TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = False),
                       OutputVtxContainerName = "BPHY23FourTrack_PsiX3872",
                       PVContainerName = "PrimaryVertices",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       RefitPV = False,
                       DoVertexType = 0)
    )))


    # Bs0 -> J/psi + K K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23FourTrackReco_Bs0",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus2Tracks(
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
                           TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = False),
                       OutputVtxContainerName = "BPHY23FourTrack_Bs0",
                       PVContainerName = "PrimaryVertices",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       RefitPV = False,
                       DoVertexType = 0)
    )))

    # B0 -> J/psi + K pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23FourTrackReco_B0",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus2Tracks(
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
                           TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = False),
                       OutputVtxContainerName = "BPHY23FourTrack_B0",
                       PVContainerName = "PrimaryVertices",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       RefitPV = False,
                       DoVertexType = 0)
    )))

    # Zc(3900)+ -> J/psi pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23ThreeTrackReco_Zc3900",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus1Track(
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
                           TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = False),
                       OutputVtxContainerName = "BPHY23ThreeTrack_Zc3900",
                       PVContainerName = "PrimaryVertices",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       RefitPV = False,
                       DoVertexType = 0)
    )))

    # B+ -> J/psi K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name = "BPHY23ThreeTrackReco_Bpm",
                       VertexSearchTool = CompFactory.Analysis.JpsiPlus1Track(
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
                           TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                           TrackSelectorTool = trackselect, # PublicToolHandle
                           UseMassConstraint = False),
                       OutputVtxContainerName = "BPHY23ThreeTrack_Bpm",
                       PVContainerName = "PrimaryVertices",
                       V0Tools = V0Tools, # PublicToolHandle
                       PVRefitter = PVrefit, #PublicToolHandle
                       RefitPV = False,
                       DoVertexType = 0)
    )))

    # revertex with mass constraints to reduce combinatorics
    # Psi(2S) -> J/psi pi pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Psi4Body")
    )))

    # X(3872) -> J/psi pi pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_X3872")
    )))

    # Bs0 -> J/psi K K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Bs0")
    )))

    # B0 -> J/psi K pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_B0Kpi")
    )))

    # B0 -> J/psi pi K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_B0piK")
    )))

    # Zc3900 -> J/psi pi
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Zc3900")
    )))

    # Bpm -> J/psi K
    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
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
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Bpm")
    )))


    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Select_onia2mumuCfg(flags,
                            name = "BPHY23Select_Jpsi",
                            HypothesisName = "Jpsi",
                            InputVtxContainerName = "BPHY23OniaCandidates",
                            V0Tools = V0Tools, # PublicToolHandle
                            TrkMasses = [Mumass, Mumass],
                            MassMin = Jpsi_lo,
                            MassMax = Jpsi_hi,
                            DoVertexType = 0)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Select_onia2mumuCfg(flags,
                            name = "BPHY23Select_Psi",
                            HypothesisName = "Psi",
                            InputVtxContainerName = "BPHY23OniaCandidates",
                            V0Tools = V0Tools, # PublicToolHandle
                            TrkMasses = [Mumass, Mumass],
                            MassMin = Psi_lo,
                            MassMax = Psi_hi,
                            DoVertexType = 0)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Select_onia2mumuCfg(flags,
                            name = "BPHY23Select_Upsi",
                            HypothesisName = "Upsi",
                            InputVtxContainerName = "BPHY23OniaCandidates",
                            V0Tools = V0Tools, # PublicToolHandle
                            TrkMasses = [Mumass, Mumass],
                            MassMin = Upsi_lo,
                            MassMax = Upsi_hi,
                            DoVertexType = 0)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name = "BPHY23Rev_Jpsi",
                    InputVtxContainerName = "BPHY23OniaCandidates",
                    HypothesisNames = [ "Jpsi" ],
                    TrackIndices = [ 0, 1 ],
                    RefitPV = False,
                    UseMassConstraint = True,
                    VertexMass = Jpsimass,
                    MassInputParticles = [Mumass, Mumass],
                    Chi2Cut = 50.,
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Jpsi")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name = "BPHY23Rev_Psi",
                    InputVtxContainerName = "BPHY23OniaCandidates",
                    HypothesisNames = [ "Psi" ],
                    TrackIndices = [ 0, 1 ],
                    RefitPV = False,
                    UseMassConstraint = True,
                    VertexMass = Psi2Smass,
                    MassInputParticles = [Mumass, Mumass],
                    Chi2Cut = 50.,
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Psi")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name = "BPHY23Rev_Upsi",
                    InputVtxContainerName = "BPHY23OniaCandidates",
                    HypothesisNames = [ "Upsi" ],
                    TrackIndices = [ 0, 1 ],
                    RefitPV = False,
                    UseMassConstraint = True,
                    VertexMass = Upsimass,
                    MassInputParticles = [Mumass, Mumass],
                    Chi2Cut = 50.,
                    TrkVertexFitterTool = vkalvrt, # VKalVrt vertex fitter (PublicToolHandle)
                    PVRefitter = PVrefit, #PublicToolHandle
                    V0Tools = V0Tools, # PublicToolHandle
                    OutputVtxContainerName = "BPHY23Revtx_Upsi")
    )))


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

    for i, hypo in enumerate(list_2trk0trk_hypo):
        TwoTrk0TrkArgs = { }
        TwoTrk0TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk0TrkArgs.setdefault("JpsiVertices", list_2trk0trk_jpsiInput[i])
        TwoTrk0TrkArgs.setdefault("PsiVertices", list_2trk0trk_psiInput[i])
        TwoTrk0TrkArgs.setdefault("NumberOfPsiDaughters", 4)
        TwoTrk0TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk0TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk0TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk0TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk0TrkArgs.setdefault("MaxnPV", 100)
        TwoTrk0TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk0TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk0TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk0TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        TwoTrk0TrkArgs.setdefault("RefitPV", True)
        TwoTrk0TrkArgs.setdefault("Vtx1Daug3MassHypo", list_2trk0trk_dau3Mass[i])
        TwoTrk0TrkArgs.setdefault("Vtx1Daug4MassHypo", list_2trk0trk_dau4Mass[i])
        TwoTrk0TrkArgs.setdefault("ApplyJpsiMassConstraint", True)
        TwoTrk0TrkArgs.setdefault("JpsiMass", list_2trk0trk_jpsiMass[i])
        TwoTrk0TrkArgs.setdefault("ApplyPsiMassConstraint", True)
        TwoTrk0TrkArgs.setdefault("PsiMass", list_2trk0trk_psiMass[i])
        TwoTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk0TrkArgs.setdefault("Jpsi2Mass", list_2trk0trk_jpsi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **TwoTrk0TrkArgs)
        )))
        OutputCollections += TwoTrk0TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

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

    for i, hypo in enumerate(list2_2trk0trk_hypo):
        TwoTrk0TrkArgs = { }
        TwoTrk0TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk0TrkArgs.setdefault("Psi1Vertices", list2_2trk0trk_psi1Input[i])
        TwoTrk0TrkArgs.setdefault("Psi2Vertices", list2_2trk0trk_psi2Input[i])
        TwoTrk0TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk0TrkArgs.setdefault("NumberOfPsi1Daughters", 4)
        TwoTrk0TrkArgs.setdefault("NumberOfPsi2Daughters", 2)
        TwoTrk0TrkArgs.setdefault("Jpsi1MassLowerCut", Jpsi_lo)
        TwoTrk0TrkArgs.setdefault("Jpsi1MassUpperCut", Jpsi_hi)
        TwoTrk0TrkArgs.setdefault("Psi1MassLowerCut", Psi_lo)
        TwoTrk0TrkArgs.setdefault("Psi1MassUpperCut", Psi_hi)
        TwoTrk0TrkArgs.setdefault("Jpsi2MassLowerCut", list2_2trk0trk_jpsi2lo[i])
        TwoTrk0TrkArgs.setdefault("Jpsi2MassUpperCut", list2_2trk0trk_jpsi2hi[i])
        TwoTrk0TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk0TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk0TrkArgs.setdefault("Jpsi1Mass", list2_2trk0trk_jpsi1mass[i])
        TwoTrk0TrkArgs.setdefault("Psi1Mass", list2_2trk0trk_psi1mass[i])
        TwoTrk0TrkArgs.setdefault("Jpsi2Mass", list2_2trk0trk_jpsi2mass[i])
        TwoTrk0TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        TwoTrk0TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        TwoTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk0TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk0TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk0TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk0TrkArgs.setdefault("OutputVertexCollections", ["BPHY23_"+hypo+"_SubVtx1","BPHY23_"+hypo+"_SubVtx2","BPHY23_"+hypo+"_MainVtx"])
        TwoTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk0TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk0TrkArgs.setdefault("RefitPV", True)
        TwoTrk0TrkArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name = "BPHY23_"+hypo, **TwoTrk0TrkArgs)
        )))
        OutputCollections += TwoTrk0TrkArgs["OutputVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_MainVtx"]

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

    for i, hypo in enumerate(list_1trk0trk_hypo):
        OneTrk0TrkArgs = { }
        OneTrk0TrkArgs.setdefault("HypothesisName", hypo)
        OneTrk0TrkArgs.setdefault("JpsiVertices", list_1trk0trk_jpsiInput[i])
        OneTrk0TrkArgs.setdefault("PsiVertices", list_1trk0trk_psiInput[i])
        OneTrk0TrkArgs.setdefault("NumberOfPsiDaughters", 3)
        OneTrk0TrkArgs.setdefault("MassLowerCut", 0.)
        OneTrk0TrkArgs.setdefault("MassUpperCut", X_hi)
        OneTrk0TrkArgs.setdefault("Chi2Cut", 30.)
        OneTrk0TrkArgs.setdefault("MaxCandidates", 15)
        OneTrk0TrkArgs.setdefault("MaxnPV", 100)
        OneTrk0TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        OneTrk0TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        OneTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk0TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        OneTrk0TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        OneTrk0TrkArgs.setdefault("RefitPV", True)
        OneTrk0TrkArgs.setdefault("Vtx1Daug3MassHypo", list_1trk0trk_dau3Mass[i])
        OneTrk0TrkArgs.setdefault("ApplyJpsiMassConstraint", True)
        OneTrk0TrkArgs.setdefault("JpsiMass", list_1trk0trk_jpsiMass[i])
        OneTrk0TrkArgs.setdefault("ApplyPsiMassConstraint", True)
        OneTrk0TrkArgs.setdefault("PsiMass", list_1trk0trk_psiMass[i])
        OneTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        OneTrk0TrkArgs.setdefault("Jpsi2Mass", list_1trk0trk_jpsi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            JpsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **OneTrk0TrkArgs)
        )))
        OutputCollections += OneTrk0TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

    list2_1trk0trk_hypo = ["Zc3900Jpsi0", "Zc3900Psi0", "Zc3900Upsi0"]
    list2_1trk0trk_psi1Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900"]
    list2_1trk0trk_psi2Input = ["BPHY23Revtx_Jpsi", "BPHY23Revtx_Psi", "BPHY23Revtx_Upsi"]
    list2_1trk0trk_jpsi2lo = [Jpsi_lo, Psi_lo, Upsi_lo]
    list2_1trk0trk_jpsi2hi = [Jpsi_hi, Psi_hi, Upsi_hi]
    list2_1trk0trk_jpsi1mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_1trk0trk_psi1mass = [Zcmass, Zcmass, Zcmass]
    list2_1trk0trk_jpsi2mass = [Jpsimass, Psi2Smass, Upsimass]

    for i, hypo in enumerate(list2_1trk0trk_hypo):
        OneTrk0TrkArgs = { }
        OneTrk0TrkArgs.setdefault("HypothesisName", hypo)
        OneTrk0TrkArgs.setdefault("Psi1Vertices", list2_1trk0trk_psi1Input[i])
        OneTrk0TrkArgs.setdefault("Psi2Vertices", list2_1trk0trk_psi2Input[i])
        OneTrk0TrkArgs.setdefault("MaxCandidates", 15)
        OneTrk0TrkArgs.setdefault("NumberOfPsi1Daughters", 3)
        OneTrk0TrkArgs.setdefault("NumberOfPsi2Daughters", 2)
        OneTrk0TrkArgs.setdefault("Jpsi1MassLowerCut", Jpsi_lo)
        OneTrk0TrkArgs.setdefault("Jpsi1MassUpperCut", Jpsi_hi)
        OneTrk0TrkArgs.setdefault("Psi1MassLowerCut", Zc_lo)
        OneTrk0TrkArgs.setdefault("Psi1MassUpperCut", Zc_hi)
        OneTrk0TrkArgs.setdefault("Jpsi2MassLowerCut", list2_1trk0trk_jpsi2lo[i])
        OneTrk0TrkArgs.setdefault("Jpsi2MassUpperCut", list2_1trk0trk_jpsi2hi[i])
        OneTrk0TrkArgs.setdefault("MassLowerCut", 0.)
        OneTrk0TrkArgs.setdefault("MassUpperCut", X_hi)
        OneTrk0TrkArgs.setdefault("Jpsi1Mass", list2_1trk0trk_jpsi1mass[i])
        OneTrk0TrkArgs.setdefault("Psi1Mass", list2_1trk0trk_psi1mass[i])
        OneTrk0TrkArgs.setdefault("Jpsi2Mass", list2_1trk0trk_jpsi2mass[i])
        OneTrk0TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        OneTrk0TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        OneTrk0TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        OneTrk0TrkArgs.setdefault("Chi2Cut", 30.)
        OneTrk0TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        OneTrk0TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        OneTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk0TrkArgs.setdefault("OutputVertexCollections", ["BPHY23_"+hypo+"_SubVtx1","BPHY23_"+hypo+"_SubVtx2","BPHY23_"+hypo+"_MainVtx"])
        OneTrk0TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk0TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        OneTrk0TrkArgs.setdefault("RefitPV", True)
        OneTrk0TrkArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name = "BPHY23_"+hypo, **OneTrk0TrkArgs)
        )))
        OutputCollections += OneTrk0TrkArgs["OutputVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_MainVtx"]

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

    for i, hypo in enumerate(list_1trk1trk_hypo):
        OneTrk1TrkArgs = { }
        OneTrk1TrkArgs.setdefault("HypothesisName", hypo)
        OneTrk1TrkArgs.setdefault("Psi1Vertices", list_1trk1trk_psi1Input[i])
        OneTrk1TrkArgs.setdefault("Psi2Vertices", list_1trk1trk_psi2Input[i])
        OneTrk1TrkArgs.setdefault("NumberOfPsi1Daughters", 3)
        OneTrk1TrkArgs.setdefault("NumberOfPsi2Daughters", 3)
        OneTrk1TrkArgs.setdefault("MassLowerCut", 0.)
        OneTrk1TrkArgs.setdefault("MassUpperCut", X_hi)
        OneTrk1TrkArgs.setdefault("Chi2CutPsi1", 5.)
        OneTrk1TrkArgs.setdefault("Chi2CutPsi2", 5.)
        OneTrk1TrkArgs.setdefault("Chi2Cut", 30.)
        OneTrk1TrkArgs.setdefault("MaxCandidates", 15)
        OneTrk1TrkArgs.setdefault("RemoveDuplicatePairs", True)
        OneTrk1TrkArgs.setdefault("MaxnPV", 100)
        OneTrk1TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        OneTrk1TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        OneTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk1TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        OneTrk1TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        OneTrk1TrkArgs.setdefault("RefitPV", True)
        OneTrk1TrkArgs.setdefault("Vtx1Daug3MassHypo", list_1trk1trk_p1dau3Mass[i])
        OneTrk1TrkArgs.setdefault("Vtx2Daug3MassHypo", list_1trk1trk_p2dau3Mass[i])
        OneTrk1TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        OneTrk1TrkArgs.setdefault("Jpsi1Mass", list_1trk1trk_jpsi1Mass[i])
        OneTrk1TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        OneTrk1TrkArgs.setdefault("Psi1Mass", list_1trk1trk_psi1Mass[i])
        OneTrk1TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        OneTrk1TrkArgs.setdefault("Jpsi2Mass", list_1trk1trk_jpsi2Mass[i])
        if list_1trk1trk_psi2Mass[i] != Zcmass:
            OneTrk1TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
            OneTrk1TrkArgs.setdefault("Psi2Mass", list_1trk1trk_psi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **OneTrk1TrkArgs)
        )))
        OutputCollections += OneTrk1TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

    list2_1trk1trk_hypo = ["Zc3900Zc3900"]
    list2_1trk1trk_psi1Input = ["BPHY23Revtx_Zc3900"]
    list2_1trk1trk_psi2Input = ["BPHY23Revtx_Zc3900"]
    list2_1trk1trk_jpsi1mass = [Jpsimass]
    list2_1trk1trk_psi1mass = [Zcmass]
    list2_1trk1trk_jpsi2mass = [Jpsimass]
    list2_1trk1trk_psi2mass = [Zcmass]

    for i, hypo in enumerate(list2_1trk1trk_hypo):
        OneTrk1TrkArgs = { }
        OneTrk1TrkArgs.setdefault("HypothesisName", hypo)
        OneTrk1TrkArgs.setdefault("Psi1Vertices", list2_1trk1trk_psi1Input[i])
        OneTrk1TrkArgs.setdefault("Psi2Vertices", list2_1trk1trk_psi2Input[i])
        OneTrk1TrkArgs.setdefault("MaxCandidates", 15)
        OneTrk1TrkArgs.setdefault("NumberOfPsi1Daughters", 3)
        OneTrk1TrkArgs.setdefault("NumberOfPsi2Daughters", 3)
        OneTrk1TrkArgs.setdefault("Jpsi1MassLowerCut", Jpsi_lo)
        OneTrk1TrkArgs.setdefault("Jpsi1MassUpperCut", Jpsi_hi)
        OneTrk1TrkArgs.setdefault("Psi1MassLowerCut", Zc_lo)
        OneTrk1TrkArgs.setdefault("Psi1MassUpperCut", Zc_hi)
        OneTrk1TrkArgs.setdefault("Jpsi2MassLowerCut", Jpsi_lo)
        OneTrk1TrkArgs.setdefault("Jpsi2MassUpperCut", Jpsi_hi)
        OneTrk1TrkArgs.setdefault("Psi2MassLowerCut", Zc_lo)
        OneTrk1TrkArgs.setdefault("Psi2MassUpperCut", Zc_hi)
        OneTrk1TrkArgs.setdefault("MassLowerCut", 0.)
        OneTrk1TrkArgs.setdefault("MassUpperCut", X_hi)
        OneTrk1TrkArgs.setdefault("Jpsi1Mass", list2_1trk1trk_jpsi1mass[i])
        OneTrk1TrkArgs.setdefault("Psi1Mass", list2_1trk1trk_psi1mass[i])
        OneTrk1TrkArgs.setdefault("Jpsi2Mass", list2_1trk1trk_jpsi2mass[i])
        OneTrk1TrkArgs.setdefault("Psi2Mass", list2_1trk1trk_psi2mass[i])
        OneTrk1TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        OneTrk1TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        OneTrk1TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        OneTrk1TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
        OneTrk1TrkArgs.setdefault("Chi2Cut", 30.)
        OneTrk1TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        OneTrk1TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        OneTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk1TrkArgs.setdefault("OutputVertexCollections", ["BPHY23_"+hypo+"_SubVtx1","BPHY23_"+hypo+"_SubVtx2","BPHY23_"+hypo+"_MainVtx"])
        OneTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk1TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        OneTrk1TrkArgs.setdefault("RefitPV", True)
        OneTrk1TrkArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name = "BPHY23_"+hypo, **OneTrk1TrkArgs)
        )))
        OutputCollections += OneTrk1TrkArgs["OutputVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_MainVtx"]

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

    for i, hypo in enumerate(list_2trk1trk_hypo):
        TwoTrk1TrkArgs = { }
        TwoTrk1TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk1TrkArgs.setdefault("Psi1Vertices", list_2trk1trk_psi1Input[i])
        TwoTrk1TrkArgs.setdefault("Psi2Vertices", list_2trk1trk_psi2Input[i])
        TwoTrk1TrkArgs.setdefault("NumberOfPsi1Daughters", 4)
        TwoTrk1TrkArgs.setdefault("NumberOfPsi2Daughters", 3)
        TwoTrk1TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk1TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk1TrkArgs.setdefault("Chi2CutPsi1", 5.)
        TwoTrk1TrkArgs.setdefault("Chi2CutPsi2", 5.)
        TwoTrk1TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk1TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk1TrkArgs.setdefault("MaxnPV", 100)
        TwoTrk1TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk1TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk1TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk1TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        TwoTrk1TrkArgs.setdefault("RefitPV", True)
        TwoTrk1TrkArgs.setdefault("Vtx1Daug3MassHypo", list_2trk1trk_p1dau3Mass[i])
        TwoTrk1TrkArgs.setdefault("Vtx1Daug4MassHypo", list_2trk1trk_p1dau4Mass[i])
        TwoTrk1TrkArgs.setdefault("Vtx2Daug3MassHypo", list_2trk1trk_p2dau3Mass[i])
        TwoTrk1TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("Jpsi1Mass", list_2trk1trk_jpsi1Mass[i])
        TwoTrk1TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("Psi1Mass", list_2trk1trk_psi1Mass[i])
        TwoTrk1TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("Jpsi2Mass", list_2trk1trk_jpsi2Mass[i])
        if list_2trk1trk_psi2Mass[i] != Zcmass:
            TwoTrk1TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
            TwoTrk1TrkArgs.setdefault("Psi2Mass", list_2trk1trk_psi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **TwoTrk1TrkArgs)
        )))
        OutputCollections += TwoTrk1TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

    list2_2trk1trk_hypo = ["Psi2Zc3900", "X3872Zc3900"]
    list2_2trk1trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872"]
    list2_2trk1trk_psi2Input = ["BPHY23Revtx_Zc3900", "BPHY23Revtx_Zc3900"]
    list2_2trk1trk_jpsi1mass = [Jpsimass, Jpsimass]
    list2_2trk1trk_psi1mass = [Psi2Smass, X3872mass]
    list2_2trk1trk_jpsi2mass = [Jpsimass, Jpsimass]
    list2_2trk1trk_psi2mass = [Zcmass, Zcmass]

    for i, hypo in enumerate(list2_2trk1trk_hypo):
        TwoTrk1TrkArgs = { }
        TwoTrk1TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk1TrkArgs.setdefault("Psi1Vertices", list2_2trk1trk_psi1Input[i])
        TwoTrk1TrkArgs.setdefault("Psi2Vertices", list2_2trk1trk_psi2Input[i])
        TwoTrk1TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk1TrkArgs.setdefault("NumberOfPsi1Daughters", 4)
        TwoTrk1TrkArgs.setdefault("NumberOfPsi2Daughters", 3)
        TwoTrk1TrkArgs.setdefault("Jpsi1MassLowerCut", Jpsi_lo)
        TwoTrk1TrkArgs.setdefault("Jpsi1MassUpperCut", Jpsi_hi)
        TwoTrk1TrkArgs.setdefault("Psi1MassLowerCut", Psi_lo)
        TwoTrk1TrkArgs.setdefault("Psi1MassUpperCut", Psi_hi)
        TwoTrk1TrkArgs.setdefault("Jpsi2MassLowerCut", Jpsi_lo)
        TwoTrk1TrkArgs.setdefault("Jpsi2MassUpperCut", Jpsi_hi)
        TwoTrk1TrkArgs.setdefault("Psi2MassLowerCut", Zc_lo)
        TwoTrk1TrkArgs.setdefault("Psi2MassUpperCut", Zc_hi)
        TwoTrk1TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk1TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk1TrkArgs.setdefault("Jpsi1Mass", list2_2trk1trk_jpsi1mass[i])
        TwoTrk1TrkArgs.setdefault("Psi1Mass", list2_2trk1trk_psi1mass[i])
        TwoTrk1TrkArgs.setdefault("Jpsi2Mass", list2_2trk1trk_jpsi2mass[i])
        TwoTrk1TrkArgs.setdefault("Psi2Mass", list2_2trk1trk_psi2mass[i])
        TwoTrk1TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
        TwoTrk1TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk1TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk1TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk1TrkArgs.setdefault("OutputVertexCollections", ["BPHY23_"+hypo+"_SubVtx1","BPHY23_"+hypo+"_SubVtx2","BPHY23_"+hypo+"_MainVtx"])
        TwoTrk1TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk1TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk1TrkArgs.setdefault("RefitPV", True)
        TwoTrk1TrkArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name = "BPHY23_"+hypo, **TwoTrk1TrkArgs)
        )))
        OutputCollections += TwoTrk1TrkArgs["OutputVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_MainVtx"]

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

    for i, hypo in enumerate(list_1trk2trk_hypo):
        OneTrk2TrkArgs = { }
        OneTrk2TrkArgs.setdefault("HypothesisName", hypo)
        OneTrk2TrkArgs.setdefault("Psi1Vertices", list_1trk2trk_psi1Input[i])
        OneTrk2TrkArgs.setdefault("Psi2Vertices", list_1trk2trk_psi2Input[i])
        OneTrk2TrkArgs.setdefault("NumberOfPsi1Daughters", 3)
        OneTrk2TrkArgs.setdefault("NumberOfPsi2Daughters", 4)
        OneTrk2TrkArgs.setdefault("MassLowerCut", 0.)
        OneTrk2TrkArgs.setdefault("MassUpperCut", X_hi)
        OneTrk2TrkArgs.setdefault("Chi2CutPsi1", 5.)
        OneTrk2TrkArgs.setdefault("Chi2CutPsi2", 5.)
        OneTrk2TrkArgs.setdefault("Chi2Cut", 30.)
        OneTrk2TrkArgs.setdefault("MaxCandidates", 15)
        OneTrk2TrkArgs.setdefault("MaxnPV", 100)
        OneTrk2TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        OneTrk2TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        OneTrk2TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        OneTrk2TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        OneTrk2TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        OneTrk2TrkArgs.setdefault("RefitPV", True)
        OneTrk2TrkArgs.setdefault("Vtx1Daug3MassHypo", list_1trk2trk_p1dau3Mass[i])
        OneTrk2TrkArgs.setdefault("Vtx2Daug3MassHypo", list_1trk2trk_p2dau3Mass[i])
        OneTrk2TrkArgs.setdefault("Vtx2Daug4MassHypo", list_1trk2trk_p2dau4Mass[i])
        OneTrk2TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        OneTrk2TrkArgs.setdefault("Jpsi1Mass", list_1trk2trk_jpsi1Mass[i])
        OneTrk2TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        OneTrk2TrkArgs.setdefault("Psi1Mass", list_1trk2trk_psi1Mass[i])
        OneTrk2TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        OneTrk2TrkArgs.setdefault("Jpsi2Mass", list_1trk2trk_jpsi2Mass[i])
        OneTrk2TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
        OneTrk2TrkArgs.setdefault("Psi2Mass", list_1trk2trk_psi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **OneTrk2TrkArgs)
        )))
        OutputCollections += OneTrk2TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

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

    for i, hypo in enumerate(list_2trk2trk_hypo):
        TwoTrk2TrkArgs = { }
        TwoTrk2TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk2TrkArgs.setdefault("Psi1Vertices", list_2trk2trk_psi1Input[i])
        TwoTrk2TrkArgs.setdefault("Psi2Vertices", list_2trk2trk_psi2Input[i])
        TwoTrk2TrkArgs.setdefault("NumberOfPsi1Daughters", 4)
        TwoTrk2TrkArgs.setdefault("NumberOfPsi2Daughters", 4)
        TwoTrk2TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk2TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk2TrkArgs.setdefault("Chi2CutPsi1", 4.)
        TwoTrk2TrkArgs.setdefault("Chi2CutPsi2", 4.)
        TwoTrk2TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk2TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk2TrkArgs.setdefault("RemoveDuplicatePairs", True)
        TwoTrk2TrkArgs.setdefault("MaxnPV", 100)
        TwoTrk2TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk2TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk2TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk2TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk2TrkArgs.setdefault("CascadeVertexCollections", ["BPHY23_"+hypo+"_CascadeVtx1","BPHY23_"+hypo+"_CascadeVtx2","BPHY23_"+hypo+"_CascadeVtx3"])
        TwoTrk2TrkArgs.setdefault("RefitPV", True)
        TwoTrk2TrkArgs.setdefault("Vtx1Daug3MassHypo", list_2trk2trk_p1dau3Mass[i])
        TwoTrk2TrkArgs.setdefault("Vtx1Daug4MassHypo", list_2trk2trk_p1dau4Mass[i])
        TwoTrk2TrkArgs.setdefault("Vtx2Daug3MassHypo", list_2trk2trk_p2dau3Mass[i])
        TwoTrk2TrkArgs.setdefault("Vtx2Daug4MassHypo", list_2trk2trk_p2dau4Mass[i])
        TwoTrk2TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("Jpsi1Mass", list_2trk2trk_jpsi1Mass[i])
        TwoTrk2TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("Psi1Mass", list_2trk2trk_psi1Mass[i])
        TwoTrk2TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("Jpsi2Mass", list_2trk2trk_jpsi2Mass[i])
        TwoTrk2TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("Psi2Mass", list_2trk2trk_psi2Mass[i])
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiCascadeCfg(flags, name = "BPHY23_"+hypo, **TwoTrk2TrkArgs)
        )))
        OutputCollections += TwoTrk2TrkArgs["CascadeVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_CascadeVtx3"]

    list2_2trk2trk_hypo = ["Psi2Psi2", "Psi2X3872", "X3872X3872"]
    list2_2trk2trk_psi1Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872"]
    list2_2trk2trk_psi2Input = ["BPHY23Revtx_Psi4Body", "BPHY23Revtx_X3872", "BPHY23Revtx_X3872"]
    list2_2trk2trk_jpsi1mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_2trk2trk_psi1mass = [Psi2Smass, Psi2Smass, X3872mass]
    list2_2trk2trk_jpsi2mass = [Jpsimass, Jpsimass, Jpsimass]
    list2_2trk2trk_psi2mass = [Psi2Smass, X3872mass, X3872mass]

    for i, hypo in enumerate(list2_2trk2trk_hypo):
        TwoTrk2TrkArgs = { }
        TwoTrk2TrkArgs.setdefault("HypothesisName", hypo)
        TwoTrk2TrkArgs.setdefault("Psi1Vertices", list2_2trk2trk_psi1Input[i])
        TwoTrk2TrkArgs.setdefault("Psi2Vertices", list2_2trk2trk_psi2Input[i])
        TwoTrk2TrkArgs.setdefault("MaxCandidates", 15)
        TwoTrk2TrkArgs.setdefault("NumberOfPsi1Daughters", 4)
        TwoTrk2TrkArgs.setdefault("NumberOfPsi2Daughters", 4)
        TwoTrk2TrkArgs.setdefault("Jpsi1MassLowerCut", Jpsi_lo)
        TwoTrk2TrkArgs.setdefault("Jpsi1MassUpperCut", Jpsi_hi)
        TwoTrk2TrkArgs.setdefault("Psi1MassLowerCut", Psi_lo)
        TwoTrk2TrkArgs.setdefault("Psi1MassUpperCut", Psi_hi)
        TwoTrk2TrkArgs.setdefault("Jpsi2MassLowerCut", Jpsi_lo)
        TwoTrk2TrkArgs.setdefault("Jpsi2MassUpperCut", Jpsi_hi)
        TwoTrk2TrkArgs.setdefault("Psi2MassLowerCut", Psi_lo)
        TwoTrk2TrkArgs.setdefault("Psi2MassUpperCut", Psi_hi)
        TwoTrk2TrkArgs.setdefault("MassLowerCut", 0.)
        TwoTrk2TrkArgs.setdefault("MassUpperCut", X_hi)
        TwoTrk2TrkArgs.setdefault("Jpsi1Mass", list2_2trk2trk_jpsi1mass[i])
        TwoTrk2TrkArgs.setdefault("Psi1Mass", list2_2trk2trk_psi1mass[i])
        TwoTrk2TrkArgs.setdefault("Jpsi2Mass", list2_2trk2trk_jpsi2mass[i])
        TwoTrk2TrkArgs.setdefault("Psi2Mass", list2_2trk2trk_psi2mass[i])
        TwoTrk2TrkArgs.setdefault("ApplyJpsi1MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("ApplyPsi1MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("ApplyJpsi2MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("ApplyPsi2MassConstraint", True)
        TwoTrk2TrkArgs.setdefault("Chi2Cut", 30.)
        TwoTrk2TrkArgs.setdefault("PVRefitter", PVrefit) # PublicToolHandle
        TwoTrk2TrkArgs.setdefault("TrkVertexFitterTool", vkalvrt) # PublicToolHandle
        TwoTrk2TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk2TrkArgs.setdefault("OutputVertexCollections", ["BPHY23_"+hypo+"_SubVtx1","BPHY23_"+hypo+"_SubVtx2","BPHY23_"+hypo+"_MainVtx"])
        TwoTrk2TrkArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        TwoTrk2TrkArgs.setdefault("RefPVContainerName", "BPHY23_"+hypo+"_RefPrimaryVertices")
        TwoTrk2TrkArgs.setdefault("RefitPV", True)
        TwoTrk2TrkArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name = "BPHY23_"+hypo, **TwoTrk2TrkArgs)
        )))
        OutputCollections += TwoTrk2TrkArgs["OutputVertexCollections"]
        RefPVContainers += ["xAOD::VertexContainer#BPHY23_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY23_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY23_" + hypo + "_MainVtx"]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY23Kernel",
        AugmentationTools = augmentation_tools,
        SkimmingTools     = [acc.addPublicTool(CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY23_SelectEvent", VertexContainerNames = passedCandidates))]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY23SlimmingHelper = SlimmingHelper("BPHY23SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import getDefaultAllVariables
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
