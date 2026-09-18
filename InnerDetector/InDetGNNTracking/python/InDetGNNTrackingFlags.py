#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from TrkConfig.TrackingPassFlags import createITkTrackingPassFlags
from TrkConfig.TrkConfigFlags import TrackingComponent
from InDetGNNTracking.InDetGNNTrackingConfigFlags import GNNTrackFinderToolType
import AthenaCommon.SystemOfUnits as Units

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

    # Keep old IP selection cut for d0 (see https://gitlab.cern.ch/atlas/athena/-/merge_requests/89555)
    icf.maxPrimaryImpactSeed = [2.0 * Units.mm]
    icf.maxPrimaryImpact = [2.0 * Units.mm, 2.0 * Units.mm, 10.0 * Units.mm]

    return icf


def gnnReaderValidation(flags):
    """flags for Reco_tf with CA used in CI tests: use GNNChain during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.doITkFastTracking = False 
    flags.Tracking.doPixelDigitalClustering = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.ToolType = GNNTrackFinderToolType.TrackReader


def gnnFinderValidation(flags):
    """flags for Reco_tf with CA used in CI tests: use GNNChain during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.doITkFastTracking = False 
    flags.Tracking.doPixelDigitalClustering = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.ToolType = GNNTrackFinderToolType.TrackFinder


def gnnTritonValidation(flags):
    """flags for Reco_tf with CA. Use GNNTrackFinderTritonTool for track finding."""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.doITkFastTracking = False 
    flags.Tracking.doPixelDigitalClustering = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.ToolType = GNNTrackFinderToolType.Triton


def gnnActsPipelineValidation(flags):
    """flags for Reco_tf with CA. Use ActsGnnModuleMapFinderTool for track finding."""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.doITkFastTracking = False 
    flags.Tracking.doPixelDigitalClustering = False
    flags.Tracking.recoChain = [TrackingComponent.GNNChain]
    flags.Tracking.GNN.ToolType = GNNTrackFinderToolType.ActsPipeline
