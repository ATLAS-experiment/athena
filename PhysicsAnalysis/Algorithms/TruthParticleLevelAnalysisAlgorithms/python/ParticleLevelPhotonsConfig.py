# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelPhotonsBlock(ConfigBlock):
    """ConfigBlock for particle-level truth photons"""

    def __init__(self):
        super(ParticleLevelPhotonsBlock, self).__init__()
        self.addOption('containerName', 'TruthPhotons', type=str,
                       info='the name of the input truth photons container.')
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. If left empty, '
                       'applies the selection to all truth photons.')
        self.addOption('isolated', True, type=bool,
                       info='select only truth photons that are isolated.')
        self.addOption('isolationVariable', '', type=str,
                       info='variable to use in isolation cuts of the form `var/pT < cut`.')
        self.addOption('isolationCut', -1, type=float,
                       info='threshold to use in isolation cuts of the form `var/pT < cut`.')
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

        # decorate the missing elements of the 4-vector so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelPtEtaPhiDecoratorAlg',
                                     'ParticleLevelPtEtaPhiDecoratorPhotons',
                                     reentrant=True)
        alg.particles = self.containerName

        # check for prompt isolation
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg',
                                     'ParticleLevelIsolationPhotons',
                                     reentrant=True)
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
        if ParticleLevelPhotonsBlock.get_instance_count() == 1 or 'pt' not in config.getOutputVars(self.containerName):
            outputVars = [
                ['pt', 'pt', 'float'],
                ['eta', 'eta', 'float'],
                ['phi', 'phi', 'float'],
                ['e', 'e', 'float'],
                ['classifierParticleType', 'type', 'unsigned'],
                ['classifierParticleOrigin', 'origin', 'unsigned'],
            ]
            if self.saveUID:
                outputVars += [['uid', 'uid', 'int']]
            for decoration, branch, auxType in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True, auxType=auxType)
