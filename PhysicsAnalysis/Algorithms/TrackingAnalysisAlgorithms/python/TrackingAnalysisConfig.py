# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import AthenaCommon.SystemOfUnits as Units

class PixelToTPIDBlock (ConfigBlock) :  ## should match the alg in ../TrackingAnalysisAlgorithms I think... not the tool...
    """the ConfigBlock for the Pixel ToT PID tool"""

    def __init__ (self, containerName='') :
        super (PixelToTPIDBlock, self).__init__ ()
        self.setBlockName('PixelToTPID')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('postfix', "", type=str,
            info="a postfix to apply to decorations and algorithm names.")
        self.addOption ('equalizeClusterMeasurements', False, type=bool,
            info="whether to equalize cluster level dE/dx measurements. ")

        
    def makeAlgs (self, config) :
        alg = config.createAlgorithm( 'CP::PixelDEdxEqualizationAlg',
                                      'PixelDEdxEqualizationAlg' + self.postfix,
                                      reentrant=True)
        config.addPrivateTool( 'PixelToTPIDTool', 'CP::PixelToTPIDTool' )
        # alg.PixelToTPIDTool.TrackContainerName = self.containerName
        alg.TrackContainerName = self.containerName # belongs to alg, not tool.
        alg.PixelToTPIDTool.EqualizeClusterMeasurements = self.equalizeClusterMeasurements # belongs to tool, not alg.
    


### Migrate others from TrackingAnalysisAlgorithmsConfig.py
