# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY25.py
# Contact: xin.chen@cern.ch
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY25"
streamName = "StreamDAOD_BPHY25"

def BPHY25Cfg(flags):
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
    pvrefitter = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
    acc.addPublicTool(pvrefitter)
    from TrkConfig.TrkV0FitterConfig import TrkV0VertexFitter_InDetExtrCfg
    v0fitter = acc.popToolsAndMerge(TrkV0VertexFitter_InDetExtrCfg(flags))
    acc.addPublicTool(v0fitter)
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    tracktovtxtool = acc.popToolsAndMerge(InDetTrackToVertexCfg(flags))
    acc.addPublicTool(tracktovtxtool)
    from TrkConfig.TrkVKalVrtFitterConfig import V0VKalVrtFitterCfg
    gammafitter = acc.popToolsAndMerge(V0VKalVrtFitterCfg(
        flags, BPHYDerivationName+"_GammaFitter",
        Robustness          = 6,
        usePhiCnst          = True,
        useThetaCnst        = True,
        InputParticleMasses = [0.511,0.511] ))
    acc.addPublicTool(gammafitter)
    from InDetConfig.InDetTrackSelectorToolConfig import V0InDetConversionTrackSelectorToolCfg
    v0trackselect = acc.popToolsAndMerge(V0InDetConversionTrackSelectorToolCfg(flags))
    acc.addPublicTool(v0trackselect)

    # mass limits and constants used in the following
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
    Phi_lo = 770.0
    Phi_hi = 1270.0
    Upsi_lo = 8900.0
    Upsi_hi = 9900.0
    Ds_lo = 1660.0
    Ds_hi = 2230.0
    Ks_lo = 300.0
    Ks_hi = 700.0
    Ld_lo = 900.0
    Ld_hi = 1350.0
    Xi_lo = 1100.0
    Xi_hi = 1550.0
    Omg_lo = 1450.0
    Omg_hi = 1900.0
    Xib_lo = 5350.0
    Xib_hi = 6250.0
    Sigmabp_lo = 5360.0
    Sigmabp_hi = 6260.0
    Sigmabm_lo = 5365.0
    Sigmabm_hi = 6265.0

    Mumass = 105.658
    Pimass = 139.570
    Kmass = 493.677
    Protonmass = 938.2721
    Phimass = 1019.461
    Dpmmass = 1869.66
    Dspmmass = 1968.35
    Jpsimass = 3096.916
    Psi2Smass = 3686.10
    X3872mass = 3871.65
    Zcmass = 3887.1
    Bpmmass = 5279.34
    B0mass = 5279.66
    Bs0mass = 5366.92
    Upsimass = 9460.30
    Lambdamass = 1115.683
    Lambdab0mass = 5619.60
    Ximass = 1321.71
    Omegamass = 1672.45
    Xibmass = 5797.0
    Sigmabpmass = 5810.56
    Sigmabmmass = 5815.64

    BPHY25JpsiFinder = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY25JpsiFinder",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        trackThresholdPt            = 2400.,
        invMassLower                = Phi_lo,
        invMassUpper                = Upsi_hi,
        Chi2Cut                     = 10.,
        oppChargesOnly	            = True,
        atLeastOneComb              = True,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None,
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False )
    acc.addPublicTool(BPHY25JpsiFinder)

    BPHY25_Reco_mumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY25_Reco_mumu",
        VertexSearchTool       = BPHY25JpsiFinder,
        OutputVtxContainerName = "BPHY25OniaCandidates",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = pvrefitter,
        DoVertexType           = 1)


    # X(3872), Psi(2S) -> J/psi + pi pi
    BPHY25PsiX3872_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name                                = "BPHY25PsiX3872_Jpsi2Trk",
        kaonkaonHypothesis		    = False,
        pionpionHypothesis                  = True,
        kaonpionHypothesis                  = False,
        kaonprotonHypothesis                = False,
        trkThresholdPt			    = 380.,
        trkMaxEta		 	    = 2.6,
        oppChargesOnly                      = False,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkQuadrupletMassLower              = Psi_lo,
        TrkQuadrupletMassUpper              = Psi_hi,
        Chi2Cut                             = 10.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = False)
    acc.addPublicTool(BPHY25PsiX3872_Jpsi2Trk)

    # Bs0 -> J/psi + K K
    BPHY25Bs0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name                                = "BPHY25Bs0_Jpsi2Trk",
        kaonkaonHypothesis		    = True,
        pionpionHypothesis                  = False,
        kaonpionHypothesis                  = False,
        kaonprotonHypothesis                = False,
        trkThresholdPt			    = 380.,
        trkMaxEta		 	    = 2.6,
        oppChargesOnly                      = False,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkQuadrupletMassLower              = Bs0_lo,
        TrkQuadrupletMassUpper              = Bs0_hi,
        Chi2Cut                             = 10.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = False)
    acc.addPublicTool(BPHY25Bs0_Jpsi2Trk)

    # B0 -> J/psi + K pi
    BPHY25B0_Jpsi2Trk = CompFactory.Analysis.JpsiPlus2Tracks(
        name                                = "BPHY25B0_Jpsi2Trk",
        kaonkaonHypothesis		    = False,
        pionpionHypothesis                  = False,
        kaonpionHypothesis                  = True,
        kaonprotonHypothesis                = False,
        trkThresholdPt			    = 380.,
        trkMaxEta		 	    = 2.6,
        oppChargesOnly                      = False,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkQuadrupletMassLower              = B_lo,
        TrkQuadrupletMassUpper              = B_hi,
        Chi2Cut                             = 10.,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi			    = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool		    = vkalvrt,
        TrackSelectorTool		    = trackselect,
        UseMassConstraint	            = False)
    acc.addPublicTool(BPHY25B0_Jpsi2Trk)

    # Zc(3900)+ -> J/psi pi
    BPHY25Zc3900_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25Zc3900_Jpsi1Trk",
        pionHypothesis                      = True,
        kaonHypothesis                      = False,
        trkThresholdPt                      = 380.,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkTrippletMassLower                = Zc_lo,
        TrkTrippletMassUpper                = Zc_hi,
        Chi2Cut                             = 10.0,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = False)
    acc.addPublicTool(BPHY25Zc3900_Jpsi1Trk)

    # B+ -> J/psi K
    BPHY25Bpm_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25Bpm_Jpsi1Trk",
        pionHypothesis                      = False,
        kaonHypothesis                      = True,
        trkThresholdPt                      = 380.,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Jpsi_lo,
        JpsiMassUpper                       = Jpsi_hi,
        TrkTrippletMassLower                = B_lo,
        TrkTrippletMassUpper                = B_hi,
        Chi2Cut                             = 10.0,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = False)
    acc.addPublicTool(BPHY25Bpm_Jpsi1Trk)

    # D+, Ds+ -> phi pi
    BPHY25DpmDs_Jpsi1Trk = CompFactory.Analysis.JpsiPlus1Track(
        name                                = "BPHY25DpmDs_Jpsi1Trk",
        pionHypothesis                      = True,
        kaonHypothesis                      = False,
        trkThresholdPt                      = 380.0,
        trkMaxEta                           = 2.6,
        JpsiMassLower                       = Phi_lo,
        JpsiMassUpper                       = Phi_hi,
        TrkTrippletMassLower                = Ds_lo,
        TrkTrippletMassUpper                = Ds_hi,
        Chi2Cut                             = 10.0,
        JpsiContainerKey                    = "BPHY25OniaCandidates",
        TrackParticleCollection             = "InDetTrackParticles",
        MuonsUsedInJpsi                     = "Muons",
        ExcludeJpsiMuonsOnly                = True,
        TrkVertexFitterTool                 = vkalvrt,
        TrackSelectorTool                   = trackselect,
        UseMassConstraint                   = False)
    acc.addPublicTool(BPHY25DpmDs_Jpsi1Trk)


    BPHY25FourTrackReco_PsiX3872 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25FourTrackReco_PsiX3872",
        VertexSearchTool         = BPHY25PsiX3872_Jpsi2Trk,
        OutputVtxContainerName   = "BPHY25FourTrack_PsiX3872",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)
 
    BPHY25FourTrackReco_Bs0 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25FourTrackReco_Bs0",
        VertexSearchTool         = BPHY25Bs0_Jpsi2Trk,
        OutputVtxContainerName   = "BPHY25FourTrack_Bs0",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25FourTrackReco_B0 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25FourTrackReco_B0",
        VertexSearchTool         = BPHY25B0_Jpsi2Trk,
        OutputVtxContainerName   = "BPHY25FourTrack_B0",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)


    BPHY25ThreeTrackReco_Zc3900 = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_Zc3900",
        VertexSearchTool         = BPHY25Zc3900_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25ThreeTrack_Zc3900",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25ThreeTrackReco_Bpm = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_Bpm",
        VertexSearchTool         = BPHY25Bpm_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25ThreeTrack_Bpm",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    BPHY25ThreeTrackReco_DpmDs = CompFactory.DerivationFramework.Reco_Vertex(
        name                     = "BPHY25ThreeTrackReco_DpmDs",
        VertexSearchTool         = BPHY25DpmDs_Jpsi1Trk,
        OutputVtxContainerName   = "BPHY25ThreeTrack_DpmDs",
        PVContainerName          = "PrimaryVertices",
        V0Tools                  = V0Tools,
        PVRefitter               = pvrefitter,
        RefitPV                  = False,
        DoVertexType             = 0)

    # revertex with mass constraints to reduce combinatorics
    # X(3872) -> J/psi pi pi
    BPHY25Rev_X3872 = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_X3872",
        InputVtxContainerName      = "BPHY25FourTrack_PsiX3872",
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = X3872mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass, Pimass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_X3872")

    # Bs0 -> J/psi K K
    BPHY25Rev_Bs0 = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Bs0",
        InputVtxContainerName      = "BPHY25FourTrack_Bs0",
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Bs0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass, Kmass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_Bs0")

    # B0 -> J/psi K pi
    BPHY25Rev_B0Kpi = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0Kpi",
        InputVtxContainerName      = "BPHY25FourTrack_B0",
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass, Pimass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_B0Kpi")

    # B0 -> J/psi pi K
    BPHY25Rev_B0piK = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_B0piK",
        InputVtxContainerName      = "BPHY25FourTrack_B0",
        TrackIndices               = [ 0, 1, 2, 3 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = B0mass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass, Kmass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_B0piK")

    # Zc3900 -> J/psi pi
    BPHY25Rev_Zc3900 = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Zc3900",
        InputVtxContainerName      = "BPHY25ThreeTrack_Zc3900",
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Pimass],
        Chi2Cut                    = 15.,
        BMassLower                 = Zc_lo,
        BMassUpper                 = Zc_hi,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_Zc3900")

    # Bpm -> J/psi K
    BPHY25Rev_Bpm = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Bpm",
        InputVtxContainerName      = "BPHY25ThreeTrack_Bpm",
        TrackIndices               = [ 0, 1, 2 ],
        SubVertexTrackIndices      = [ 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Bpmmass,
        SubVertexMass              = Jpsimass,
        MassInputParticles         = [Mumass, Mumass, Kmass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_Bpm")

    # Ds -> phi pi
    BPHY25Rev_Ds = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Ds",
        InputVtxContainerName      = "BPHY25ThreeTrack_DpmDs",
        TrackIndices               = [ 0, 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Dspmmass,
        MassInputParticles         = [Mumass, Mumass, Pimass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_Ds")

    # Dpm -> phi pi
    BPHY25Rev_Dpm = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY25Rev_Dpm",
        InputVtxContainerName      = "BPHY25ThreeTrack_DpmDs",
        TrackIndices               = [ 0, 1, 2 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Dpmmass,
        MassInputParticles         = [Mumass, Mumass, Pimass],
        Chi2Cut                    = 15.,
        TrkVertexFitterTool	   = vkalvrt,
        PVRefitter                 = pvrefitter,
        V0Tools                    = V0Tools,
        OutputVtxContainerName     = "BPHY25Revtx_Dpm")


    BPHY25Select_Phi               = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Phi",
        HypothesisName             = "Phi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Phi_lo,
        MassMax                    = Phi_hi,
        DoVertexType               = 0)

    BPHY25Select_Jpsi              = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Jpsi",
        HypothesisName             = "Jpsi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Jpsi_lo,
        MassMax                    = Jpsi_hi,
        DoVertexType               = 0)

    BPHY25Select_Psi               = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Psi",
        HypothesisName             = "Psi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Psi_lo,
        MassMax                    = Psi_hi,
        DoVertexType               = 0)

    BPHY25Select_Upsi              = CompFactory.DerivationFramework.Select_onia2mumu(
        name                       = "BPHY25Select_Upsi",
        HypothesisName             = "Upsi",
        InputVtxContainerName      = "BPHY25OniaCandidates",
        V0Tools                    = V0Tools,
        TrkMasses                  = [Mumass, Mumass],
        MassMin                    = Upsi_lo,
        MassMax                    = Upsi_hi,
        DoVertexType               = 0)

    ## V0 vertices
    from DerivationFrameworkBPhys.V0ToolConfig import BPHY_Reco_V0FinderCfg
    BPHY25_Reco_V0Finder = acc.popToolsAndMerge(BPHY_Reco_V0FinderCfg(
        flags, derivation = BPHYDerivationName,
        V0ContainerName = "BPHY25V0Candidates",
        KshortContainerName = "BPHY25KsCandidates_dummy",
        LambdaContainerName = "BPHY25LambdaCandidates_dummy",
        LambdabarContainerName = "BPHY25LambdabarCandidates_dummy",
        CheckVertexContainers = ["BPHY25OniaCandidates"]
    ))


    ##############################
    ### J/psi + 0 trk + Lambda ###
    ##############################

    list_0trkLd_hypo = ["PhiLd", "JpsiLd", "Psi0Ld", "UpsiLd"]
    list_0trkLd_jpsiHypo = ["Phi", "Jpsi", "Psi", "Upsi"]
    list_0trkLd_jpsiMass = [Phimass, Jpsimass, Psi2Smass, Upsimass]

    list_0trkLd_obj = []
    for hypo in list_0trkLd_hypo:
        list_0trkLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_0trkLd_obj)):
        list_0trkLd_obj[i].JXVertices               = "BPHY25OniaCandidates"
        if i == 0:
            list_0trkLd_obj[i].V0Vertices            = "BPHY25V0Candidates"
            list_0trkLd_obj[i].RefitV0               = True
            list_0trkLd_obj[i].OutoutV0VtxCollection = "LambdaCollection"
            list_0trkLd_obj[i].V0MassLowerCut        = Ld_lo
            list_0trkLd_obj[i].V0MassUpperCut        = Ld_hi
            list_0trkLd_obj[i].DoV0Enumeration       = True
            list_0trkLd_obj[i].DecorateV0Momentum    = True
        else:
            list_0trkLd_obj[i].V0Vertices            = "LambdaCollection"
            list_0trkLd_obj[i].RefitV0               = False
        list_0trkLd_obj[i].JXVtxHypoNames           = [list_0trkLd_jpsiHypo[i]]
        list_0trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkLd_hypo[i]+"_CascadeMainVtx"]
        list_0trkLd_obj[i].HasJXSubVertex           = False
        list_0trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_0trkLd_obj[i].V0Hypothesis             = "Lambda"
        list_0trkLd_obj[i].MassCutGamma             = 10.
        list_0trkLd_obj[i].Chi2CutGamma             = 5.
        list_0trkLd_obj[i].LxyV0Cut                 = 20.
        list_0trkLd_obj[i].HypothesisName           = list_0trkLd_hypo[i]
        list_0trkLd_obj[i].NumberOfJXDaughters      = 2
        list_0trkLd_obj[i].JXDaug1MassHypo          = Mumass
        list_0trkLd_obj[i].JXDaug2MassHypo          = Mumass
        list_0trkLd_obj[i].NumberOfDisVDaughters    = 2
        list_0trkLd_obj[i].JpsiMass                 = list_0trkLd_jpsiMass[i]
        list_0trkLd_obj[i].V0Mass                   = Lambdamass
        list_0trkLd_obj[i].ApplyJpsiMassConstraint  = True
        list_0trkLd_obj[i].ApplyV0MassConstraint    = True
        list_0trkLd_obj[i].Chi2CutV0                = 10.
        list_0trkLd_obj[i].Chi2Cut                  = 8.
        list_0trkLd_obj[i].Trackd0Cut               = 3.
        list_0trkLd_obj[i].MaxJXCandidates          = 10
        list_0trkLd_obj[i].MaxV0Candidates          = 10
        list_0trkLd_obj[i].RefitPV                  = True
        list_0trkLd_obj[i].MaxnPV                   = 20
        list_0trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkLd_hypo[i]+"_RefPrimaryVertices"
        list_0trkLd_obj[i].TrkVertexFitterTool      = vkalvrt
        list_0trkLd_obj[i].V0VertexFitterTool       = v0fitter
        list_0trkLd_obj[i].GammaFitterTool          = gammafitter
        list_0trkLd_obj[i].PVRefitter               = pvrefitter
        list_0trkLd_obj[i].V0Tools                  = V0Tools
        list_0trkLd_obj[i].TrackToVertexTool        = tracktovtxtool
        list_0trkLd_obj[i].V0TrackSelectorTool      = v0trackselect
        list_0trkLd_obj[i].TrackSelectorTool        = trackselect

    ##########################
    ### J/psi + 0 trk + Xi ###
    ##########################

    list_0trkXi_hypo = ["PhiXi", "JpsiXi", "UpsiXi"]
    list_0trkXi_jpsiHypo = ["Phi", "Jpsi", "Upsi"]
    list_0trkXi_jpsiMass = [Phimass, Jpsimass, Upsimass]

    list_0trkXi_obj = []
    for hypo in list_0trkXi_hypo:
        list_0trkXi_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_0trkXi_obj)):
        list_0trkXi_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_0trkXi_obj[i].V0Vertices               = "LambdaCollection"
        list_0trkXi_obj[i].RefitV0                  = False
        if i == 0:
            list_0trkXi_obj[i].OutoutDisVtxCollection = "XiCollection"
            list_0trkXi_obj[i].DisplacedMassLowerCut  = Xi_lo
            list_0trkXi_obj[i].DisplacedMassUpperCut  = Xi_hi
            list_0trkXi_obj[i].Chi2CutDisV            = 10.
        else:
            list_0trkXi_obj[i].DisplacedVertices      = "XiCollection"
        list_0trkXi_obj[i].JXVtxHypoNames           = [list_0trkXi_jpsiHypo[i]]
        list_0trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_0trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkXi_hypo[i]+"_CascadeMainVtx"]
        list_0trkXi_obj[i].HasJXSubVertex           = False
        list_0trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_0trkXi_obj[i].V0Hypothesis             = "Lambda"
        list_0trkXi_obj[i].MassCutGamma             = 10.
        list_0trkXi_obj[i].Chi2CutGamma             = 5.
        list_0trkXi_obj[i].LxyV0Cut                 = 20.
        list_0trkXi_obj[i].LxyDisVtxCut             = 20.
        list_0trkXi_obj[i].HypothesisName           = list_0trkXi_hypo[i]
        list_0trkXi_obj[i].NumberOfJXDaughters      = 2
        list_0trkXi_obj[i].JXDaug1MassHypo          = Mumass
        list_0trkXi_obj[i].JXDaug2MassHypo          = Mumass
        list_0trkXi_obj[i].NumberOfDisVDaughters    = 3
        list_0trkXi_obj[i].DisVDaug3MassHypo        = Pimass
        list_0trkXi_obj[i].JpsiMass                 = list_0trkXi_jpsiMass[i]
        list_0trkXi_obj[i].V0Mass                   = Lambdamass
        list_0trkXi_obj[i].DisVtxMass               = Ximass
        list_0trkXi_obj[i].ApplyJpsiMassConstraint  = True
        list_0trkXi_obj[i].ApplyV0MassConstraint    = True
        list_0trkXi_obj[i].ApplyDisVMassConstraint  = True
        list_0trkXi_obj[i].Chi2CutV0                = 10.
        list_0trkXi_obj[i].Chi2Cut                  = 8.
        list_0trkXi_obj[i].Trackd0Cut               = 3.
        list_0trkXi_obj[i].MaxJXCandidates          = 10
        list_0trkXi_obj[i].MaxV0Candidates          = 10
        list_0trkXi_obj[i].MaxDisVCandidates        = 20
        list_0trkXi_obj[i].RefitPV                  = True
        list_0trkXi_obj[i].MaxnPV                   = 20
        list_0trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkXi_hypo[i]+"_RefPrimaryVertices"
        list_0trkXi_obj[i].TrkVertexFitterTool      = vkalvrt
        list_0trkXi_obj[i].V0VertexFitterTool       = v0fitter
        list_0trkXi_obj[i].GammaFitterTool          = gammafitter
        list_0trkXi_obj[i].PVRefitter               = pvrefitter
        list_0trkXi_obj[i].V0Tools                  = V0Tools
        list_0trkXi_obj[i].TrackToVertexTool        = tracktovtxtool
        list_0trkXi_obj[i].V0TrackSelectorTool      = v0trackselect
        list_0trkXi_obj[i].TrackSelectorTool        = trackselect

    #############################
    ### J/psi + 0 trk + Omega ###
    #############################

    list_0trkOmg_hypo = ["PhiOmg", "JpsiOmg", "UpsiOmg"]
    list_0trkOmg_jpsiHypo = ["Phi", "Jpsi", "Upsi"]
    list_0trkOmg_jpsiMass = [Phimass, Jpsimass, Upsimass]

    list_0trkOmg_obj = []
    for hypo in list_0trkOmg_hypo:
        list_0trkOmg_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_0trkOmg_obj)):
        list_0trkOmg_obj[i].JXVertices               = "BPHY25OniaCandidates"
        list_0trkOmg_obj[i].V0Vertices               = "LambdaCollection"
        list_0trkOmg_obj[i].RefitV0                  = False
        if i == 0:
            list_0trkOmg_obj[i].OutoutDisVtxCollection = "OmegaCollection"
            list_0trkOmg_obj[i].DisplacedMassLowerCut  = Omg_lo
            list_0trkOmg_obj[i].DisplacedMassUpperCut  = Omg_hi
            list_0trkOmg_obj[i].Chi2CutDisV            = 10.
        else:
            list_0trkOmg_obj[i].DisplacedVertices      = "OmegaCollection"
        list_0trkOmg_obj[i].JXVtxHypoNames           = [list_0trkOmg_jpsiHypo[i]]
        list_0trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_0trkOmg_hypo[i]+"_CascadeMainVtx"]
        list_0trkOmg_obj[i].HasJXSubVertex           = False
        list_0trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_0trkOmg_obj[i].V0Hypothesis             = "Lambda"
        list_0trkOmg_obj[i].MassCutGamma             = 10.
        list_0trkOmg_obj[i].Chi2CutGamma             = 5.
        list_0trkOmg_obj[i].LxyV0Cut                 = 20.
        list_0trkOmg_obj[i].LxyDisVtxCut             = 20.
        list_0trkOmg_obj[i].HypothesisName           = list_0trkOmg_hypo[i]
        list_0trkOmg_obj[i].NumberOfJXDaughters      = 2
        list_0trkOmg_obj[i].JXDaug1MassHypo          = Mumass
        list_0trkOmg_obj[i].JXDaug2MassHypo          = Mumass
        list_0trkOmg_obj[i].NumberOfDisVDaughters    = 3
        list_0trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
        list_0trkOmg_obj[i].JpsiMass                 = list_0trkOmg_jpsiMass[i]
        list_0trkOmg_obj[i].V0Mass                   = Lambdamass
        list_0trkOmg_obj[i].DisVtxMass               = Omegamass
        list_0trkOmg_obj[i].ApplyJpsiMassConstraint  = True
        list_0trkOmg_obj[i].ApplyV0MassConstraint    = True
        list_0trkOmg_obj[i].ApplyDisVMassConstraint  = True
        list_0trkOmg_obj[i].Chi2CutV0                = 10.
        list_0trkOmg_obj[i].Chi2Cut                  = 8.
        list_0trkOmg_obj[i].Trackd0Cut               = 3.
        list_0trkOmg_obj[i].MaxJXCandidates          = 10
        list_0trkOmg_obj[i].MaxV0Candidates          = 10
        list_0trkOmg_obj[i].MaxDisVCandidates        = 20
        list_0trkOmg_obj[i].RefitPV                  = True
        list_0trkOmg_obj[i].MaxnPV                   = 20
        list_0trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_0trkOmg_hypo[i]+"_RefPrimaryVertices"
        list_0trkOmg_obj[i].TrkVertexFitterTool      = vkalvrt
        list_0trkOmg_obj[i].V0VertexFitterTool       = v0fitter
        list_0trkOmg_obj[i].GammaFitterTool          = gammafitter
        list_0trkOmg_obj[i].PVRefitter               = pvrefitter
        list_0trkOmg_obj[i].V0Tools                  = V0Tools
        list_0trkOmg_obj[i].TrackToVertexTool        = tracktovtxtool
        list_0trkOmg_obj[i].V0TrackSelectorTool      = v0trackselect
        list_0trkOmg_obj[i].TrackSelectorTool        = trackselect


    ##############################
    ### J/psi + 1 trk + Lambda ###
    ##############################

    list_1trkLd_hypo = ["Zc3900Ld", "BpmLd", "DsLd", "DpmLd"]
    list_1trkLd_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
    list_1trkLd_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
    list_1trkLd_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
    list_1trkLd_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

    list_1trkLd_obj = []
    for hypo in list_1trkLd_hypo:
        list_1trkLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_1trkLd_obj)):
        list_1trkLd_obj[i].JXVertices               = list_1trkLd_jxInput[i]
        list_1trkLd_obj[i].V0Vertices               = "LambdaCollection"
        list_1trkLd_obj[i].RefitV0                  = False
        if i == 0:
            list_1trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkLd_hypo[i]+"_CascadeMainVtx"]
            list_1trkLd_obj[i].HasJXSubVertex           = False
        else:
            list_1trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkLd_hypo[i]+"_CascadeMainVtx"]
            list_1trkLd_obj[i].HasJXSubVertex           = True
        list_1trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_1trkLd_obj[i].V0Hypothesis             = "Lambda"
        list_1trkLd_obj[i].MassCutGamma             = 10.
        list_1trkLd_obj[i].Chi2CutGamma             = 5.
        list_1trkLd_obj[i].LxyV0Cut                 = 20.
        list_1trkLd_obj[i].HypothesisName           = list_1trkLd_hypo[i]
        list_1trkLd_obj[i].NumberOfJXDaughters      = 3
        list_1trkLd_obj[i].JXDaug1MassHypo          = Mumass
        list_1trkLd_obj[i].JXDaug2MassHypo          = Mumass
        list_1trkLd_obj[i].JXDaug3MassHypo          = list_1trkLd_jxDau3Mass[i]
        list_1trkLd_obj[i].NumberOfDisVDaughters    = 2
        list_1trkLd_obj[i].JXMass                   = list_1trkLd_jxMass[i]
        list_1trkLd_obj[i].JpsiMass                 = list_1trkLd_jpsiMass[i]
        list_1trkLd_obj[i].V0Mass                   = Lambdamass
        list_1trkLd_obj[i].ApplyJXMassConstraint    = True
        list_1trkLd_obj[i].ApplyJpsiMassConstraint  = True
        list_1trkLd_obj[i].ApplyV0MassConstraint    = True
        list_1trkLd_obj[i].Chi2CutV0                = 10.
        list_1trkLd_obj[i].Chi2Cut                  = 8.
        list_1trkLd_obj[i].Trackd0Cut               = 3.
        list_1trkLd_obj[i].MaxJXCandidates          = 10
        list_1trkLd_obj[i].MaxV0Candidates          = 10
        list_1trkLd_obj[i].RefitPV                  = True
        list_1trkLd_obj[i].MaxnPV                   = 20
        list_1trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkLd_hypo[i]+"_RefPrimaryVertices"
        list_1trkLd_obj[i].TrkVertexFitterTool      = vkalvrt
        list_1trkLd_obj[i].V0VertexFitterTool       = v0fitter
        list_1trkLd_obj[i].GammaFitterTool          = gammafitter
        list_1trkLd_obj[i].PVRefitter               = pvrefitter
        list_1trkLd_obj[i].V0Tools                  = V0Tools
        list_1trkLd_obj[i].TrackToVertexTool        = tracktovtxtool
        list_1trkLd_obj[i].V0TrackSelectorTool      = v0trackselect
        list_1trkLd_obj[i].TrackSelectorTool        = trackselect

    ##########################
    ### J/psi + 1 trk + Xi ###
    ##########################

    list_1trkXi_hypo = ["Zc3900Xi", "BpmXi", "DsXi", "DpmXi"]
    list_1trkXi_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
    list_1trkXi_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
    list_1trkXi_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
    list_1trkXi_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

    list_1trkXi_obj = []
    for hypo in list_1trkXi_hypo:
        list_1trkXi_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_1trkXi_obj)):
        list_1trkXi_obj[i].JXVertices               = list_1trkXi_jxInput[i]
        list_1trkXi_obj[i].V0Vertices               = "LambdaCollection"
        list_1trkXi_obj[i].RefitV0                  = False
        list_1trkXi_obj[i].DisplacedVertices        = "XiCollection"
        if i == 0:
            list_1trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeMainVtx"]
            list_1trkXi_obj[i].HasJXSubVertex           = False
        else:
            list_1trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkXi_hypo[i]+"_CascadeMainVtx"]
            list_1trkXi_obj[i].HasJXSubVertex           = True
        list_1trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_1trkXi_obj[i].V0Hypothesis             = "Lambda"
        list_1trkXi_obj[i].MassCutGamma             = 10.
        list_1trkXi_obj[i].Chi2CutGamma             = 5.
        list_1trkXi_obj[i].LxyV0Cut                 = 20.
        list_1trkXi_obj[i].LxyDisVtxCut             = 20.
        list_1trkXi_obj[i].HypothesisName           = list_1trkXi_hypo[i]
        list_1trkXi_obj[i].NumberOfJXDaughters      = 3
        list_1trkXi_obj[i].JXDaug1MassHypo          = Mumass
        list_1trkXi_obj[i].JXDaug2MassHypo          = Mumass
        list_1trkXi_obj[i].JXDaug3MassHypo          = list_1trkXi_jxDau3Mass[i]
        list_1trkXi_obj[i].NumberOfDisVDaughters    = 3
        list_1trkXi_obj[i].DisVDaug3MassHypo        = Pimass
        list_1trkXi_obj[i].JXMass                   = list_1trkXi_jxMass[i]
        list_1trkXi_obj[i].JpsiMass                 = list_1trkXi_jpsiMass[i]
        list_1trkXi_obj[i].V0Mass                   = Lambdamass
        list_1trkXi_obj[i].DisVtxMass               = Ximass
        list_1trkXi_obj[i].ApplyJXMassConstraint    = True
        list_1trkXi_obj[i].ApplyJpsiMassConstraint  = True
        list_1trkXi_obj[i].ApplyV0MassConstraint    = True
        list_1trkXi_obj[i].ApplyDisVMassConstraint  = True
        list_1trkXi_obj[i].Chi2CutV0                = 10.
        list_1trkXi_obj[i].Chi2Cut                  = 8.
        list_1trkXi_obj[i].Trackd0Cut               = 3.
        list_1trkXi_obj[i].MaxJXCandidates          = 10
        list_1trkXi_obj[i].MaxV0Candidates          = 10
        list_1trkXi_obj[i].MaxDisVCandidates        = 20
        list_1trkXi_obj[i].RefitPV                  = True
        list_1trkXi_obj[i].MaxnPV                   = 20
        list_1trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkXi_hypo[i]+"_RefPrimaryVertices"
        list_1trkXi_obj[i].TrkVertexFitterTool      = vkalvrt
        list_1trkXi_obj[i].V0VertexFitterTool       = v0fitter
        list_1trkXi_obj[i].GammaFitterTool          = gammafitter
        list_1trkXi_obj[i].PVRefitter               = pvrefitter
        list_1trkXi_obj[i].V0Tools                  = V0Tools
        list_1trkXi_obj[i].TrackToVertexTool        = tracktovtxtool
        list_1trkXi_obj[i].V0TrackSelectorTool      = v0trackselect
        list_1trkXi_obj[i].TrackSelectorTool        = trackselect

    #############################
    ### J/psi + 1 trk + Omega ###
    #############################

    list_1trkOmg_hypo = ["Zc3900Omg", "BpmOmg", "DsOmg", "DpmOmg"]
    list_1trkOmg_jxInput = ["BPHY25Revtx_Zc3900", "BPHY25Revtx_Bpm", "BPHY25Revtx_Ds", "BPHY25Revtx_Dpm"]
    list_1trkOmg_jxMass = [Zcmass, Bpmmass, Dspmmass, Dpmmass]
    list_1trkOmg_jpsiMass = [Jpsimass, Jpsimass, Phimass, Phimass]
    list_1trkOmg_jxDau3Mass = [Pimass, Kmass, Pimass, Pimass]

    list_1trkOmg_obj = []
    for hypo in list_1trkOmg_hypo:
        list_1trkOmg_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_1trkOmg_obj)):
        list_1trkOmg_obj[i].JXVertices               = list_1trkOmg_jxInput[i]
        list_1trkOmg_obj[i].V0Vertices               = "LambdaCollection"
        list_1trkOmg_obj[i].RefitV0                  = False
        list_1trkOmg_obj[i].DisplacedVertices        = "OmegaCollection"
        if i == 0:
            list_1trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeMainVtx"]
            list_1trkOmg_obj[i].HasJXSubVertex           = False
        else:
            list_1trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeVtx2","BPHY25_"+list_1trkOmg_hypo[i]+"_CascadeMainVtx"]
            list_1trkOmg_obj[i].HasJXSubVertex           = True
        list_1trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_1trkOmg_obj[i].V0Hypothesis             = "Lambda"
        list_1trkOmg_obj[i].MassCutGamma             = 10.
        list_1trkOmg_obj[i].Chi2CutGamma             = 5.
        list_1trkOmg_obj[i].LxyV0Cut                 = 20.
        list_1trkOmg_obj[i].LxyDisVtxCut             = 20.
        list_1trkOmg_obj[i].HypothesisName           = list_1trkOmg_hypo[i]
        list_1trkOmg_obj[i].NumberOfJXDaughters      = 3
        list_1trkOmg_obj[i].JXDaug1MassHypo          = Mumass
        list_1trkOmg_obj[i].JXDaug2MassHypo          = Mumass
        list_1trkOmg_obj[i].JXDaug3MassHypo          = list_1trkOmg_jxDau3Mass[i]
        list_1trkOmg_obj[i].NumberOfDisVDaughters    = 3
        list_1trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
        list_1trkOmg_obj[i].JXMass                   = list_1trkOmg_jxMass[i]
        list_1trkOmg_obj[i].JpsiMass                 = list_1trkOmg_jpsiMass[i]
        list_1trkOmg_obj[i].V0Mass                   = Lambdamass
        list_1trkOmg_obj[i].DisVtxMass               = Omegamass
        list_1trkOmg_obj[i].ApplyJXMassConstraint    = True
        list_1trkOmg_obj[i].ApplyJpsiMassConstraint  = True
        list_1trkOmg_obj[i].ApplyV0MassConstraint    = True
        list_1trkOmg_obj[i].ApplyDisVMassConstraint  = True
        list_1trkOmg_obj[i].Chi2CutV0                = 10.
        list_1trkOmg_obj[i].Chi2Cut                  = 8.
        list_1trkOmg_obj[i].Trackd0Cut               = 3.
        list_1trkOmg_obj[i].MaxJXCandidates          = 10
        list_1trkOmg_obj[i].MaxV0Candidates          = 10
        list_1trkOmg_obj[i].MaxDisVCandidates        = 20
        list_1trkOmg_obj[i].RefitPV                  = True
        list_1trkOmg_obj[i].MaxnPV                   = 20
        list_1trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_1trkOmg_hypo[i]+"_RefPrimaryVertices"
        list_1trkOmg_obj[i].TrkVertexFitterTool      = vkalvrt
        list_1trkOmg_obj[i].V0VertexFitterTool       = v0fitter
        list_1trkOmg_obj[i].GammaFitterTool          = gammafitter
        list_1trkOmg_obj[i].PVRefitter               = pvrefitter
        list_1trkOmg_obj[i].V0Tools                  = V0Tools
        list_1trkOmg_obj[i].TrackToVertexTool        = tracktovtxtool
        list_1trkOmg_obj[i].V0TrackSelectorTool      = v0trackselect
        list_1trkOmg_obj[i].TrackSelectorTool        = trackselect


    ###############################
    ### J/psi + 2 trks + Lambda ###
    ###############################

    list_2trkLd_hypo = ["X3872Ld", "Bs2KLd", "B0KpiLd", "B0piKLd"]
    list_2trkLd_jxInput = ["BPHY25Revtx_X3872", "BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
    list_2trkLd_jxMass = [X3872mass, Bs0mass, B0mass, B0mass]
    list_2trkLd_jpsiMass = [Jpsimass, Jpsimass, Jpsimass, Jpsimass]
    list_2trkLd_jxDau3Mass = [Pimass, Kmass, Kmass, Pimass]
    list_2trkLd_jxDau4Mass = [Pimass, Kmass, Pimass, Kmass]

    list_2trkLd_obj = []
    for hypo in list_2trkLd_hypo:
        list_2trkLd_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_2trkLd_obj)):
        list_2trkLd_obj[i].JXVertices               = list_2trkLd_jxInput[i]
        list_2trkLd_obj[i].V0Vertices               = "LambdaCollection"
        list_2trkLd_obj[i].RefitV0                  = False
        if i == 0:
            list_2trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkLd_hypo[i]+"_CascadeMainVtx"]
            list_2trkLd_obj[i].HasJXSubVertex           = False
        else:
            list_2trkLd_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkLd_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkLd_hypo[i]+"_CascadeMainVtx"]
            list_2trkLd_obj[i].HasJXSubVertex           = True
        list_2trkLd_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2trkLd_obj[i].V0Hypothesis             = "Lambda"
        list_2trkLd_obj[i].MassCutGamma             = 10.
        list_2trkLd_obj[i].Chi2CutGamma             = 5.
        list_2trkLd_obj[i].LxyV0Cut                 = 25.
        list_2trkLd_obj[i].HypothesisName           = list_2trkLd_hypo[i]
        list_2trkLd_obj[i].NumberOfJXDaughters      = 4
        list_2trkLd_obj[i].JXDaug1MassHypo          = Mumass
        list_2trkLd_obj[i].JXDaug2MassHypo          = Mumass
        list_2trkLd_obj[i].JXDaug3MassHypo          = list_2trkLd_jxDau3Mass[i]
        list_2trkLd_obj[i].JXDaug4MassHypo          = list_2trkLd_jxDau4Mass[i]
        list_2trkLd_obj[i].NumberOfDisVDaughters    = 2
        list_2trkLd_obj[i].JXMass                   = list_2trkLd_jxMass[i]
        list_2trkLd_obj[i].JpsiMass                 = list_2trkLd_jpsiMass[i]
        list_2trkLd_obj[i].V0Mass                   = Lambdamass
        list_2trkLd_obj[i].ApplyJXMassConstraint    = True
        list_2trkLd_obj[i].ApplyJpsiMassConstraint  = True
        list_2trkLd_obj[i].ApplyV0MassConstraint    = True
        list_2trkLd_obj[i].Chi2CutV0                = 10.
        list_2trkLd_obj[i].Chi2Cut                  = 8.
        list_2trkLd_obj[i].Trackd0Cut               = 3.
        list_2trkLd_obj[i].MaxJXCandidates          = 10
        list_2trkLd_obj[i].MaxV0Candidates          = 10
        list_2trkLd_obj[i].RefitPV                  = True
        list_2trkLd_obj[i].MaxnPV                   = 20
        list_2trkLd_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkLd_hypo[i]+"_RefPrimaryVertices"
        list_2trkLd_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2trkLd_obj[i].V0VertexFitterTool       = v0fitter
        list_2trkLd_obj[i].GammaFitterTool          = gammafitter
        list_2trkLd_obj[i].PVRefitter               = pvrefitter
        list_2trkLd_obj[i].V0Tools                  = V0Tools
        list_2trkLd_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2trkLd_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2trkLd_obj[i].TrackSelectorTool        = trackselect

    ###########################
    ### J/psi + 2 trks + Xi ###
    ###########################

    list_2trkXi_hypo = ["Bs2KXi", "B0KpiXi", "B0piKXi"]
    list_2trkXi_jxInput = ["BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
    list_2trkXi_jxMass = [Bs0mass, B0mass, B0mass]
    list_2trkXi_jpsiMass = [Jpsimass, Jpsimass, Jpsimass]
    list_2trkXi_jxDau3Mass = [Kmass, Kmass, Pimass]
    list_2trkXi_jxDau4Mass = [Kmass, Pimass, Kmass]

    list_2trkXi_obj = []
    for hypo in list_2trkXi_hypo:
        list_2trkXi_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_2trkXi_obj)):
        list_2trkXi_obj[i].JXVertices               = list_2trkXi_jxInput[i]
        list_2trkXi_obj[i].V0Vertices               = "LambdaCollection"
        list_2trkXi_obj[i].RefitV0                  = False
        list_2trkXi_obj[i].DisplacedVertices        = "XiCollection"
        list_2trkXi_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkXi_hypo[i]+"_CascadeMainVtx"]
        list_2trkXi_obj[i].HasJXSubVertex           = True
        list_2trkXi_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2trkXi_obj[i].V0Hypothesis             = "Lambda"
        list_2trkXi_obj[i].MassCutGamma             = 10.
        list_2trkXi_obj[i].Chi2CutGamma             = 5.
        list_2trkXi_obj[i].LxyV0Cut                 = 20.
        list_2trkXi_obj[i].LxyDisVtxCut             = 20.
        list_2trkXi_obj[i].HypothesisName           = list_2trkXi_hypo[i]
        list_2trkXi_obj[i].NumberOfJXDaughters      = 4
        list_2trkXi_obj[i].JXDaug1MassHypo          = Mumass
        list_2trkXi_obj[i].JXDaug2MassHypo          = Mumass
        list_2trkXi_obj[i].JXDaug3MassHypo          = list_2trkXi_jxDau3Mass[i]
        list_2trkXi_obj[i].JXDaug4MassHypo          = list_2trkXi_jxDau4Mass[i]
        list_2trkXi_obj[i].NumberOfDisVDaughters    = 3
        list_2trkXi_obj[i].DisVDaug3MassHypo        = Pimass
        list_2trkXi_obj[i].JXMass                   = list_2trkXi_jxMass[i]
        list_2trkXi_obj[i].JpsiMass                 = list_2trkXi_jpsiMass[i]
        list_2trkXi_obj[i].V0Mass                   = Lambdamass
        list_2trkXi_obj[i].DisVtxMass               = Ximass
        list_2trkXi_obj[i].ApplyJXMassConstraint    = True
        list_2trkXi_obj[i].ApplyJpsiMassConstraint  = True
        list_2trkXi_obj[i].ApplyV0MassConstraint    = True
        list_2trkXi_obj[i].ApplyDisVMassConstraint  = True
        list_2trkXi_obj[i].Chi2CutV0                = 10.
        list_2trkXi_obj[i].Chi2Cut                  = 8.
        list_2trkXi_obj[i].Trackd0Cut               = 3.
        list_2trkXi_obj[i].MaxJXCandidates          = 10
        list_2trkXi_obj[i].MaxV0Candidates          = 10
        list_2trkXi_obj[i].MaxDisVCandidates        = 20
        list_2trkXi_obj[i].RefitPV                  = True
        list_2trkXi_obj[i].MaxnPV                   = 20
        list_2trkXi_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkXi_hypo[i]+"_RefPrimaryVertices"
        list_2trkXi_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2trkXi_obj[i].V0VertexFitterTool       = v0fitter
        list_2trkXi_obj[i].GammaFitterTool          = gammafitter
        list_2trkXi_obj[i].PVRefitter               = pvrefitter
        list_2trkXi_obj[i].V0Tools                  = V0Tools
        list_2trkXi_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2trkXi_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2trkXi_obj[i].TrackSelectorTool        = trackselect

    ##############################
    ### J/psi + 2 trks + Omega ###
    ##############################

    list_2trkOmg_hypo = ["Bs2KOmg", "B0KpiOmg", "B0piKOmg"]
    list_2trkOmg_jxInput = ["BPHY25Revtx_Bs0", "BPHY25Revtx_B0Kpi", "BPHY25Revtx_B0piK"]
    list_2trkOmg_jxMass = [Bs0mass, B0mass, B0mass]
    list_2trkOmg_jpsiMass = [Jpsimass, Jpsimass, Jpsimass]
    list_2trkOmg_jxDau3Mass = [Kmass, Kmass, Pimass]
    list_2trkOmg_jxDau4Mass = [Kmass, Pimass, Kmass]

    list_2trkOmg_obj = []
    for hypo in list_2trkOmg_hypo:
        list_2trkOmg_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_"+hypo) )

    for i in range(len(list_2trkOmg_obj)):
        list_2trkOmg_obj[i].JXVertices               = list_2trkOmg_jxInput[i]
        list_2trkOmg_obj[i].V0Vertices               = "LambdaCollection"
        list_2trkOmg_obj[i].RefitV0                  = False
        list_2trkOmg_obj[i].DisplacedVertices        = "OmegaCollection"
        list_2trkOmg_obj[i].CascadeVertexCollections = ["BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx1_sub","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx1","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeVtx2","BPHY25_"+list_2trkOmg_hypo[i]+"_CascadeMainVtx"]
        list_2trkOmg_obj[i].HasJXSubVertex           = True
        list_2trkOmg_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_2trkOmg_obj[i].V0Hypothesis             = "Lambda"
        list_2trkOmg_obj[i].MassCutGamma             = 10.
        list_2trkOmg_obj[i].Chi2CutGamma             = 5.
        list_2trkOmg_obj[i].LxyV0Cut                 = 20.
        list_2trkOmg_obj[i].LxyDisVtxCut             = 20.
        list_2trkOmg_obj[i].HypothesisName           = list_2trkOmg_hypo[i]
        list_2trkOmg_obj[i].NumberOfJXDaughters      = 4
        list_2trkOmg_obj[i].JXDaug1MassHypo          = Mumass
        list_2trkOmg_obj[i].JXDaug2MassHypo          = Mumass
        list_2trkOmg_obj[i].JXDaug3MassHypo          = list_2trkOmg_jxDau3Mass[i]
        list_2trkOmg_obj[i].JXDaug4MassHypo          = list_2trkOmg_jxDau4Mass[i]
        list_2trkOmg_obj[i].NumberOfDisVDaughters    = 3
        list_2trkOmg_obj[i].DisVDaug3MassHypo        = Kmass
        list_2trkOmg_obj[i].JXMass                   = list_2trkOmg_jxMass[i]
        list_2trkOmg_obj[i].JpsiMass                 = list_2trkOmg_jpsiMass[i]
        list_2trkOmg_obj[i].V0Mass                   = Lambdamass
        list_2trkOmg_obj[i].DisVtxMass               = Omegamass
        list_2trkOmg_obj[i].ApplyJXMassConstraint    = True
        list_2trkOmg_obj[i].ApplyJpsiMassConstraint  = True
        list_2trkOmg_obj[i].ApplyV0MassConstraint    = True
        list_2trkOmg_obj[i].ApplyDisVMassConstraint  = True
        list_2trkOmg_obj[i].Chi2CutV0                = 10.
        list_2trkOmg_obj[i].Chi2Cut                  = 8.
        list_2trkOmg_obj[i].Trackd0Cut               = 3.
        list_2trkOmg_obj[i].MaxJXCandidates          = 10
        list_2trkOmg_obj[i].MaxV0Candidates          = 10
        list_2trkOmg_obj[i].MaxDisVCandidates        = 20
        list_2trkOmg_obj[i].RefitPV                  = True
        list_2trkOmg_obj[i].MaxnPV                   = 20
        list_2trkOmg_obj[i].RefPVContainerName       = "BPHY25_"+list_2trkOmg_hypo[i]+"_RefPrimaryVertices"
        list_2trkOmg_obj[i].TrkVertexFitterTool      = vkalvrt
        list_2trkOmg_obj[i].V0VertexFitterTool       = v0fitter
        list_2trkOmg_obj[i].GammaFitterTool          = gammafitter
        list_2trkOmg_obj[i].PVRefitter               = pvrefitter
        list_2trkOmg_obj[i].V0Tools                  = V0Tools
        list_2trkOmg_obj[i].TrackToVertexTool        = tracktovtxtool
        list_2trkOmg_obj[i].V0TrackSelectorTool      = v0trackselect
        list_2trkOmg_obj[i].TrackSelectorTool        = trackselect


    #############################
    ## B- -> J/psi Lambda pbar ##
    #############################

    list_3body_obj = []

    list_3body_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_Bpm_3body") )
    list_3body_obj[0].JXVertices               = "BPHY25OniaCandidates"
    list_3body_obj[0].JXVtxHypoNames           = ["Jpsi"]
    list_3body_obj[0].V0Vertices               = "LambdaCollection"
    list_3body_obj[0].RefitV0                  = False
    list_3body_obj[0].CascadeVertexCollections = ["BPHY25_Bpm_3body_CascadeVtx1","BPHY25_Bpm_3body_CascadeMainVtx"]
    list_3body_obj[0].HasJXSubVertex           = False
    list_3body_obj[0].VxPrimaryCandidateName   = "PrimaryVertices"
    list_3body_obj[0].V0Hypothesis             = "Lambda"
    list_3body_obj[0].MassCutGamma             = 10.
    list_3body_obj[0].Chi2CutGamma             = 5.
    list_3body_obj[0].LxyV0Cut                 = 20.
    list_3body_obj[0].HypothesisName           = "Bpm_3body"
    list_3body_obj[0].NumberOfJXDaughters      = 2
    list_3body_obj[0].JXDaug1MassHypo          = Mumass
    list_3body_obj[0].JXDaug2MassHypo          = Mumass
    list_3body_obj[0].NumberOfDisVDaughters    = 2
    list_3body_obj[0].ExtraTrackMassHypo       = Protonmass
    list_3body_obj[0].ExtraTrackMinPt          = 500.
    list_3body_obj[0].MassLowerCut             = B_lo
    list_3body_obj[0].MassUpperCut             = B_hi
    list_3body_obj[0].JpsiMass                 = Jpsimass
    list_3body_obj[0].V0Mass                   = Lambdamass
    list_3body_obj[0].MainVtxMass              = Bpmmass
    list_3body_obj[0].ApplyJpsiMassConstraint  = True
    list_3body_obj[0].ApplyV0MassConstraint    = True
    list_3body_obj[0].ApplyMainVMassConstraint = True
    list_3body_obj[0].Chi2CutV0                = 10.
    list_3body_obj[0].Chi2Cut                  = 8.
    list_3body_obj[0].Trackd0Cut               = 3.
    list_3body_obj[0].MaxJXCandidates          = 10
    list_3body_obj[0].MaxV0Candidates          = 10
    list_3body_obj[0].MaxMainVCandidates       = 20
    list_3body_obj[0].RefitPV                  = True
    list_3body_obj[0].MaxnPV                   = 20
    list_3body_obj[0].RefPVContainerName       = "BPHY25_Bpm_3body_RefPrimaryVertices"
    list_3body_obj[0].TrkVertexFitterTool      = vkalvrt
    list_3body_obj[0].V0VertexFitterTool       = v0fitter
    list_3body_obj[0].GammaFitterTool          = gammafitter
    list_3body_obj[0].PVRefitter               = pvrefitter
    list_3body_obj[0].V0Tools                  = V0Tools
    list_3body_obj[0].TrackToVertexTool        = tracktovtxtool
    list_3body_obj[0].V0TrackSelectorTool      = v0trackselect
    list_3body_obj[0].TrackSelectorTool        = trackselect

    ##############################
    ## Xi_b- -> J/psi Lambda K- ##
    ##############################

    list_3body_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_Xib_3body") )
    list_3body_obj[1].JXVertices               = "BPHY25OniaCandidates"
    list_3body_obj[1].JXVtxHypoNames           = ["Jpsi"]
    list_3body_obj[1].V0Vertices               = "LambdaCollection"
    list_3body_obj[1].RefitV0                  = False
    list_3body_obj[1].CascadeVertexCollections = ["BPHY25_Xib_3body_CascadeVtx1","BPHY25_Xib_3body_CascadeMainVtx"]
    list_3body_obj[1].HasJXSubVertex           = False
    list_3body_obj[1].VxPrimaryCandidateName   = "PrimaryVertices"
    list_3body_obj[1].V0Hypothesis             = "Lambda"
    list_3body_obj[1].MassCutGamma             = 10.
    list_3body_obj[1].Chi2CutGamma             = 5.
    list_3body_obj[1].LxyV0Cut                 = 20.
    list_3body_obj[1].HypothesisName           = "Xib_3body"
    list_3body_obj[1].NumberOfJXDaughters      = 2
    list_3body_obj[1].JXDaug1MassHypo          = Mumass
    list_3body_obj[1].JXDaug2MassHypo          = Mumass
    list_3body_obj[1].NumberOfDisVDaughters    = 2
    list_3body_obj[1].ExtraTrackMassHypo       = Kmass
    list_3body_obj[1].ExtraTrackMinPt          = 500.
    list_3body_obj[1].MassLowerCut             = Xib_lo
    list_3body_obj[1].MassUpperCut             = Xib_hi
    list_3body_obj[1].JpsiMass                 = Jpsimass
    list_3body_obj[1].V0Mass                   = Lambdamass
    list_3body_obj[1].MainVtxMass              = Xibmass
    list_3body_obj[1].ApplyJpsiMassConstraint  = True
    list_3body_obj[1].ApplyV0MassConstraint    = True
    list_3body_obj[1].ApplyMainVMassConstraint = True
    list_3body_obj[1].Chi2CutV0                = 10.
    list_3body_obj[1].Chi2Cut                  = 8.
    list_3body_obj[1].Trackd0Cut               = 3.
    list_3body_obj[1].MaxJXCandidates          = 10
    list_3body_obj[1].MaxV0Candidates          = 10
    list_3body_obj[1].MaxMainVCandidates       = 20
    list_3body_obj[1].RefitPV                  = True
    list_3body_obj[1].MaxnPV                   = 20
    list_3body_obj[1].RefPVContainerName       = "BPHY25_Xib_3body_RefPrimaryVertices"
    list_3body_obj[1].TrkVertexFitterTool      = vkalvrt
    list_3body_obj[1].V0VertexFitterTool       = v0fitter
    list_3body_obj[1].GammaFitterTool          = gammafitter
    list_3body_obj[1].PVRefitter               = pvrefitter
    list_3body_obj[1].V0Tools                  = V0Tools
    list_3body_obj[1].TrackToVertexTool        = tracktovtxtool
    list_3body_obj[1].V0TrackSelectorTool      = v0trackselect
    list_3body_obj[1].TrackSelectorTool        = trackselect

    ##################################
    ## Sigma_b+ -> J/psi Lambda pi+ ##
    ##################################

    list_3body_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_Sigmabp_3body") )
    list_3body_obj[2].JXVertices               = "BPHY25OniaCandidates"
    list_3body_obj[2].JXVtxHypoNames           = ["Jpsi"]
    list_3body_obj[2].V0Vertices               = "LambdaCollection"
    list_3body_obj[2].RefitV0                  = False
    list_3body_obj[2].CascadeVertexCollections = ["BPHY25_Sigmabp_3body_CascadeVtx1","BPHY25_Sigmabp_3body_CascadeMainVtx"]
    list_3body_obj[2].HasJXSubVertex           = False
    list_3body_obj[2].VxPrimaryCandidateName   = "PrimaryVertices"
    list_3body_obj[2].V0Hypothesis             = "Lambda"
    list_3body_obj[2].MassCutGamma             = 10.
    list_3body_obj[2].Chi2CutGamma             = 5.
    list_3body_obj[2].LxyV0Cut                 = 20.
    list_3body_obj[2].HypothesisName           = "Sigmabp_3body"
    list_3body_obj[2].NumberOfJXDaughters      = 2
    list_3body_obj[2].JXDaug1MassHypo          = Mumass
    list_3body_obj[2].JXDaug2MassHypo          = Mumass
    list_3body_obj[2].NumberOfDisVDaughters    = 2
    list_3body_obj[2].ExtraTrackMassHypo       = Pimass
    list_3body_obj[2].ExtraTrackMinPt          = 500.
    list_3body_obj[2].MassLowerCut             = Sigmabp_lo
    list_3body_obj[2].MassUpperCut             = Sigmabp_hi
    list_3body_obj[2].JpsiMass                 = Jpsimass
    list_3body_obj[2].V0Mass                   = Lambdamass
    list_3body_obj[2].MainVtxMass              = Sigmabpmass
    list_3body_obj[2].ApplyJpsiMassConstraint  = True
    list_3body_obj[2].ApplyV0MassConstraint    = True
    list_3body_obj[2].ApplyMainVMassConstraint = True
    list_3body_obj[2].Chi2CutV0                = 10.
    list_3body_obj[2].Chi2Cut                  = 8.
    list_3body_obj[2].Trackd0Cut               = 3.
    list_3body_obj[2].MaxJXCandidates          = 10
    list_3body_obj[2].MaxV0Candidates          = 10
    list_3body_obj[2].MaxMainVCandidates       = 20
    list_3body_obj[2].RefitPV                  = True
    list_3body_obj[2].MaxnPV                   = 20
    list_3body_obj[2].RefPVContainerName       = "BPHY25_Sigmabp_3body_RefPrimaryVertices"
    list_3body_obj[2].TrkVertexFitterTool      = vkalvrt
    list_3body_obj[2].V0VertexFitterTool       = v0fitter
    list_3body_obj[2].GammaFitterTool          = gammafitter
    list_3body_obj[2].PVRefitter               = pvrefitter
    list_3body_obj[2].V0Tools                  = V0Tools
    list_3body_obj[2].TrackToVertexTool        = tracktovtxtool
    list_3body_obj[2].V0TrackSelectorTool      = v0trackselect
    list_3body_obj[2].TrackSelectorTool        = trackselect

    ##################################
    ## Sigma_b- -> J/psi Lambda pi- ##
    ##################################

    list_3body_obj.append( CompFactory.DerivationFramework.JpsiXPlusDisplaced("BPHY25_Sigmabm_3body") )
    list_3body_obj[3].JXVertices               = "BPHY25OniaCandidates"
    list_3body_obj[3].JXVtxHypoNames           = ["Jpsi"]
    list_3body_obj[3].V0Vertices               = "LambdaCollection"
    list_3body_obj[3].RefitV0                  = False
    list_3body_obj[3].CascadeVertexCollections = ["BPHY25_Sigmabm_3body_CascadeVtx1","BPHY25_Sigmabm_3body_CascadeMainVtx"]
    list_3body_obj[3].HasJXSubVertex           = False
    list_3body_obj[3].VxPrimaryCandidateName   = "PrimaryVertices"
    list_3body_obj[3].V0Hypothesis             = "Lambda"
    list_3body_obj[3].MassCutGamma             = 10.
    list_3body_obj[3].Chi2CutGamma             = 5.
    list_3body_obj[3].LxyV0Cut                 = 20.
    list_3body_obj[3].HypothesisName           = "Sigmabm_3body"
    list_3body_obj[3].NumberOfJXDaughters      = 2
    list_3body_obj[3].JXDaug1MassHypo          = Mumass
    list_3body_obj[3].JXDaug2MassHypo          = Mumass
    list_3body_obj[3].NumberOfDisVDaughters    = 2
    list_3body_obj[3].ExtraTrackMassHypo       = Pimass
    list_3body_obj[3].ExtraTrackMinPt          = 500.
    list_3body_obj[3].MassLowerCut             = Sigmabm_lo
    list_3body_obj[3].MassUpperCut             = Sigmabm_hi
    list_3body_obj[3].JpsiMass                 = Jpsimass
    list_3body_obj[3].V0Mass                   = Lambdamass
    list_3body_obj[3].MainVtxMass              = Sigmabmmass
    list_3body_obj[3].ApplyJpsiMassConstraint  = True
    list_3body_obj[3].ApplyV0MassConstraint    = True
    list_3body_obj[3].ApplyMainVMassConstraint = True
    list_3body_obj[3].Chi2CutV0                = 10.
    list_3body_obj[3].Chi2Cut                  = 8.
    list_3body_obj[3].Trackd0Cut               = 3.
    list_3body_obj[3].MaxJXCandidates          = 10
    list_3body_obj[3].MaxV0Candidates          = 10
    list_3body_obj[3].MaxMainVCandidates       = 20
    list_3body_obj[3].RefitPV                  = True
    list_3body_obj[3].MaxnPV                   = 20
    list_3body_obj[3].RefPVContainerName       = "BPHY25_Sigmabm_3body_RefPrimaryVertices"
    list_3body_obj[3].TrkVertexFitterTool      = vkalvrt
    list_3body_obj[3].V0VertexFitterTool       = v0fitter
    list_3body_obj[3].GammaFitterTool          = gammafitter
    list_3body_obj[3].PVRefitter               = pvrefitter
    list_3body_obj[3].V0Tools                  = V0Tools
    list_3body_obj[3].TrackToVertexTool        = tracktovtxtool
    list_3body_obj[3].V0TrackSelectorTool      = v0trackselect
    list_3body_obj[3].TrackSelectorTool        = trackselect

    list_1V0_obj = list_0trkLd_obj + list_0trkXi_obj + list_0trkOmg_obj + list_1trkLd_obj + list_1trkXi_obj + list_1trkOmg_obj + list_2trkLd_obj + list_2trkXi_obj + list_2trkOmg_obj + list_3body_obj


    ###################################
    ## Bs0 -> J/psi Lambda Lambdabar ##
    ###################################

    list_2V0_obj = []

    list_2V0_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_Bs0_2V0") )
    list_2V0_obj[0].JXVertices               = "BPHY25OniaCandidates"
    list_2V0_obj[0].JXVtxHypoNames           = ["Jpsi"]
    list_2V0_obj[0].V0Containers             = ["LambdaCollection"]
    list_2V0_obj[0].RefitV0                  = False
    list_2V0_obj[0].CascadeVertexCollections = ["BPHY25_Bs0_2V0_CascadeVtx1","BPHY25_Bs0_2V0_CascadeVtx2","BPHY25_Bs0_2V0_CascadeMainVtx"]
    list_2V0_obj[0].HasJXSubVertex           = False
    list_2V0_obj[0].HasJXV02SubVertex        = False
    list_2V0_obj[0].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2V0_obj[0].V01Hypothesis            = "Lambda"
    list_2V0_obj[0].LxyV01Cut                = 20.
    list_2V0_obj[0].V02Hypothesis            = "Lambda"
    list_2V0_obj[0].LxyV02Cut                = 20.
    list_2V0_obj[0].MassCutGamma             = 10.
    list_2V0_obj[0].Chi2CutGamma             = 5.
    list_2V0_obj[0].HypothesisName           = "Bs0_2V0"
    list_2V0_obj[0].NumberOfJXDaughters      = 2
    list_2V0_obj[0].JXDaug1MassHypo          = Mumass
    list_2V0_obj[0].JXDaug2MassHypo          = Mumass
    list_2V0_obj[0].MassLowerCut             = Bs0_lo
    list_2V0_obj[0].MassUpperCut             = Bs0_hi
    list_2V0_obj[0].JpsiMass                 = Jpsimass
    list_2V0_obj[0].MainVtxMass              = Bs0mass
    list_2V0_obj[0].ApplyJpsiMassConstraint  = True
    list_2V0_obj[0].ApplyV01MassConstraint   = True
    list_2V0_obj[0].ApplyV02MassConstraint   = True
    list_2V0_obj[0].ApplyMainVMassConstraint = True
    list_2V0_obj[0].Chi2CutV0                = 10.
    list_2V0_obj[0].Chi2Cut                  = 8.
    list_2V0_obj[0].MaxJXCandidates          = 10
    list_2V0_obj[0].MaxV0Candidates          = 10
    list_2V0_obj[0].MaxMainVCandidates       = 20
    list_2V0_obj[0].RefitPV                  = True
    list_2V0_obj[0].MaxnPV                   = 20
    list_2V0_obj[0].RefPVContainerName       = "BPHY25_Bs0_2V0_RefPrimaryVertices"
    list_2V0_obj[0].TrkVertexFitterTool      = vkalvrt
    list_2V0_obj[0].V0VertexFitterTool       = v0fitter
    list_2V0_obj[0].GammaFitterTool          = gammafitter
    list_2V0_obj[0].PVRefitter               = pvrefitter
    list_2V0_obj[0].V0Tools                  = V0Tools

    ##############################################################
    ## Xi_bc^0 -> Lambda_b^0 + Ks, Lambda_b^0 -> J/psi + Lambda ##
    ##############################################################

    list_2V0_obj.append( CompFactory.DerivationFramework.JpsiXPlus2V0("BPHY25_Xibc_2V0") )
    list_2V0_obj[1].JXVertices               = "BPHY25OniaCandidates"
    list_2V0_obj[1].JXVtxHypoNames           = ["Jpsi"]
    list_2V0_obj[1].V0Containers             = ["BPHY25V0Candidates"]
    list_2V0_obj[1].RefitV0                  = True
    list_2V0_obj[1].ApplyV0MassConstraint    = True
    list_2V0_obj[1].DoV0Enumeration          = True
    list_2V0_obj[1].DecorateV0Momentum       = True
    list_2V0_obj[1].CascadeVertexCollections = ["BPHY25_Xibc_2V0_CascadeVtx1","BPHY25_Xibc_2V0_CascadeVtx2","BPHY25_Xibc_2V0_CascadeVtx3","BPHY25_Xibc_2V0_CascadeMainVtx"]
    list_2V0_obj[1].HasJXSubVertex           = True
    list_2V0_obj[1].HasJXV02SubVertex        = True
    list_2V0_obj[1].VxPrimaryCandidateName   = "PrimaryVertices"
    list_2V0_obj[1].V01Hypothesis            = "Ks"
    list_2V0_obj[1].V01MassLowerCut          = Ks_lo
    list_2V0_obj[1].V01MassUpperCut          = Ks_hi
    list_2V0_obj[1].LxyV01Cut                = 20.
    list_2V0_obj[1].V02Hypothesis            = "Lambda"
    list_2V0_obj[1].V02MassLowerCut          = Ld_lo
    list_2V0_obj[1].V02MassUpperCut          = Ld_hi
    list_2V0_obj[1].LxyV02Cut                = 20.
    list_2V0_obj[1].MassCutGamma             = 10.
    list_2V0_obj[1].Chi2CutGamma             = 5.
    list_2V0_obj[1].HypothesisName           = "Xibc_2V0"
    list_2V0_obj[1].NumberOfJXDaughters      = 2
    list_2V0_obj[1].JXDaug1MassHypo          = Mumass
    list_2V0_obj[1].JXDaug2MassHypo          = Mumass
    list_2V0_obj[1].JpsiMass                 = Jpsimass
    list_2V0_obj[1].JXV02VtxMass             = Lambdab0mass
    list_2V0_obj[1].ApplyJpsiMassConstraint  = True
    list_2V0_obj[1].ApplyV01MassConstraint   = True
    list_2V0_obj[1].ApplyV02MassConstraint   = True
    list_2V0_obj[1].ApplyJXV02MassConstraint = True
    list_2V0_obj[1].Chi2CutV0                = 10.
    list_2V0_obj[1].Chi2Cut                  = 8.
    list_2V0_obj[1].MaxJXCandidates          = 10
    list_2V0_obj[1].MaxV0Candidates          = 10
    list_2V0_obj[1].MaxMainVCandidates       = 20
    list_2V0_obj[1].RefitPV                  = True
    list_2V0_obj[1].MaxnPV                   = 20
    list_2V0_obj[1].RefPVContainerName       = "BPHY25_Xibc_2V0_RefPrimaryVertices"
    list_2V0_obj[1].TrkVertexFitterTool      = vkalvrt
    list_2V0_obj[1].V0VertexFitterTool       = v0fitter
    list_2V0_obj[1].GammaFitterTool          = gammafitter
    list_2V0_obj[1].PVRefitter               = pvrefitter
    list_2V0_obj[1].V0Tools                  = V0Tools


    CascadeCollections = []
    RefPVContainers = []
    RefPVAuxContainers = []
    passedCandidates = []

    for obj in list_1V0_obj:
        CascadeCollections += obj.CascadeVertexCollections
        RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    for obj in list_2V0_obj:
        CascadeCollections += obj.CascadeVertexCollections
        RefPVContainers += ["xAOD::VertexContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY25_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY25_" + obj.HypothesisName + "_CascadeMainVtx"]

    BPHY25_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY25_SelectEvent", VertexContainerNames = passedCandidates)

    augmentation_tools = [BPHY25_Reco_mumu, BPHY25FourTrackReco_PsiX3872, BPHY25FourTrackReco_Bs0, BPHY25FourTrackReco_B0, BPHY25ThreeTrackReco_Zc3900, BPHY25ThreeTrackReco_Bpm, BPHY25ThreeTrackReco_DpmDs, BPHY25Rev_X3872, BPHY25Rev_Bs0, BPHY25Rev_B0Kpi, BPHY25Rev_B0piK, BPHY25Rev_Zc3900, BPHY25Rev_Bpm, BPHY25Rev_Ds, BPHY25Rev_Dpm, BPHY25Select_Phi, BPHY25Select_Jpsi, BPHY25Select_Psi, BPHY25Select_Upsi, BPHY25_Reco_V0Finder] + list_1V0_obj + list_2V0_obj
    for t in augmentation_tools : acc.addPublicTool(t)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY25Kernel",
        AugmentationTools = augmentation_tools,
        SkimmingTools     = [BPHY25_SelectEvent]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY25SlimmingHelper = SlimmingHelper("BPHY25SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    from DerivationFrameworkBPhys.commonBPHYMethodsCfg import getDefaultAllVariables
    BPHY25_AllVariables  = getDefaultAllVariables()
    BPHY25_StaticContent = []

    # Needed for trigger objects
    BPHY25SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY25SlimmingHelper.IncludeBPhysTriggerContent = True

    ## primary vertices
    BPHY25_AllVariables += ["PrimaryVertices"]
    BPHY25_StaticContent += RefPVContainers
    BPHY25_StaticContent += RefPVAuxContainers

    ## ID track particles
    BPHY25_AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are stored in InDetTrackParticles collection)
    BPHY25_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    ## muon container
    BPHY25_AllVariables += ["Muons", "MuonSegments"]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for cascade in CascadeCollections:
        BPHY25_StaticContent += ["xAOD::VertexContainer#%s" % cascade]
        BPHY25_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % cascade]

    # Truth information for MC only
    if isSimulation:
        BPHY25_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY25SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
    BPHY25SlimmingHelper.AllVariables = BPHY25_AllVariables
    BPHY25SlimmingHelper.StaticContent = BPHY25_StaticContent

    BPHY25ItemList = BPHY25SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY25", ItemList=BPHY25ItemList, AcceptAlgs=["BPHY25Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY25", AcceptAlgs=["BPHY25Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True, printComponentsOnly=False)
    return acc
