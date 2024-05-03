# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class FTagEventSFConfig(ConfigBlock):
    """the ConfigBlock for the event-level FTAG scale factor"""

    def __init__(self, containerName='', selectionName=''):
        super(FTagEventSFConfig, self).__init__()
        self.addDependency('OverlapRemoval', required=False)
        self.addOption('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption('selectionName', selectionName, type=str,
            noneAction='error',
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as internally the string "
            "f'{btagger}_{btagWP}' is used.")
        self.addOption('btagWP', "FixedCutBEff_77", type=str,
            info="the flavour tagging WP. The default is FixedCutBEff_77.")
        self.addOption('btagger', "DL1r", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v00. The default is DL1r.")

    def makeAlgs(self, config):

        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_':
            postfix = '_' + postfix

        # Set up the per-event FTAG efficiency scale factor calculation algorithm
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg',
                                         'FTagEventScaleFactorAlg' + postfix)
            particles, preselection = config.readNameAndSelection(self.containerName)
            alg.particles = particles
            alg.preselection = ((preselection + '&&' if preselection else '')
                                + 'no_ftag_' + selectionName + ',as_char')
            alg.scaleFactorInputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.scaleFactorOutputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                                'weight_ftag_effSF_' + selectionName)


def makeFTagEventSFConfig(seq, containerName,
                          selectionName,
                          btagWP=None,
                          btagger=None):

    config = FTagEventSFConfig(containerName, selectionName)
    config.setOptionValue('btagWP', btagWP)
    config.setOptionValue('btagger', btagger)
    seq.append(config)
