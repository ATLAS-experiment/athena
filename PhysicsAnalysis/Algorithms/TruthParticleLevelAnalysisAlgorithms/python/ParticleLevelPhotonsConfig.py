# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class ParticleLevelPhotonsBlock(ConfigBlock):
    """ConfigBlock for particle-level truth photons"""

    def __init__(self):
        super(ParticleLevelPhotonsBlock, self).__init__()
        self.addOption('containerName', 'TruthPhotons', type=str,
                       info='the name of the input truth photons container')
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. The default is "",'
                       ' which applies the selection to all truth photons.')
        self.addOption('isolated', True, type=bool,
                       info='select only truth photons that are isolated.')
        self.addOption('isolationVariable', '', type=str,
                       info='variable to use in isolation cuts of the form "var/pT < cut".')
        self.addOption('isolationCut', -1, type=float,
                       info='threshold to use in isolation cuts of the form "var/pT < cut".')

    def makeAlgs(self, config):
        if config.dataType() is DataType.Data:
            return

        config.setSourceName (self.containerName, self.containerName)

        # decorate the missing elements of the 4-vector so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelPtEtaPhiDecoratorAlg', 'ParticleLevelPtEtaPhiDecoratorPhotons' + self.selectionName)
        alg.particles = self.containerName

        # check for prompt isolation
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg', 'ParticleLevelIsolationPhotons' + self.selectionName)
        alg.particles    = self.containerName
        alg.isolation    = 'isIsolated' + self.selectionName if self.isolated else 'isIsolatedButNotRequired' + self.selectionName
        alg.notTauOrigin = 'notFromTauButNotRequired' + self.selectionName
        alg.checkType    = 'IsoPhoton'
        if self.isolationVariable != '':
            alg.isoVar       = self.isolationVariable
            alg.isoCut       = self.isolationCut

        if self.isolated:
            config.addSelection (self.containerName, self.selectionName, alg.isolation+',as_char')

        # output branches to be scheduled only once
        if ParticleLevelPhotonsBlock.get_instance_count() == 1:
            outputVars = [
                ['pt', 'pt'],
                ['eta', 'eta'],
                ['phi', 'phi'],
                ['e', 'e'],
                ['classifierParticleType', 'type'],
                ['classifierParticleOrigin', 'origin'],
            ]
            for decoration, branch in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True)
