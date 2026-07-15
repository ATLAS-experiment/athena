# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.Constants import DEBUG 


class BoostedJetTaggerAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the Boosted Jet Tagger tool sequence"""

    def __init__ (self) :
        super (BoostedJetTaggerAnalysisConfig, self).__init__ ()
        self.setBlockName('BoostedJetTagger')

        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")

        self.addOption ('CalibArea', '', type=str,
            noneAction='error',
            info="calibration area on cvmfs.")

        self.addOption ('IsMC', '', type=bool,
            noneAction='error',
            info="wheter to run on MC or data samples.")

        self.addOption ('ConfigFile', '', type=str,
            noneAction='error',
            info="the name of the input configuration file.")


    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + '_BoostedJetTagger_'

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data: return

        # Set up the per-event jet efficiency scale factor calculation algorithm
        alg = config.createAlgorithm('BJT::BoostedJetTaggerAlg', 'BoostedJetTaggerAlg')

        # configure the tool
        config.addPrivateTool( 'tagger', 'SmoothedWZTagger' )
        alg.tagger.ConfigFile = self.ConfigFile
        alg.tagger.CalibArea = self.CalibArea
        alg.tagger.IsMC = self.IsMC ### to adjust
        alg.tagger.OutputLevel = DEBUG ### to remove

        # configure algorithm
        alg.jets = config.readName(self.containerName)

        # output info
        config.addOutputVar(self.containerName, 'WTransformer_Tagged', 'WTransformer_Tagged', auxType='char') # harmonise the decoration tag with the cfg file
