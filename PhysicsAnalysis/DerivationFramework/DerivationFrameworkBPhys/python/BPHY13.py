# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#====================================================================
# BPHY13.py for 4-muon resonance search
# Contact: xin.chen@cern.ch
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
    from AthenaServices.PartPropSvcConfig import PartPropSvcCfg
    acc.getPrimaryAndMerge(PartPropSvcCfg(flags))
    isSimulation = flags.Input.isMC
    V0Tools = acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName))
    vkalvrt = acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName)) # VKalVrt vertex fitter
    acc.addPublicTool(vkalvrt)
    acc.addPublicTool(V0Tools)
    trackselect = acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName))
    acc.addPublicTool(trackselect)
    vpest = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName))
    acc.addPublicTool(vpest)

    # mass bounds and constants used in the following
    Phi_lo = 770.0
    Phi_hi = 1300.0
    Jpsi_lo = 2600.0
    Jpsi_hi = 3600.0
    Psi_lo = 3300.0
    Psi_hi = 4200.0
    Upsi_lo = 8800.0
    Upsi_hi = 10000.0
    Dimu_lo = 5000.0
    Dimu_hi = 9100.0

    Mumass = 105.658
    Phimass = 1019.461
    Jpsimass = 3096.916
    Psi2Smass = 3686.10
    Upsimass = 9460.30

    BPHY13PhiFinder_mumu = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13PhiFinder_mumu",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Phi_lo,
        invMassUpper                = Phi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
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
    acc.addPublicTool(BPHY13PhiFinder_mumu)

    BPHY13JpsiFinder_mumu = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13JpsiFinder_mumu",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Jpsi_lo,
        invMassUpper                = Jpsi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
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
    acc.addPublicTool(BPHY13JpsiFinder_mumu)

    BPHY13PsiFinder_mumu = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13PsiFinder_mumu",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Psi_lo,
        invMassUpper                = Psi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
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
    acc.addPublicTool(BPHY13PsiFinder_mumu)

    BPHY13UpsiFinder_mumu = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13UpsiFinder_mumu",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Upsi_lo,
        invMassUpper                = Upsi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
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
    acc.addPublicTool(BPHY13UpsiFinder_mumu)

    BPHY13DimuFinder_mumu = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13DimuFinder_mumu",
        muAndMu                     = True,
        muAndTrack                  = False,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Dimu_lo,
        invMassUpper                = Dimu_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
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
    acc.addPublicTool(BPHY13DimuFinder_mumu)


    BPHY13PhiFinder_mutrk = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13PhiFinder_mutrk",
        muAndMu                     = False,
        muAndTrack                  = True,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Phi_lo,
        invMassUpper                = Phi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False,
        doTagAndProbe               = True)
    acc.addPublicTool(BPHY13PhiFinder_mutrk)

    BPHY13JpsiFinder_mutrk = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13JpsiFinder_mutrk",
        muAndMu                     = False,
        muAndTrack                  = True,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Jpsi_lo,
        invMassUpper                = Jpsi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False,
        doTagAndProbe               = True )
    acc.addPublicTool(BPHY13JpsiFinder_mutrk)

    BPHY13PsiFinder_mutrk = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13PsiFinder_mutrk",
        muAndMu                     = False,
        muAndTrack                  = True,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Psi_lo,
        invMassUpper                = Psi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False,
        doTagAndProbe               = True )
    acc.addPublicTool(BPHY13PsiFinder_mutrk)

    BPHY13UpsiFinder_mutrk = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13UpsiFinder_mutrk",
        muAndMu                     = False,
        muAndTrack                  = True,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Upsi_lo,
        invMassUpper                = Upsi_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False,
        doTagAndProbe               = True )
    acc.addPublicTool(BPHY13UpsiFinder_mutrk)

    BPHY13DimuFinder_mutrk = CompFactory.Analysis.JpsiFinder(
        name                        = "BPHY13DimuFinder_mutrk",
        muAndMu                     = False,
        muAndTrack                  = True,
        TrackAndTrack               = False,
        assumeDiMuons               = True,  # If true, will assume dimu hypothesis and use PDG value for mu mass
        muonThresholdPt             = 2400.,
        trackThresholdPt            = 2400.,
        invMassLower                = Dimu_lo,
        invMassUpper                = Dimu_hi,
        Chi2Cut                     = 50.,
        oppChargesOnly	            = True,
        atLeastOneComb              = False,
        useCombinedMeasurement      = False, # Only takes effect if combOnly=True	
        muonCollectionKey           = "Muons",
        TrackParticleCollection     = "InDetTrackParticles",
        V0VertexFitterTool          = None, # V0 vertex fitter
        useV0Fitter                 = False, # if False a TrkVertexFitterTool will be used
        TrkVertexFitterTool         = vkalvrt, # VKalVrt vertex fitter
        TrackSelectorTool           = trackselect,
        VertexPointEstimator        = vpest,
        useMCPCuts                  = False,
        doTagAndProbe               = True )
    acc.addPublicTool(BPHY13DimuFinder_mutrk)


    BPHY13_Reco_Phimumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Phimumu",
        VertexSearchTool       = BPHY13PhiFinder_mumu,
        OutputVtxContainerName = "BPHY13Phimumu",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Jpsimumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Jpsimumu",
        VertexSearchTool       = BPHY13JpsiFinder_mumu,
        OutputVtxContainerName = "BPHY13Jpsimumu",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Psimumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Psimumu",
        VertexSearchTool       = BPHY13PsiFinder_mumu,
        OutputVtxContainerName = "BPHY13Psimumu",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Upsimumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Upsimumu",
        VertexSearchTool       = BPHY13UpsiFinder_mumu,
        OutputVtxContainerName = "BPHY13Upsimumu",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Dimumumu = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Dimumumu",
        VertexSearchTool       = BPHY13DimuFinder_mumu,
        OutputVtxContainerName = "BPHY13Dimumumu",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)


    BPHY13_Reco_Phimutrk = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Phimutrk",
        VertexSearchTool       = BPHY13PhiFinder_mutrk,
        OutputVtxContainerName = "BPHY13Phimutrk",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Jpsimutrk = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Jpsimutrk",
        VertexSearchTool       = BPHY13JpsiFinder_mutrk,
        OutputVtxContainerName = "BPHY13Jpsimutrk",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Psimutrk = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Psimutrk",
        VertexSearchTool       = BPHY13PsiFinder_mutrk,
        OutputVtxContainerName = "BPHY13Psimutrk",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Upsimutrk = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Upsimutrk",
        VertexSearchTool       = BPHY13UpsiFinder_mutrk,
        OutputVtxContainerName = "BPHY13Upsimutrk",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)

    BPHY13_Reco_Dimumutrk = CompFactory.DerivationFramework.Reco_Vertex(
        name                   = "BPHY13_Reco_Dimumutrk",
        VertexSearchTool       = BPHY13DimuFinder_mutrk,
        OutputVtxContainerName = "BPHY13Dimumutrk",
        PVContainerName        = "PrimaryVertices",
        RefPVContainerName     = "SHOULDNOTBEUSED",
        V0Tools                = V0Tools,
        PVRefitter             = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        DoVertexType           = 1)


    BPHY13_Rev_Phimumu = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Phimumu",
        InputVtxContainerName      = "BPHY13Phimumu",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Phimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Phimumu_revtx")

    BPHY13_Rev_Jpsimumu = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Jpsimumu",
        InputVtxContainerName      = "BPHY13Jpsimumu",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Jpsimumu_revtx")

    BPHY13_Rev_Psimumu = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Psimumu",
        InputVtxContainerName      = "BPHY13Psimumu",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Psi2Smass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Psimumu_revtx")

    BPHY13_Rev_Upsimumu = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Upsimumu",
        InputVtxContainerName      = "BPHY13Upsimumu",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Upsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Upsimumu_revtx")


    BPHY13_Rev_Phimutrk = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Phimutrk",
        InputVtxContainerName      = "BPHY13Phimutrk",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Phimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Phimutrk_revtx")

    BPHY13_Rev_Jpsimutrk = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Jpsimutrk",
        InputVtxContainerName      = "BPHY13Jpsimutrk",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Jpsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Jpsimutrk_revtx")

    BPHY13_Rev_Psimutrk = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Psimutrk",
        InputVtxContainerName      = "BPHY13Psimutrk",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Psi2Smass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Psimutrk_revtx")

    BPHY13_Rev_Upsimutrk = CompFactory.DerivationFramework.ReVertex(
        name                       = "BPHY13_Rev_Upsimutrk",
        InputVtxContainerName      = "BPHY13Upsimutrk",
        TrackIndices               = [ 0, 1 ],
        RefitPV                    = False,
        UseMassConstraint          = True,
        VertexMass                 = Upsimass,
        MassInputParticles         = [Mumass, Mumass],
        Chi2Cut                    = 50.,
        PVRefitter                 = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)),
        V0Tools                    = V0Tools,
        TrkVertexFitterTool	   = vkalvrt,
        OutputVtxContainerName     = "BPHY13Upsimutrk_revtx")


    list_4mu_hypo = ["UpsiUpsi_4mu", "UpsiPsi_4mu", "UpsiJpsi_4mu", "UpsiPhi_4mu",
                     "PsiPsi_4mu", "PsiJpsi_4mu", "PsiPhi_4mu",
                     "JpsiJpsi_4mu", "JpsiPhi_4mu", "PhiPhi_4mu",
                     "UpsiDimu_4mu"]
    list_4mu_psi1Input = ["BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx",
                          "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx",
                          "BPHY13Jpsimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Upsimumu_revtx"]
    list_4mu_psi2Input = ["BPHY13Upsimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Dimumumu"]
    list_4mu_jpsi1lo = [Upsi_lo,Upsi_lo,Upsi_lo,Upsi_lo,
                        Psi_lo,Psi_lo,Psi_lo,
                        Jpsi_lo,Jpsi_lo,Phi_lo,
                        Upsi_lo]
    list_4mu_jpsi1hi = [Upsi_hi,Upsi_hi,Upsi_hi,Upsi_hi,
                        Psi_hi,Psi_hi,Psi_hi,
                        Jpsi_hi,Jpsi_hi,Phi_hi,
                        Upsi_hi]
    list_4mu_jpsi1mass = [Upsimass, Upsimass, Upsimass, Upsimass,
                          Psi2Smass, Psi2Smass, Psi2Smass,
                          Jpsimass, Jpsimass, Phimass,
                          Upsimass]
    list_4mu_jpsi2lo = [Upsi_lo,Psi_lo,Jpsi_lo,Phi_lo,
                        Psi_lo,Jpsi_lo,Phi_lo,
                        Jpsi_lo,Phi_lo,Phi_lo,
                        Dimu_lo]
    list_4mu_jpsi2hi = [Upsi_hi,Psi_hi,Jpsi_hi,Phi_hi,
                        Psi_hi,Jpsi_hi,Phi_hi,
                        Jpsi_hi,Phi_hi,Phi_hi,
                        Dimu_hi]
    list_4mu_jpsi2mass = [Upsimass, Psi2Smass, Jpsimass, Phimass,
                          Psi2Smass, Jpsimass, Phimass,
                          Jpsimass, Phimass, Phimass]

    list_4mu_obj = []
    for hypo in list_4mu_hypo:
        list_4mu_obj.append( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY13_"+hypo) )

    for i in range(len(list_4mu_obj)):
        list_4mu_obj[i].HypothesisName           = list_4mu_hypo[i]
        list_4mu_obj[i].Psi1Vertices             = list_4mu_psi1Input[i]
        list_4mu_obj[i].Psi2Vertices             = list_4mu_psi2Input[i]
        list_4mu_obj[i].NumberOfPsi1Daughters    = 2
        list_4mu_obj[i].NumberOfPsi2Daughters    = 2
        list_4mu_obj[i].Jpsi1MassLowerCut        = list_4mu_jpsi1lo[i]
        list_4mu_obj[i].Jpsi1MassUpperCut        = list_4mu_jpsi1hi[i]
        list_4mu_obj[i].Jpsi2MassLowerCut        = list_4mu_jpsi2lo[i]
        list_4mu_obj[i].Jpsi2MassUpperCut        = list_4mu_jpsi2hi[i]
        list_4mu_obj[i].MassLowerCut             = 0.
        list_4mu_obj[i].MassUpperCut             = 31000.
        list_4mu_obj[i].Jpsi1Mass                = list_4mu_jpsi1mass[i]
        list_4mu_obj[i].ApplyJpsi1MassConstraint = True
        if i == len(list_4mu_obj)-1:
            list_4mu_obj[i].ApplyJpsi2MassConstraint = False
        else:
            list_4mu_obj[i].Jpsi2Mass                = list_4mu_jpsi2mass[i]
            list_4mu_obj[i].ApplyJpsi2MassConstraint = True
        list_4mu_obj[i].Chi2Cut                  = 25.
        list_4mu_obj[i].PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
        list_4mu_obj[i].TrkVertexFitterTool      = vkalvrt
        list_4mu_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_4mu_obj[i].OutputVertexCollections  = ["BPHY13_"+list_4mu_hypo[i]+"_SubVtx1","BPHY13_"+list_4mu_hypo[i]+"_SubVtx2","BPHY13_"+list_4mu_hypo[i]+"_MainVtx"]
        list_4mu_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_4mu_obj[i].RefPVContainerName       = "BPHY13_"+list_4mu_hypo[i]+"_RefPrimaryVertices"
        list_4mu_obj[i].RefitPV                  = True
        list_4mu_obj[i].MaxnPV                   = 100


    list_3mu1trk_hypo = ["UpsiUpsi_3mu1trk", "UpsiPsi_3mu1trk", "UpsiJpsi_3mu1trk", "UpsiPhi_3mu1trk",
                         "PsiPsi_3mu1trk", "PsiJpsi_3mu1trk", "PsiPhi_3mu1trk",
                         "JpsiJpsi_3mu1trk", "JpsiPhi_3mu1trk", "PhiPhi_3mu1trk",
                         "PsiUpsi_3mu1trk", "JpsiUpsi_3mu1trk", "PhiUpsi_3mu1trk",
                         "JpsiPsi_3mu1trk", "PhiPsi_3mu1trk", "PhiJpsi_3mu1trk",
                         "UpsiDimu_3mu1trk", "DimuUpsi_3mu1trk"]
    list_3mu1trk_psi1Input = ["BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx",
                              "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx",
                              "BPHY13Jpsimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Upsimumu_revtx", "BPHY13Dimumumu"]
    list_3mu1trk_psi2Input = ["BPHY13Upsimutrk_revtx", "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Upsimutrk_revtx", "BPHY13Upsimutrk_revtx", "BPHY13Upsimutrk_revtx",
                              "BPHY13Psimutrk_revtx", "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx",
                              "BPHY13Dimumutrk", "BPHY13Upsimutrk_revtx"]
    list_3mu1trk_jpsi1lo = [Upsi_lo,Upsi_lo,Upsi_lo,Upsi_lo,
                            Psi_lo,Psi_lo,Psi_lo,
                            Jpsi_lo,Jpsi_lo,Phi_lo,
                            Psi_lo,Jpsi_lo,Phi_lo,
                            Jpsi_lo,Phi_lo,Phi_lo,
                            Upsi_lo, Dimu_lo]
    list_3mu1trk_jpsi1hi = [Upsi_hi,Upsi_hi,Upsi_hi,Upsi_hi,
                            Psi_hi,Psi_hi,Psi_hi,
                            Jpsi_hi,Jpsi_hi,Phi_hi,
                            Psi_hi,Jpsi_hi,Phi_hi,
                            Jpsi_hi,Phi_hi,Phi_hi,
                            Upsi_hi, Dimu_hi]
    list_3mu1trk_jpsi1mass = [Upsimass, Upsimass, Upsimass, Upsimass,
                              Psi2Smass, Psi2Smass, Psi2Smass,
                              Jpsimass, Jpsimass, Phimass,
                              Psi2Smass, Jpsimass, Phimass,
                              Jpsimass, Phimass, Phimass,
                              Upsimass, 0.0] # 0.0 is dummy
    list_3mu1trk_jpsi2lo = [Upsi_lo,Psi_lo,Jpsi_lo,Phi_lo,
                            Psi_lo,Jpsi_lo,Phi_lo,
                            Jpsi_lo,Phi_lo,Phi_lo,
                            Upsi_lo,Upsi_lo,Upsi_lo,
                            Psi_lo,Psi_lo,Jpsi_lo,
                            Dimu_lo, Upsi_lo]
    list_3mu1trk_jpsi2hi = [Upsi_hi,Psi_hi,Jpsi_hi,Phi_hi,
                            Psi_hi,Jpsi_hi,Phi_hi,
                            Jpsi_hi,Phi_hi,Phi_hi,
                            Upsi_hi,Upsi_hi,Upsi_hi,
                            Psi_hi,Psi_hi,Jpsi_hi,
                            Dimu_hi, Upsi_lo]
    list_3mu1trk_jpsi2mass = [Upsimass, Psi2Smass, Jpsimass, Phimass,
                              Psi2Smass, Jpsimass, Phimass,
                              Jpsimass, Phimass, Phimass,
                              Upsimass, Upsimass, Upsimass,
                              Psi2Smass, Psi2Smass, Jpsimass,
                              0.0, Upsimass] # 0.0 is dummy

    list_3mu1trk_obj = []
    for hypo in list_3mu1trk_hypo:
        list_3mu1trk_obj.append( CompFactory.DerivationFramework.PsiPlusPsiSingleVertex("BPHY13_"+hypo) )

    for i in range(len(list_3mu1trk_obj)):
        list_3mu1trk_obj[i].HypothesisName           = list_3mu1trk_hypo[i]
        list_3mu1trk_obj[i].Psi1Vertices             = list_3mu1trk_psi1Input[i]
        list_3mu1trk_obj[i].Psi2Vertices             = list_3mu1trk_psi2Input[i]
        list_3mu1trk_obj[i].NumberOfPsi1Daughters    = 2
        list_3mu1trk_obj[i].NumberOfPsi2Daughters    = 2
        list_3mu1trk_obj[i].MaxCandidates            = 20
        list_3mu1trk_obj[i].Jpsi1MassLowerCut        = list_3mu1trk_jpsi1lo[i]
        list_3mu1trk_obj[i].Jpsi1MassUpperCut        = list_3mu1trk_jpsi1hi[i]
        list_3mu1trk_obj[i].Jpsi2MassLowerCut        = list_3mu1trk_jpsi2lo[i]
        list_3mu1trk_obj[i].Jpsi2MassUpperCut        = list_3mu1trk_jpsi2hi[i]
        list_3mu1trk_obj[i].MassLowerCut             = 0.
        list_3mu1trk_obj[i].MassUpperCut             = 31000.
        if i == len(list_3mu1trk_obj)-2:
            list_3mu1trk_obj[i].Jpsi1Mass                = list_3mu1trk_jpsi1mass[i]
            list_3mu1trk_obj[i].ApplyJpsi1MassConstraint = True
            list_3mu1trk_obj[i].ApplyJpsi2MassConstraint = False
        elif i == len(list_3mu1trk_obj)-1:
            list_3mu1trk_obj[i].Jpsi2Mass                = list_3mu1trk_jpsi2mass[i]
            list_3mu1trk_obj[i].ApplyJpsi1MassConstraint = False
            list_3mu1trk_obj[i].ApplyJpsi2MassConstraint = True
        else:
            list_3mu1trk_obj[i].Jpsi1Mass                = list_3mu1trk_jpsi1mass[i]
            list_3mu1trk_obj[i].Jpsi2Mass                = list_3mu1trk_jpsi2mass[i]
            list_3mu1trk_obj[i].ApplyJpsi1MassConstraint = True
            list_3mu1trk_obj[i].ApplyJpsi2MassConstraint = True
        list_3mu1trk_obj[i].Chi2Cut                  = 25.
        list_3mu1trk_obj[i].PVRefitter               = acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags))
        list_3mu1trk_obj[i].TrkVertexFitterTool      = vkalvrt
        list_3mu1trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3mu1trk_obj[i].OutputVertexCollections  = ["BPHY13_"+list_3mu1trk_hypo[i]+"_SubVtx1","BPHY13_"+list_3mu1trk_hypo[i]+"_SubVtx2","BPHY13_"+list_3mu1trk_hypo[i]+"_MainVtx"]
        list_3mu1trk_obj[i].VxPrimaryCandidateName   = "PrimaryVertices"
        list_3mu1trk_obj[i].RefPVContainerName       = "BPHY13_"+list_3mu1trk_hypo[i]+"_RefPrimaryVertices"
        list_3mu1trk_obj[i].RefitPV                  = True
        list_3mu1trk_obj[i].MaxnPV                   = 100


    list_all_obj = list_4mu_obj + list_3mu1trk_obj

    OutputCollections = []
    RefPVContainers = []
    RefPVAuxContainers = []
    passedCandidates = []

    for obj in list_all_obj:
        OutputCollections += obj.OutputVertexCollections
        RefPVContainers += ["xAOD::VertexContainer#BPHY13_" + obj.HypothesisName + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY13_" + obj.HypothesisName + "_RefPrimaryVerticesAux."]
        passedCandidates += [ "BPHY13_" + obj.HypothesisName + "_MainVtx" ]

    BPHY13_SelectEvent = CompFactory.DerivationFramework.AnyVertexSkimmingTool(name = "BPHY13_SelectEvent", VertexContainerNames = passedCandidates)
    acc.addPublicTool(BPHY13_SelectEvent)

    augmentation_tools = [BPHY13_Reco_Phimumu, BPHY13_Reco_Jpsimumu, BPHY13_Reco_Psimumu, BPHY13_Reco_Upsimumu, BPHY13_Reco_Dimumumu, BPHY13_Reco_Phimutrk, BPHY13_Reco_Jpsimutrk, BPHY13_Reco_Psimutrk, BPHY13_Reco_Upsimutrk, BPHY13_Reco_Dimumutrk, BPHY13_Rev_Phimumu, BPHY13_Rev_Jpsimumu, BPHY13_Rev_Psimumu, BPHY13_Rev_Upsimumu, BPHY13_Rev_Phimutrk, BPHY13_Rev_Jpsimutrk, BPHY13_Rev_Psimutrk, BPHY13_Rev_Upsimutrk] + list_all_obj
    for t in augmentation_tools : acc.addPublicTool(t)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY13Kernel",
        AugmentationTools = augmentation_tools,
        SkimmingTools     = [BPHY13_SelectEvent]
    ))

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
    BPHY13_StaticContent += RefPVContainers
    BPHY13_StaticContent += RefPVAuxContainers

    ## ID track particles
    BPHY13_AllVariables += ["InDetTrackParticles"]

    ## combined / extrapolated muon track particles 
    ## (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    ##        are store in InDetTrackParticles collection)
    BPHY13_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    ## muon container
    BPHY13_AllVariables += ["Muons", "MuonSegments"]

    ## we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for output in OutputCollections:
        BPHY13_StaticContent += ["xAOD::VertexContainer#%s" % output]
        BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % output]

    # Truth information for MC only
    if isSimulation:
        BPHY13_AllVariables += ["TruthEvents","TruthParticles","TruthVertices","MuonTruthParticles"]

    BPHY13SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
    BPHY13SlimmingHelper.AllVariables = BPHY13_AllVariables
    BPHY13SlimmingHelper.StaticContent = BPHY13_StaticContent

    BPHY13ItemList = BPHY13SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY13", ItemList=BPHY13ItemList, AcceptAlgs=["BPHY13Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY13", AcceptAlgs=["BPHY13Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents = [], printDefaults=True)
    return acc
