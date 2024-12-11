#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType

def createGNNTrackingConfigFlags():
    """Create flags for configuring the GNN tracking."""
    icf = AthConfigFlags()
    icf.addFlag("Tracking.GNN.useTrackFinder", False)
    icf.addFlag("Tracking.GNN.useTrackReader", False)
    icf.addFlag("Tracking.GNN.usePixelHitsOnly", False)

    # Dump objects
    icf.addFlag("Tracking.GNN.DumpObjects.NtupleFileName", "/DumpObjects/")
    icf.addFlag("Tracking.GNN.DumpObjects.NtupleTreeName", "GNN4ITk")

    # GNN Track finder tool
    icf.addFlag("Tracking.GNN.TrackFinder.embeddingDim", 8)
    icf.addFlag("Tracking.GNN.TrackFinder.rVal", 1.7)
    icf.addFlag("Tracking.GNN.TrackFinder.knnVal", 500)
    icf.addFlag("Tracking.GNN.TrackFinder.filterCut", 0.21)
    icf.addFlag("Tracking.GNN.TrackFinder.inputMLModelDir", "TrainedMLModels4ITk")
    icf.addFlag("Tracking.GNN.TrackFinder.ORTExeProvider", OnnxRuntimeType.CPU)
    
    # GNN Track Reader Tool
    icf.addFlag("Tracking.GNN.TrackReader.inputTracksDir", "gnntracks")
    icf.addFlag("Tracking.GNN.TrackReader.csvPrefix", "track")

    icf.addFlag("Tracking.GNN.useClusterTracks", False)
    icf.addFlag("Tracking.GNN.doAmbiResolution", True)

    return icf
