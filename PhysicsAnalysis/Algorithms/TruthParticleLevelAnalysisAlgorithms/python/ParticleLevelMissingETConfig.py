# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelMissingETBlock(ConfigBlock):
    """ConfigBlock for particle-level truth missing transverse energy"""

    def __init__(self):
        super(ParticleLevelMissingETBlock, self).__init__()
        self.addOption('containerName', 'MET_Truth', type=str,
                       info='the name of the input truth MET container')
        self.addOption('outputContainerName', 'TruthMET', type=str,
                       info='the name of the output MET container')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):
        # decorate the energy and phi so we can save them later
        alg = config.createAlgorithm( 'CP::ParticleLevelMissingETAlg',
                                      'ParticleLevelMissingET',
                                      reentrant=True )
        alg.met = self.containerName

        config.setSourceName (self.outputContainerName, self.containerName, isMet=True)
        if config.wantCopy (self.outputContainerName):
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'TruthMissingETShallowCopyAlg' )
            alg.input = config.readName (self.outputContainerName)
            alg.output = config.copyName (self.outputContainerName)

        config.addOutputVar (self.outputContainerName, 'met', 'met', noSys=True)
        config.addOutputVar (self.outputContainerName, 'phi', 'phi', noSys=True)
