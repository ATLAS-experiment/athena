# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelOverlapRemovalBlock(ConfigBlock):
    """ConfigBlock for particle-level overlap removal"""

    def __init__(self):
        super(ParticleLevelOverlapRemovalBlock, self).__init__()
        self.addOption('jets', '', type=str,
                       info ='the name of the input truth jets container, in the format `container` or `container.selection`.',
                       meta={'role':'containerRef'})
        self.addOption('electrons', '', type=str,
                       info='the name of the input truth electrons container, in the format `container` or `container.selection`.',
                       meta={'role':'containerRef'})
        self.addOption('muons', '', type=str,
                       info='the name of the input truth muons container, in the format `container` or `container.selection`.',
                       meta={'role':'containerRef'})
        self.addOption('photons', '', type=str,
                       info='the name of the input truth photons container, in the format `container` or `container.selection`.',
                       meta={'role':'containerRef'})
        self.addOption('label', 'passesOR', type=str,
                       info='the name of the decoration to apply to all particles passing OR.')
        self.addOption('useDressedProperties', True, type=bool,
                       info='whether to use dressed electron and muon kinematics rather than simple 4-vector kinematics.')
        self.addOption('useRapidityForDeltaR', True, type=bool,
                       info=r'whether to use rapidity instead of pseudo-rapidity for the calculation of $\Delta R$.')
        # Always skip on data
        self.setOptionValue('skipOnData', True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.label

    def makeAlgs(self, config):
        if not self.jets:
            raise ValueError('Particle-level overlap removal needs the jet container to be run!')

        alg = config.createAlgorithm('CP::ParticleLevelOverlapRemovalAlg',
                                     'ParticleLevelOverlapRemoval',
                                     reentrant=True)
        alg.useDressedProperties = self.useDressedProperties
        alg.useRapidityForDeltaR = self.useRapidityForDeltaR
        alg.labelOR = self.label
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection (self.electrons)
            alg.doJetElectronOR = True
            container, _, selection = self.electrons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection (self.muons)
            alg.doJetMuonOR = True
            container, _, selection = self.muons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        if self.photons:
            alg.photons, alg.photonSelection = config.readNameAndSelection (self.photons)
            alg.doJetPhotonOR = True
            container, _, selection = self.photons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        alg.jets, alg.jetSelection = config.readNameAndSelection (self.jets)
        container, _, selection = self.jets.partition(".")
        config.addSelection (container, selection, alg.labelOR + ',as_char')
