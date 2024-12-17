# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class ParticleLevelMissingETBlock(ConfigBlock):
    """ConfigBlock for particle-level truth missing transverse energy"""

    def __init__(self):
        super(ParticleLevelMissingETBlock, self).__init__()
        self.addOption('containerName', 'MET_Truth', type=str,
                       info='the name of the input truth MET container')

    def makeAlgs(self, config):
        if config.dataType() is DataType.Data:
            return

        # decorate the energy and phi so we can save them later
        alg = config.createAlgorithm( 'CP::ParticleLevelMissingETAlg', 'ParticleLevelMissingET' + self.containerName )
        alg.met = self.containerName

        newContainerName = "Truth_MET"
        config.setSourceName (newContainerName, self.containerName, isMet=True)
        if config.wantCopy (newContainerName):
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'TruthMissingETShallowCopyAlg' )
            alg.input = config.readName (newContainerName)
            alg.output = config.copyName (newContainerName)


        outputVars = [
            ['met_met', 'met'],
            ['met_phi', 'phi'],
        ]
        for decoration, branch in outputVars:
            config.addOutputVar (newContainerName, decoration, branch, noSys=True)
