# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelElectronsBlock(ConfigBlock):
    """ConfigBlock for particle-level truth electrons"""

    def __init__(self):
        super(ParticleLevelElectronsBlock, self).__init__()
        self.addOption('containerName', 'TruthElectrons', type=str,
                       info='the name of the input truth electrons container.')
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. If left empty, '
                       'applies the selection to all truth electrons.')
        self.addOption('isolated', True, type=bool,
                       info='select only truth electrons that are isolated.')
        self.addOption('notFromTau', True, type=bool,
                       info='select only truth electrons that did not orginate '
                       'from a tau-lepton decay.')
        self.addOption('saveUID', False, type=bool,
                       info='save unique ID in output.')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        name = self.containerName
        if self.selectionName: name = name + '_' + self.selectionName
        return name

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # decorate the charge so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelChargeDecoratorAlg',
                                     'ParticleLevelChargeDecoratorElectrons',
                                     reentrant=True)
        alg.particles = self.containerName

        # check for prompt isolation and possible origin from tau decays
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg',
                                     'ParticleLevelIsolationElectrons',
                                     reentrant=True)
        alg.particles    = self.containerName
        alg.isolation    = 'isIsolated' + self.selectionName if self.isolated else 'isIsolatedButNotRequired' + self.selectionName
        alg.notTauOrigin = 'notFromTau' + self.selectionName if self.notFromTau else 'notFromTauButNotRequired' + self.selectionName
        alg.checkType    = 'IsoElectron'

        if self.isolated:
            config.addSelection (self.containerName, self.selectionName, alg.isolation+',as_char')
        if self.notFromTau:
            config.addSelection (self.containerName, self.selectionName, alg.notTauOrigin+',as_char')

        # output branches to be scheduled only once
        if ParticleLevelElectronsBlock.get_instance_count() == 1 or 'pt' not in config.getOutputVars(self.containerName):
            outputVars = [
                ['pt_dressed', 'pt', 'float'],
                ['eta_dressed', 'eta', 'float'],
                ['phi_dressed', 'phi', 'float'],
                ['e_dressed', 'e', 'float'],
                ['charge', 'charge', 'float'],
                ['classifierParticleType', 'type', 'unsigned'],
                ['classifierParticleOrigin', 'origin', 'unsigned'],
            ]
            if self.saveUID:
                outputVars += [['uid', 'uid', 'int']]
            for decoration, branch, auxType in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True, auxType=auxType)
