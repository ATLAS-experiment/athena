# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class BootstrapGeneratorConfig(ConfigBlock):
    '''ConfigBlock for the bootstrap generator'''

    def __init__(self):
        super(BootstrapGeneratorConfig, self).__init__()
        self.addOption ('nReplicas', 1000, type=int,
            info="the number (int) of bootstrap replicas to generate. "
            "The default is 1000.")
        self.addOption ('decoration', None, type=str,
            info="the name of the output vector branch containing the "
            "bootstrapped weights. The default is bootstrapWeights.")
        self.setOptionValue('skipOnMC', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # no instance name needed for singleton block

    def makeAlgs(self, config):

        alg = config.createAlgorithm( 'CP::BootstrapGeneratorAlg', 'BootstrapGenerator')
        alg.nReplicas = self.nReplicas
        alg.isData = config.dataType() is DataType.Data
        if self.decoration:
            alg.decorationName = self.decoration
        else:
            alg.decorationName = "bootstrapWeights_%SYS%"

        config.addOutputVar ('EventInfo', alg.decorationName, alg.decorationName.split("_%SYS%")[0], noSys=True)

        return
