# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelJetsBlock(ConfigBlock):
    """ConfigBlock for particle-level truth jets"""

    def __init__(self):
        super(ParticleLevelJetsBlock, self).__init__()
        self.addOption('containerName', 'AntiKt4TruthDressedWZJets', type=str,
                       info='the name of the input truth jets container')
        self.addOption('outputTruthLabelIDs', False, type=bool,
                       info='Enable or disable HadronConeExclTruthLabelID and PartonTruthLabelID decorations')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # count the number of heavy-flavour jets for normalisation of e.g. V+HF samples
        if "AntiKt4" in self.containerName:
            alg = config.createAlgorithm('CP::ParticleLevelJetsAlg',
                                         'ParticleLevelJetsAlg' + self.containerName,
                                         reentrant=True)
            alg.jets = self.containerName

        # decorate the energy so we can save it later
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'ParticleLevelEnergyDecorator' + self.containerName )
        alg.particles = self.containerName

        outputVars = [
            ['pt', 'pt'],
            ['eta', 'eta'],
            ['phi', 'phi'],
            ['e_%SYS%', 'e'],
            ['GhostBHadronsFinalCount', 'nGhosts_bHadron'],
            ['GhostCHadronsFinalCount', 'nGhosts_cHadron'],
        ]
        
        if self.outputTruthLabelIDs:
            outputVars += [
                ['HadronConeExclTruthLabelID', 'HadronConeExclTruthLabelID'],
                ['PartonTruthLabelID', 'PartonTruthLabelID'],
            ]

        for decoration, branch in outputVars:
            config.addOutputVar (self.containerName, decoration, branch, noSys=True)

        if "AntiKt4" in self.containerName:
            config.addOutputVar('EventInfo', 'num_truth_bjets_nocuts', 'num_truth_bjets_nocuts', noSys=True)
            config.addOutputVar('EventInfo', 'num_truth_cjets_nocuts', 'num_truth_cjets_nocuts', noSys=True)
