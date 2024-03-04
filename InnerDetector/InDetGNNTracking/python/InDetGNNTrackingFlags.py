#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from TrkConfig.TrackingPassFlags import createITkTrackingPassFlags
from TrkConfig.TrkConfigFlags import TrackingComponent


def createGNNTrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "GNN"
    icf.doAthenaCluster = True
    icf.doAthenaSpacePoint = True
    icf.doAthenaSeed = False
    icf.doAthenaTrack = False
    icf.doAthenaAmbiguityResolution = True

    icf.doGNNTrack = True

    icf.doActsCluster = False
    icf.doActsSpacePoint = False
    icf.doActsSeed = False
    icf.doActsTrack = False
    return icf


def gnnReaderValidation(flags):
    """flags for Reco_tf with CA used in CI tests: use GNNChain during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.useTrackReader = True
    flags.Tracking.GNN.useTrackFinder = False


def gnnFinderValidation(flags):
    """flags for Reco_tf with CA used in CI tests: use GNNChain during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.useTrackReader = False
    flags.Tracking.GNN.useTrackFinder = True
