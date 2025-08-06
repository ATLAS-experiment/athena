# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.Logging import logging

class VGammaORBlock(ConfigBlock):

    def __init__(self):
        super(VGammaORBlock, self).__init__()
        self.addOption('dR_lepton_photon_cuts', [0.0, 0.05, 0.075, 0.1, 0.125, 0.15, 0.2], type=list,
                       info='list of cuts on deltaR between the leptons and the photon.')
        self.addOption('photon_pT_cuts', [10e3], type=list,
                       info='list of pT cuts (in MeV) on the photon.')
        self.addOption('noFilter', False, type=bool,
                       info='do not apply an event filter. The default is False, i.e. remove events not passing the overlap removal. If set to True, all events are kept and the decision flag is written to the output ntuple instead. ')
        self.addOption('keepInOverlap', [700011, 700012, 700013, 700014, 700015, 700016, 700017], type=list,
                       info='list of DSIDs (integers) for which events are to be kept if found to be in the overlap region. For instance, Vy samples in V+jets vs Vy+jets overlap removal. The default list was taken from the PmgWeakBosonProcesses twiki but is not actively maintained!')
        self.addOption('removeInOverlap', [700320, 700321, 700322, 700467, 700468, 700469, 700323, 700324, 700325, 700470, 700471, 700472, 700326, 700327, 700328, 700329, 700330, 700331, 700332, 700333, 700334, 700473, 700474, 700475, 700476, 700477, 700478, 700479, 700480, 700481, 700341, 700342, 700343, 700338, 700339, 700340, 700344, 700345, 700346, 700347, 700348, 700349, 700598, 700599, 700439, 700440, 700441], type=list,
                       info='list of DSIDs (integers) for which events are to be remoevd if found to be in the overlap region. For instance, V samples in V+jets vs Vy+jets overlap removal. The default list was taken from the PmgWeakBosonProcesses twiki but is not actively maintained!')

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # not sure if this is a singleton block, but I don't know what name to use if not

    def makeAlgs(self, config):

        log = logging.getLogger('VGammaORBlock')

        if config.dataType() is DataType.Data: return
        if config.dsid() not in self.keepInOverlap and config.dsid() not in self.removeInOverlap:
            log.warning(f"CP::VGammaORAlg --> this sample has DSID {config.dsid()}, which is not set up for overlap removal. Will skip the configuration of the algorithm!")
            return

        alg = config.createAlgorithm('CP::VGammaORAlg', 'VGammaORAlg')
        alg.affectingSystematicsFilter = '.*'
        alg.noFilter = self.noFilter
        alg.FilterDescription = 'events passing V/VGamma overlap removal'
        alg.eventDecisionOutputDecoration = 'ignore_vgammaor_%SYS%'

        if config.dsid() in self.keepInOverlap:
            alg.keepOverlap = True
        elif config.dsid() in self.removeInOverlap:
            alg.keepOverlap = False

        config.addPrivateTool('VGammaORTool', 'VGammaORTool')
        alg.VGammaORTool.dR_lepton_photon_cuts = self.dR_lepton_photon_cuts
        alg.VGammaORTool.photon_pT_cuts = self.photon_pT_cuts

        if self.noFilter:
            # if we don't apply the filter, we still want to study the output of the tool!
            config.addOutputVar('EventInfo', 'in_vgamma_overlap_%SYS%', 'in_vgamma_overlap', noSys=True)
