# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class BootstrapGeneratorConfig(ConfigBlock):
    '''ConfigBlock for the bootstrap generator'''

    def __init__(self):
        super().__init__()
        self.addOption ('nReplicas', 1000, type=int,
            info="the number of bootstrap replicas to generate.")
        self.addOption ('decoration', None, type=str,
            info="the name of the output vector branch containing the "
            "bootstrapped weights.")
        self.setOptionValue('skipOnMC', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # no instance name needed for singleton block

    def makeAlgs(self, config):

        alg = config.createAlgorithm( 'CP::BootstrapGeneratorAlg', 'BootstrapGenerator', reentrant=True)
        alg.nReplicas = self.nReplicas
        alg.isData = config.dataType() is DataType.Data
        decorationName = (self.decoration or "bootstrapWeights").replace("_%SYS%", "")
        alg.decorationName = decorationName

        config.addOutputVar ('EventInfo', decorationName, decorationName, noSys=True, auxType='vector_uint8')

        return
