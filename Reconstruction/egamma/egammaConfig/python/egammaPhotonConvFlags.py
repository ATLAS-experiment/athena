# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import LHCPeriod

def createEGammaPhotonConvFlags():
    flags = AthConfigFlags()

    # Track selector tool
    flags.addFlag("TrkSel.minPt", lambda pcf: (
        pcf.Tracking.__getattr__(pcf.Tracking.PrimaryPassConfig.value+'Pass').minPT
        if pcf.GeoModel.Run <= LHCPeriod.Run3 else
        pcf.Tracking.__getattr__(pcf.Tracking.ITkPrimaryPassConfig.value+'Pass').minPT[0]
    ))
    # e-prob for Si conversion tracks (affects 1Si, 2Si, SiTRT): Ntrt < 15
    flags.addFlag("TrkSel.RatioCut1", 0.0)
    # e-prob for Si conversion tracks (affects 1Si, 2Si, SiTRT): 15 < Ntrt < 25
    flags.addFlag("TrkSel.RatioCut2", 0.1)
    # e-prob for Si conversion tracks (affects 1Si, 2Si, SiTRT): Ntrt > 25
    flags.addFlag("TrkSel.RatioCut3", 0.1)
    # e-prob cut for TRT conversion tracks (affects 1TRT, 2TRT, SiTRT) (see also below)
    flags.addFlag("TrkSel.RatioTRT", 0.51)
    # eta bins (10) for eta-dep cuts on TRT conversion tracks
    flags.addFlag("TrkSel.TRTTrksEtaBins", [
        0.7,   0.8,   0.9,  1.2,  1.3,  1.6,  1.7,  1.8,  1.9,  999])
    # eta-dep e-prob cut for TRT conversion tracks
    flags.addFlag("TrkSel.TRTTrksBinnedRatioTRT", lambda pcf: (
        [0.51,  0.80,  0.90, 0.80, 0.51, 0.51, 0.51, 0.51, 0.51, 0.51]
        if pcf.GeoModel.Run is LHCPeriod.Run3
        else [0.60,  0.80,  0.90, 0.80, 0.51, 0.51, 0.51, 0.51, 0.51, 0.51]))

    # Track pairs selector
    # hacky way to determine if TRT only of SI
    flags.addFlag("TrkPairSel.MaxDistBetweenTracks", [10., 50., 50.])
    # delta cotan(theta) cuts, no cut in barrel for TRT only in code
    flags.addFlag("TrkPairSel.MaxEta", [0.3, 0.5, 0.5])
    flags.addFlag("TrkPairSel.MinTrackAngle", 0.0)

    # Vertex point estimator
    flags.addFlag("VtxPt.MinDeltaR", [-5., -25., -50.])
    flags.addFlag("VtxPt.MaxDeltaR", [5., 10., 10.])
    flags.addFlag("VtxPt.MaxPhi", [0.05, 0.2, 0.2])

    # Secondary Vertex post selector
    flags.addFlag("SecVtxPost.MinRadius", [20.0, 70.0, 250.0])
    flags.addFlag("SecVtxPost.MaxPhiVtxTrk", 0.2)

    # Single track secondary vertex tool
    flags.addFlag("SingleTrk.MaxBLayerHits", 0)
    flags.addFlag("SingleTrk.MinInitialHitRadius", 70.)
    flags.addFlag("SingleTrk.MinInitialHitRadius_noBlay", 120.)
    # e-prob cut for 1TRT and 1Si converisons
    flags.addFlag("SingleTrk.MinRatioOfHLhits", lambda pcf: (
        0.40 if pcf.GeoModel.Run is LHCPeriod.Run3 else 0.51))

    #  InDetSecVtxFinderTool
    flags.addFlag("Finder.RemoveTrtTracks", False)
    flags.addFlag("Finder.MinDistVtxHit", -350.)
    flags.addFlag("Finder.MaxDistVtxHit", 250.)
    flags.addFlag("Finder.MinFlightAngle", 0.)
    flags.addFlag("Finder.MinInitVtxR", 0.)

    return flags
