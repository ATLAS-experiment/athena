# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

# Jet config imports
#from BoostedJetTaggers.BoostedJetTaggerConfig import BJTToolCfg


class JetBJTAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the Boosted Jet Tagger tool sequence"""

    def __init__ (self) :
        super (JetBJTAnalysisConfig, self).__init__ ()
        self.setBlockName('BJT')

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
        return self.containerName + '_testBJT_' # + self.postfix

    def makeAlgs (self, config) :

        if config.dataType() is DataType.Data: return

        # Set up the per-event jet efficiency scale factor calculation algorithm
        alg = config.createAlgorithm('BJT::BoostedJetTaggerAlg', 'BoostedJetTaggerAlg')

        # configure tool
        #ConfigFile = '/eos/atlas/atlascerngroupdisk/perf-jets/LocalStorage/TAGGING/PreliminaryConfigs/WTagger/WTagger_AntiKt10UFOSoftDrop_ParT_FixSigEff50.dat'
        #tool = BJTToolCfg(config.flags, 
        #                  #ConfigFile=self.ConfigFile, 
        #                  ConfigFile=ConfigFile, 
        #                  ContainerName=self.containerName)

        config.addPrivateTool( 'tagger', 'SmoothedWZTagger' )
        alg.tagger.ConfigFile = self.ConfigFile
        alg.tagger.CalibArea = self.CalibArea
        alg.tagger.IsMC = self.IsMC

        # configure algorithm
        #alg.jets = self.containerName
        alg.jets = config.readName(self.containerName)

        #alg.tagger = tool

        # output info
        #config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_jvt_effSF' + postfix)
