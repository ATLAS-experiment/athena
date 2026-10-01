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
            electronsName, alg.electronSelection = config.readNameAndSelection (self.electrons)
            alg.electrons = electronsName
            alg.doJetElectronOR = True
            alg.decORelectron = f'{electronsName}.{self.label}'
            if self.useDressedProperties:
                alg.ptDressedElectron = f'{electronsName}.pt_dressed'
                alg.etaDressedElectron = f'{electronsName}.eta_dressed'
                alg.phiDressedElectron = f'{electronsName}.phi_dressed'
                alg.eDressedElectron = f'{electronsName}.e_dressed'
            container, _, selection = self.electrons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        if self.muons:
            muonsName, alg.muonSelection = config.readNameAndSelection (self.muons)
            alg.muons = muonsName
            alg.doJetMuonOR = True
            alg.decORmuon = f'{muonsName}.{self.label}'
            if self.useDressedProperties:
                alg.ptDressedMuon = f'{muonsName}.pt_dressed'
                alg.etaDressedMuon = f'{muonsName}.eta_dressed'
                alg.phiDressedMuon = f'{muonsName}.phi_dressed'
                alg.eDressedMuon = f'{muonsName}.e_dressed'
            container, _, selection = self.muons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        if self.photons:
            photonsName, alg.photonSelection = config.readNameAndSelection (self.photons)
            alg.photons = photonsName
            alg.doJetPhotonOR = True
            alg.decORphoton = f'{photonsName}.{self.label}'
            container, _, selection = self.photons.partition(".")
            config.addSelection (container, selection, alg.labelOR + ',as_char')
        jetsName, alg.jetSelection = config.readNameAndSelection (self.jets)
        alg.jets = jetsName
        alg.decORjet = f'{jetsName}.{self.label}'
        container, _, selection = self.jets.partition(".")
        config.addSelection (container, selection, alg.labelOR + ',as_char')

