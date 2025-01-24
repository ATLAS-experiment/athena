# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelMuonsBlock(ConfigBlock):
    """ConfigBlock for particle-level truth muons"""

    def __init__(self):
        super(ParticleLevelMuonsBlock, self).__init__()
        self.addOption('containerName', 'TruthMuons', type=str,
                       info='the name of the input truth muons container')
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. The default is "",'
                       ' which applies the selection to all truth muons.')
        self.addOption('isolated', True, type=bool,
                       info='select only truth muons that are isolated.')
        self.addOption('notFromTau', True, type=bool,
                       info='select only truth muons that did not orginate '
                       'from a tau decay.')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # decorate the charge so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelChargeDecoratorAlg',
                                     'ParticleLevelChargeDecoratorMuons' + self.selectionName,
                                     reentrant=True)
        alg.particles = self.containerName

        # check for prompt isolation and possible origin from tau decays
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg',
                                     'ParticleLevelIsolationMuons' + self.selectionName,
                                     reentrant=True)
        alg.particles    = self.containerName
        alg.isolation    = 'isIsolated' + self.selectionName if self.isolated else 'isIsolatedButNotRequired' + self.selectionName
        alg.notTauOrigin = 'notFromTau' + self.selectionName if self.notFromTau else 'notFromTauButNotRequired' + self.selectionName
        alg.checkType    = 'IsoMuon'

        if self.isolated:
            config.addSelection (self.containerName, self.selectionName, alg.isolation+',as_char')
        if self.notFromTau:
            config.addSelection (self.containerName, self.selectionName, alg.notTauOrigin+',as_char')

        # output branches to be scheduled only once
        if ParticleLevelMuonsBlock.get_instance_count() == 1:
            outputVars = [
                ['pt_dressed', 'pt'],
                ['eta_dressed', 'eta'],
                ['phi_dressed', 'phi'],
                ['e_dressed', 'e'],
                ['charge', 'charge'],
                ['classifierParticleType', 'type'],
                ['classifierParticleOrigin', 'origin'],
            ]
            for decoration, branch in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True)


