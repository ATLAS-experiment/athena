# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class LeptonSFCalculatorBlock(ConfigBlock):
    """ConfigBlock for the common electron-muon-photon SF calculator"""
    """--> combine all the per-object SFs into a single per-event SF"""

    def __init__(self):
        super(LeptonSFCalculatorBlock, self).__init__()
        self.addOption('electrons', None, type=str,
                       info='the input electron container, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption('electronSFs', None, type=list,
                       info='list of decorated electron SFs to use in the computation. If not set, will use reconstruction x ID x isolation.')
        self.addOption('muons', None, type=str,
                       info='the input muon container, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption('muonSFs', None, type=list,
                       info='list of decorated muon SFs to use in the computation. If not set, will use reconstruction x isolation x TTVA.')
        self.addOption('photons', None, type=str,
                       info='the input photon container, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption('photonSFs', None, type=list,
                       info='list of decorated photon SFs to use in the computation. If not set, will use ID x isolation.')
        self.addOption('lepton_postfix', None, type=str,
                       info='the name of the common lepton SF, e.g. `tight`.')

    def makeAlgs(self, config):
        if config.dataType() is DataType.Data: return

        alg = config.createAlgorithm('CP::LeptonSFCalculatorAlg',
                                     f'leptonSFCalculator_{self.lepton_postfix}')

        if self.electrons:
            electrons, electronSelection = config.readNameAndSelection(self.electrons)
            alg.electrons                = electrons
            alg.electronSelection        = electronSelection
            if self.electronSFs:
                alg.electronSFs = self.electronSFs
            else:
                alg.electronSFs = [ f'el_reco_effSF_{self.electrons.split(".")[1]}_%SYS%',
                                    f'el_id_effSF_{self.electrons.split(".")[1]}_%SYS%' ]
                if 'isolated' in alg.electronSelection:
                    alg.electronSFs += [ f'el_isol_effSF_{self.electrons.split(".")[1]}_%SYS%' ]

        if self.muons:
            muons, muonSelection         = config.readNameAndSelection(self.muons)
            alg.muons                    = muons
            alg.muonSelection            = muonSelection
            if self.muonSFs:
                alg.muonSFs = self.muonSFs
            else:
                alg.muonSFs = [ f'muon_reco_effSF_{self.muons.split(".")[1]}_%SYS%']
                if 'trackSelection' in alg.muonSelection:
                    alg.muonSFs += [ f'muon_TTVA_effSF_{self.muons.split(".")[1]}_%SYS%' ]
                if 'isolated' in alg.muonSelection:
                    alg.muonSFs += [ f'muon_isol_effSF_{self.muons.split(".")[1]}_%SYS%' ]

        if self.photons:
            photons, photonSelection     = config.readNameAndSelection(self.photons)
            alg.photons                  = photons
            alg.photonSelection          = photonSelection
            if self.photonSFs:
                alg.photonSFs = self.photonSFs
            else:
                alg.photonSFs = [ f'ph_id_effSF_{self.photons.split(".")[1]}_%SYS%' ]
                if 'isolated' in alg.photonSelection:
                    alg.photonSFs += [ f'ph_isol_effSF_{self.photons.split(".")[1]}_%SYS%' ]

        alg.event_leptonSF = f'leptonSF_{self.lepton_postfix}_%SYS%'

        config.addOutputVar('EventInfo', alg.event_leptonSF, f'weight_leptonSF_{self.lepton_postfix}')
