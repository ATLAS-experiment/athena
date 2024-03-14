# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class BootstrapGeneratorConfig(ConfigBlock):
    '''ConfigBlock for the bootstrap generator'''

    def __init__(self):
        super(BootstrapGeneratorConfig, self).__init__()
        self.addOption ('nReplicas', 1000, type=int)
        self.addOption ('decoration', None, type=str)
        self.addOption ('runOnMC', False, type=bool)
    
    def makeAlgs(self, config):
        if config.dataType() is not DataType.Data and not self.runOnMC:
            print("Skipping the configuration of CP::BootstrapGeneratorAlg since we are not running on data. "
                  "Set the option 'runOnMC' to True if you want to force the bootstrapping of MC too.")
            return
        
        alg = config.createAlgorithm( 'CP::BootstrapGeneratorAlg', 'BootstrapGenerator')
        alg.nReplicas = self.nReplicas
        alg.isData = config.dataType() is DataType.Data
        if self.decoration:
            alg.decorationName = self.decoration
        else:
            alg.decorationName = "bootstrapWeights_%SYS%"
        
        config.addOutputVar ('EventInfo', alg.decorationName, alg.decorationName.split("_%SYS%")[0], noSys=True)

        return

def makeBootstrapGeneratorConfig(seq,
                                 nReplicas = None,
                                 decoration = None,
                                 runOnMC = None):
    """
    Setup a simple bootstrapping algorithm

    Keyword arguments:
      nReplicas -- the number of bootstrap replicas to generate
      decoration -- the name of the output vector branch containing the bootstrapped weights
      runOnMC -- toggle to force running on MC samples (default: only data)
    """

    config = BootstrapGeneratorConfig()
    config.setOptionValue ('nReplicas', nReplicas)
    config.setOptionValue ('decoration', decoration)
    config.setOptionValue ('runOnMC', runOnMC)
    seq.append (config)
