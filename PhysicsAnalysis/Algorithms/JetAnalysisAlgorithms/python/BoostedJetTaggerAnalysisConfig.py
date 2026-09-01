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

        self.addOption ('decorations', [], type=list,
            noneAction='error',
            info="list of the names of decorations to add in output.")


    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + '_BoostedJetTagger_'

    def makeAlgs (self, config) :

        # Set up the per-event jet efficiency scale factor calculation algorithm
        alg = config.createAlgorithm('BJT::BoostedJetTaggerAlg', 'BoostedJetTaggerAlg')

        # build the config file
        ConfigFile = self.Tagger + 'Tagger_AntiKt10UFOSoftDrop_' + self.Generation + '_FixSigEff' + self.WP
        # or override it
        if self.ConfigFile != '':
            ConfigFile = self.ConfigFile

        # configure the tool
        tagger_class = ''
        if self.Tagger == 'W':
            if self.Generation == 'ANN':
                tagger_class = 'JSSWTopTaggerANN'
            elif self.Generation == 'DNN':
                tagger_class = 'JSSWTopTaggerDNN'
            else:
                tagger_class = 'SmoothedWZTagger'
        elif self.Tagger == 'Top':
            if self.Generation == 'ANN':
                tagger_class = 'JSSWTopTaggerANN'
            elif self.Generation == 'DNN':
                tagger_class = 'JSSWTopTaggerDNN'
            else:
                tagger_class = 'SmoothedTopTagger'
        elif self.Tagger == 'qg':
            tagger_class = 'BJT::qgTagger'
        elif self.Tagger == 'inference':
            tagger_class = 'JSSTaggerUtils'
        else:
            raise Exception('not supported tagger!')

        config.addPrivateTool('tagger', tagger_class)
        alg.tagger.ConfigFile = ConfigFile
        alg.tagger.CalibArea = self.CalibArea
        alg.tagger.IsMC = config.dataType() is not DataType.Data
        # for debugging/developments, can configure it?
        # alg.tagger.OutputLevel = DEBUG

        # configure algorithm
        alg.jets = config.readName(self.containerName)

        # output info
        decoration_name = self.Tagger
        # auto config for ParT
        if self.Generation == 'ParT':
            decoration_name += 'Transformer_50eff'
            decorations = [decoration_name]
        else:
            raise Exception('not supported tagger!')
        # override from yaml config
        if len(self.decorations) > 0:
            decorations = self.decorations

        for decoration_name in decorations:
            config.addOutputVar(self.containerName,
                                decoration_name + '_Tagged',
                                decoration_name + '_Tagged',
                                auxType='char')
