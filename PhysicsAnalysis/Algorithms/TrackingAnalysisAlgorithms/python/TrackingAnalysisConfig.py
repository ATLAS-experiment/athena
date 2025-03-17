# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import AthenaCommon.SystemOfUnits as Units

#class PixelToTPIDDualToolConfig (ConfigBlock) :  ## should match the alg in ../TrackingAnalysisAlgorithms I think... not the tool...
class PixelToTPIDBlock (ConfigBlock) :  ## should match the alg in ../TrackingAnalysisAlgorithms I think... not the tool...
    """the ConfigBlock for the Pixel ToT PID tool"""

    def __init__ (self, containerName='') :
        # super (PixelToTPIDDualToolConfig, self).__init__ ()
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
        # log = logging.getLogger('PixelToTPIDDualToolConfig')
        log = logging.getLogger('PixelToTPIDBlock')
        
        # Setup the muon quality selection
        alg = config.createAlgorithm( 'CP::PixelDEdxEqualizationAlg', ## should match the alg in ../TrackingAnalysisAlgorithms I think... not the tool...
                               'PixelDEdxEqualizationAlg' + self.postfix )
        #config.addPrivateTool( 'PixelDEdxEqualizationAlg', 'CP::PixelDEdxEqualizationAlg' )
        config.addPrivateTool( 'PixelToTPIDDualTool', 'CP::PixelToTPIDDualTool' )
        alg.PixelToTPIDDualTool.TrackContainerName = self.containerName
        alg.PixelToTPIDDualTool.EqualizeClusterMeasurements = self.equalizeClusterMeasurements
    


### Migrate others from TrackingAnalysisAlgorithmsConfig.py
