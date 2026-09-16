# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelResonancesBlock(ConfigBlock):
    """ConfigBlock for particle-level truth resonances, i.e. unstable objects that """
    """would NOT be used to build a detector-level final state. This includes """
    """TruthBoson, TruthBottom, TruthTop, TruthBSM. Provides only basic functionalities, """
    """like storing the four-vector information, applying pT/eta selections, and thinning."""

    def __init__(self):
        super(ParticleLevelResonancesBlock, self).__init__()
        self.addOption('containerName', '', type=str,
                       info='the name of the input truth container. Supported options: `TruthBoson`, `TruthBottom`, `TruthTop`, `TruthBSM`.',
                       meta={'role':'container','choices':(['TruthBoson','TruthBottom','TruthTop','TruthBSM'],1)})
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):
        config.setSourceName (self.containerName, self.containerName)

        # decorate the missing elements of the 4-vector so we can save it later
        alg = config.createAlgorithm('CP::ParticleLevelPtEtaPhiDecoratorAlg',
                                     'ParticleLevelPtEtaPhiEDecoratorResonances',
                                     reentrant=True)
        alg.particles    = self.containerName

        outputVars = [
            ['pt', 'pt', 'float'],
            ['eta', 'eta', 'float'],
            ['phi', 'phi', 'float'],
            ['m', 'm', 'float'],
            ['classifierParticleType', 'type', 'unsigned'],
            ['classifierParticleOrigin', 'origin', 'unsigned'],
            ['pdgId', 'pdgId', 'int'],
            ['status', 'status', 'int'],
        ]
        for decoration, branch, auxType in outputVars:
            config.addOutputVar (self.containerName, decoration, branch, noSys=True, auxType=auxType)
