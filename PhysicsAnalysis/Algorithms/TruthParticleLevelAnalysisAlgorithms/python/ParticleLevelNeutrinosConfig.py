# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelNeutrinosBlock(ConfigBlock):
    """ConfigBlock for particle-level truth neutrinos"""

    def __init__(self):
        super(ParticleLevelNeutrinosBlock, self).__init__()
        self.addOption('containerName', 'TruthNeutrinos', type=str,
                       info='the name of the input truth neutrinos container')
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. The default is "",'
                       ' which applies the selection to all truth neutrinos.')
        self.addOption('isolated', True, type=bool,
                       info='select only truth neutrinos that are isolated.')
        self.addOption('notFromTau', True, type=bool,
                       info='select only truth neutrinos that did not orginate '
                       'from a tau decay.')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # decorate the missing elements of the 4-vector so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelPtEtaPhiDecoratorAlg',
                                     'ParticleLevelPtEtaPhiEDecoratorNeutrinos' + self.selectionName,
                                     reentrant=True)
        alg.particles    = self.containerName

        # check for prompt isolation and possible origin from tau decays
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg',
                                     'ParticleLevelIsolationNeutrinos' + self.selectionName,
                                     reentrant=True)
        alg.particles    = self.containerName
        alg.isolation    = 'isIsolated' + self.selectionName if self.isolated else 'isIsolatedButNotRequired' + self.selectionName
        alg.notTauOrigin = 'notFromTau' + self.selectionName if self.notFromTau else 'notFromTauButNotRequired' + self.selectionName
        alg.checkType    = 'Neutrino'

        if self.isolated:
            config.addSelection (self.containerName, self.selectionName, alg.isolation+',as_char')
        if self.notFromTau:
            config.addSelection (self.containerName, self.selectionName, alg.notTauOrigin+',as_char')

        # output branches to be scheduled only once
        if ParticleLevelNeutrinosBlock.get_instance_count() == 1 or 'pt' not in config.getOutputVars(self.containerName):
            outputVars = [
                ['pt', 'pt'],
                ['eta', 'eta'],
                ['phi', 'phi'],
                ['e', 'e'],
            ]
            for decoration, branch in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True)
