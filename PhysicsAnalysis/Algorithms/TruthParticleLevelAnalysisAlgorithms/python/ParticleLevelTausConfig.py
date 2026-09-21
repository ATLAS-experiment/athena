# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelTausBlock(ConfigBlock):
    """ConfigBlock for particle-level truth taus"""

    def __init__(self):
        super(ParticleLevelTausBlock, self).__init__()
        self.addOption('containerName', 'TruthTaus', type=str,
                       info='the name of the input truth taus container.',
                       meta={'role':'container'})
        self.addOption('selectionName', '', type=str,
                       info='the name of the selection to create. If left empty, '
                       'applies the selection to all truth taus.',
                       meta={'role':'selection'})
        self.addOption('isolated', True, type=bool,
                       info='select only truth taus that are isolated.')
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
                                     'ParticleLevelPtEtaPhiDecoratorTaus',
                                     reentrant=True)
        alg.particles = self.containerName

        # decorate the charge so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelChargeDecoratorAlg',
                                     'ParticleLevelChargeDecoratorTaus',
                                     reentrant=True)
        alg.particles = self.containerName

        # check for prompt isolation and possible origin from tau decays
        alg = config.createAlgorithm('CP::ParticleLevelIsolationAlg',
                                     'ParticleLevelIsolationTaus',
                                     reentrant=True)
        alg.particles    = self.containerName
        alg.isolation    = 'isIsolated' + self.selectionName if self.isolated else 'isIsolatedButNotRequired' + self.selectionName
        alg.notTauOrigin = 'notFromTauButNotRequired' + self.selectionName
        alg.checkType    = 'IsoTau'

        if self.isolated:
            config.addSelection (self.containerName, self.selectionName, alg.isolation+',as_char')

        # output branches to be scheduled only once
        if ParticleLevelTausBlock.get_instance_count() == 1 or 'pt' not in config.getOutputVars(self.containerName):
            outputVars = [
                ['pt', 'pt', 'float'],
                ['eta', 'eta', 'float'],
                ['phi', 'phi', 'float'],
                ['e', 'e', 'float'],
                ['charge', 'charge', 'float'],
                ['IsHadronicTau', 'IsHadronicTau', 'char'],
                ['pdgId', 'pdgId', 'int'],
                ['numCharged', 'numCharged', 'unsigned_long'],                
                ['classifierParticleType', 'type', 'unsigned'],
                ['classifierParticleOrigin', 'origin', 'unsigned'],
                ['pt_vis', 'pt_vis', 'double'],
                ['eta_vis', 'eta_vis', 'double'],
                ['phi_vis', 'phi_vis', 'double'],
                ['m_vis', 'm_vis', 'double'],
                ['pt_vis_dressed', 'pt_vis_dressed', 'float'],
                ['eta_vis_dressed', 'eta_vis_dressed', 'float'],
                ['phi_vis_dressed', 'phi_vis_dressed', 'float'],
                ['m_vis_dressed', 'm_vis_dressed', 'float'],
                ['pt_invis', 'pt_invis', 'double'],
                ['eta_invis', 'eta_invis', 'double'],
                ['phi_invis', 'phi_invis', 'double'],
                ['m_invis', 'm_invis', 'double'],
                
            ]
            if self.saveUID:
                outputVars += [['uid', 'uid', 'int']]
            for decoration, branch, auxType in outputVars:
                config.addOutputVar (self.containerName, decoration, branch, noSys=True, auxType=auxType)
