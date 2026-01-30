# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

class PhotonExtraVariablesBlock(ConfigBlock):
    """a ConfigBlock for additional photon output variables"""
    """Decorates the output photons with the conversion type and calorimeter eta"""
    """and writes them to the output. Useful e.g. for photon-fake studies."""

    def __init__(self):
        super(PhotonExtraVariablesBlock, self).__init__()
        self.addOption('containerName', None, type=str, info='the input photon container.')

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):

        alg = config.createAlgorithm('CP::PhotonExtraVariablesAlg', 'PhotonExtraVariables')
        alg.photons = config.readName(self.containerName)
        alg.affectingSystematicsFilter = '.*'

        config.addOutputVar(self.containerName, 'conversionType_%SYS%', 'conversionType', noSys=True)
        config.addOutputVar(self.containerName, 'caloEta2_%SYS%', 'caloEta2', noSys=True)
