#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
from AthenaConfiguration.Enums import FlagEnum


class GNNTrackFinderToolType(FlagEnum):
    TrackReader = "TrackReader"
    TrackFinder = "TrackFinder"
    Triton = "Triton"


def createGNNTrackingConfigFlags():
    """Create flags for configuring the GNN tracking."""
    icf = AthConfigFlags()
    icf.addFlag("Tracking.GNN.ToolType", GNNTrackFinderToolType.TrackReader, type=GNNTrackFinderToolType)
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

    # Triton Tool
    icf.addFlag("Tracking.GNN.Triton.url", "localhost")
    icf.addFlag("Tracking.GNN.Triton.model", "MetricLearning")
    icf.addFlag("Tracking.GNN.Triton.port", 8001)
    icf.addFlag("Tracking.GNN.spacepointFeatures", "r,phi,z,cluster_x_1,cluster_y_1,cluster_z_1,cluster_x_2,cluster_y_2,cluster_z_2,count_1,charge_count_1,loc_eta_1,loc_phi_1,localDir0_1,localDir1_1,localDir2_1,lengthDir0_1,lengthDir1_1,lengthDir2_1,glob_eta_1,glob_phi_1,eta_angle_1,phi_angle_1,count_2,charge_count_2,loc_eta_2,loc_phi_2,localDir0_2,localDir1_2,localDir2_2,lengthDir0_2,lengthDir1_2,lengthDir2_2,glob_eta_2,glob_phi_2,eta_angle_2,phi_angle_2,eta,cluster_r_1,cluster_phi_1,cluster_eta_1,cluster_r_2,cluster_phi_2,cluster_eta_2")

    return icf

