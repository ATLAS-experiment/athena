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

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # count the number of heavy-flavour jets for normalisation of e.g. V+HF samples
        if "AntiKt4" in self.containerName:
            alg = config.createAlgorithm('CP::ParticleLevelJetsAlg',
                                         'ParticleLevelJetsAlg',
                                         reentrant=True)
            alg.jets = self.containerName

        # decorate the energy so we can save it later
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'ParticleLevelEnergyDecorator' )
        alg.particles = self.containerName

        # For some decorations the type is not known at initialization
        # time, in which case we need to pass it in manually. We could
        # also pass in the type for all decorations, but is not
        # necessary and introduces the possibility of a type mismatch.
        outputVars = [
            ['pt', 'pt', None],
            ['eta', 'eta', None],
            ['phi', 'phi', None],
            ['e_%SYS%', 'e', None],
            ['GhostBHadronsFinalCount', 'nGhosts_bHadron', 'int'],
            ['GhostCHadronsFinalCount', 'nGhosts_cHadron', 'int'],
        ]
        
        if self.outputTruthLabelIDs:
            outputVars += [
                ['HadronConeExclTruthLabelID', 'HadronConeExclTruthLabelID', None],
                ['PartonTruthLabelID', 'PartonTruthLabelID', None],
            ]

        for decoration, branch, auxType in outputVars:
            config.addOutputVar (self.containerName, decoration, branch, noSys=True, auxType=auxType)

        if "AntiKt4" in self.containerName:
            config.addOutputVar('EventInfo', 'num_truth_bjets_nocuts', 'num_truth_bjets_nocuts', noSys=True)
            config.addOutputVar('EventInfo', 'num_truth_cjets_nocuts', 'num_truth_cjets_nocuts', noSys=True)
