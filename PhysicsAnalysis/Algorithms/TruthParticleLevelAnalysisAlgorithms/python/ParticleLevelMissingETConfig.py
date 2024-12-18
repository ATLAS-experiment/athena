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

    def makeAlgs(self, config):
        # decorate the energy and phi so we can save them later
        alg = config.createAlgorithm( 'CP::ParticleLevelMissingETAlg', 'ParticleLevelMissingET' + self.containerName )
        alg.met = self.containerName

        config.setSourceName (self.outputContainerName, self.containerName, isMet=True)
        if config.wantCopy (self.outputContainerName):
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'TruthMissingETShallowCopyAlg' )
            alg.input = config.readName (self.outputContainerName)
            alg.output = config.copyName (self.outputContainerName)

        outputVars = [
            ['met_met', 'met'],
            ['met_phi', 'phi'],
        ]
        for decoration, branch in outputVars:
            config.addOutputVar (self.outputContainerName, decoration, branch, noSys=True)
