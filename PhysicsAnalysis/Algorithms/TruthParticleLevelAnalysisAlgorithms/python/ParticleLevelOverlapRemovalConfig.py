# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class ParticleLevelOverlapRemovalBlock(ConfigBlock):
    """ConfigBlock for particle-level overlap removal"""

    def __init__(self):
        super(ParticleLevelOverlapRemovalBlock, self).__init__()
        self.addOption('jets', '', type=str,
                       info ='the name of the input truth jets container, in the format `container` or `container.selection`.')
        self.addOption('electrons', '', type=str,
                       info='the name of the input truth electrons container, in the format `container` or `container.selection`.')
        self.addOption('muons', '', type=str,
                       info='the name of the input truth muons container, in the format `container` or `container.selection`.')
        self.addOption('photons', '', type=str,
                       info='the name of the input truth photons container, in the format `container` or `container.selection`.')
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
        alg = config.createAlgorithm('CP::ParticleLevelOverlapRemovalAlg',
                                     'ParticleLevelOverlapRemoval',
                                     reentrant=True)
        alg.useDressedProperties = self.useDressedProperties
        alg.useRapidityForDeltaR = self.useRapidityForDeltaR
        alg.labelOR = self.label
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection (self.electrons)
            alg.doJetElectronOR = True
            selection = self.electrons.split(".")[1] if len(self.electrons.split(".")) == 2 else ''
            config.addSelection (self.electrons.split(".")[0], selection, alg.labelOR + ',as_char')
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection (self.muons)
            alg.doJetMuonOR = True
            selection = self.muons.split(".")[1] if len(self.muons.split(".")) == 2 else ''
            config.addSelection (self.muons.split(".")[0], selection, alg.labelOR + ',as_char')
        if self.photons:
            alg.photons, alg.photonSelection = config.readNameAndSelection (self.photons)
            alg.doJetPhotonOR = True
            selection = self.photons.split(".")[1] if len(self.photons.split(".")) == 2 else ''
            config.addSelection (self.photons.split(".")[0], selection, alg.labelOR + ',as_char')
        if self.jets:
            alg.jets, alg.jetSelection = config.readNameAndSelection (self.jets)
            selection = self.jets.split(".")[1] if len(self.jets.split(".")) == 2 else ''
            config.addSelection (self.jets.split(".")[0], selection, alg.labelOR + ',as_char')
        else:
            raise ValueError('Particle-level overlap removal needs the jet container to be run!')
