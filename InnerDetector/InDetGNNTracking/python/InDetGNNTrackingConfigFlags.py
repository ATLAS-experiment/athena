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
    icf.addFlag("Tracking.GNN.TrackFinder.inputMLModelDir", "TrainedMLModels4ITk")
    icf.addFlag("Tracking.GNN.TrackFinder.ORTExeProvider", OnnxRuntimeType.CPU)
    
    # GNN Track Reader Tool
    icf.addFlag("Tracking.GNN.TrackReader.inputTracksDir", "gnntracks")
    icf.addFlag("Tracking.GNN.TrackReader.csvPrefix", "track")

    icf.addFlag("Tracking.GNN.useClusterTracks", False)
    
    # the following cuts are applied to the tracks before the track fitting
    icf.addFlag("Tracking.GNN.minPixelClusters", 1)
    icf.addFlag("Tracking.GNN.minStripClusters", 0)
    icf.addFlag("Tracking.GNN.minClusters", 6)

    # the following cuts are applied to the tracks after the track fitting
    icf.addFlag("Tracking.GNN.etamax", 4.0)
    import AthenaCommon.SystemOfUnits as Units
    icf.addFlag("Tracking.GNN.pTmin", 400. * Units.MeV)

    # this option applies eta dependent track selection to the output tracks
    icf.addFlag("Tracking.GNN.doRecoTrackCuts", True)

    # this option turns on the recovery attempts for failed track fits
    icf.addFlag("Tracking.GNN.doRecoverFailedFits", True)

    # this option turns on the ambiguity resolution, False by default
    icf.addFlag("Tracking.GNN.doAmbiResolution", False)

    return icf

