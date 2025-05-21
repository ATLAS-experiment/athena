# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

class PixelDEdxEqualizationBlock (ConfigBlock) :  ## should match the alg in ../TrackingAnalysisAlgorithms I think... not the tool...
    """the ConfigBlock for the Pixel ToT PID tool"""

    def __init__ (self, containerName='') :
        super (PixelDEdxEqualizationBlock, self).__init__ ()
        self.setBlockName('PixelDEdxEqualization')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('postfix', "", type=str,
            info="a postfix to apply to decorations and algorithm names.")
        self.addOption ('msosLink', "", type=str,
            info="Name of link from tracks to MSOSs.")
        self.addOption ('equalizeClusterMeasurements', False, type=bool,
            info="whether to equalize cluster level dE/dx measurements.")
        self.addOption ('equalizeTrackMeasurements', False, type=bool,
            info="whether to equalize track-level truncated mean dE/dx measurements (no pixel clusters required).")
        self.addOption ('tightClusterCleaning', False, type=bool,
            info="whether to perform extra cluster cleaning for dE/dx measurements (e.g. cluster size/shape).")
        self.addOption ('trackPtCutMeV', -1., type=float,
            info="Minimum track pT for equalizing dE/dx & decorating.")
        self.addOption ('sfLocalFileName', "", type=str,
            info="Path to scale factor trees, overriding files stored in ASG calibration area.")
        self.addOption ('clusterSFTreeName', "cluster_SFs", type=str, # FIX! TBD
            info="Name of tree storing the cluster-level dE/dx equalization scale factors.")
        self.addOption ('trackSFTreeName', "track_SFs", type=str, # FIX! TBD
            info="Name of tree storing the track-level dE/dx equalization scale factors.")

        
    def makeAlgs (self, config) :
        alg = config.createAlgorithm( 'CP::PixelDEdxEqualizationAlg',
                                      'PixelDEdxEqualizationAlg' + self.postfix,
                                      reentrant=True)
        config.addPrivateTool( 'PixelDEdxEqualizationTool', 'CP::PixelDEdxEqualizationTool' )
        ### Algorithm properties
        alg.TrackContainerName = self.containerName
        alg.MSOSLink = self.msosLink
        alg.EqualizeClusterMeasurements = self.equalizeClusterMeasurements
        alg.EqualizeTrackMeasurements = self.equalizeTrackMeasurements
        alg.TightClusterCleaning = self.tightClusterCleaning
        alg.TrackPtCutMeV = self.trackPtCutMev
        ### Tool properties
        alg.PixelDEdxEqualizationTool.EqualizeClusterMeasurements = self.equalizeClusterMeasurements
        alg.PixelDEdxEqualizationTool.EqualizeTrackMeasurements = self.equalizeTrackMeasurements
        alg.PixelDEdxEqualizationTool.SFLocalFileName = self.sfLocalFileName
        alg.PixelDEdxEqualizationTool.ClusterSFTreeName = self.clusterSFTreeName
        alg.PixelDEdxEqualizationTool.TrackSFTreeName = self.trackSFTreeName
    


### Migrate others from TrackingAnalysisAlgorithmsConfig.py
