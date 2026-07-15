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

        self.addOption ('ConfigFile', '', type=str,
            noneAction='error',
            info="the name of the input configuration file.")

        self.addOption ('Tagger', '', type=str,
            noneAction='error',
            info="the jet tagger: W/Z, top, q/g.")

        self.addOption ('Generation', '', type=str,
            noneAction='error',
            info="the tagger generation.")

        self.addOption ('WP', '', type=str,
            noneAction='error',
            info="the working point selection to apply.")


    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + '_BoostedJetTagger_'

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data: return

        # Set up the per-event jet efficiency scale factor calculation algorithm
        alg = config.createAlgorithm('BJT::BoostedJetTaggerAlg', 'BoostedJetTaggerAlg')

        # pick the config file
        ConfigFile = self.Tagger + 'Tagger_AntiKt10UFOSoftDrop_' + self.Generation + '_FixSigEff' + self.WP
        if self.ConfigFile != '':
            ConfigFile = self.ConfigFile

        # configure the tool
        config.addPrivateTool('tagger', 'SmoothedWZTagger')
        alg.tagger.ConfigFile = ConfigFile
        alg.tagger.CalibArea = self.CalibArea
        alg.tagger.IsMC = True ### to adjust
        alg.tagger.OutputLevel = DEBUG ### to remove

        # configure algorithm
        alg.jets = config.readName(self.containerName)

        # output info
        decoration_name = self.Tagger
        if self.Generation == 'ParT':
            decoration_name += 'Transformer'
        else:
            raise Exception('not supported tagger!')
        config.addOutputVar(self.containerName, decoration_name + '_Tagged', decoration_name + '_Tagged', auxType='char')
