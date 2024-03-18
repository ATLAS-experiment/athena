# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


class FTagEventSFConfig(ConfigBlock):
    """the ConfigBlock for the event-level FTAG scale factor"""

    def __init__(self, containerName, selectionName):
        super(FTagEventSFConfig, self).__init__()
        self.addDependency('OverlapRemoval', required=True)
        self.containerName = containerName
        self.postfix = selectionName
        self.addOption('btagWP', "FixedCutBEff_77", type=str)
        self.addOption('btagger', "DL1r", type=str)

    def makeAlgs(self, config):

        selectionName = self.postfix
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_':
            postfix = '_' + postfix

        # Set up the per-event FTAG efficiency scale factor calculation algorithm
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg',
                                         'FTagEventScaleFactorAlg' + postfix)
            preselection = config.getFullSelection(self.containerName, '')
            alg.preselection = ((preselection + '&&' if preselection else '')
                                + 'ftag_kin_select_' + selectionName + ',as_char')
            alg.scaleFactorInputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.scaleFactorOutputDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.particles = config.readName(self.containerName)

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                                'weight_ftag_effSF_' + selectionName)


def makeFTagEventSFConfig(seq, containerName,
                          selectionName,
                          btagWP=None,
                          btagger=None):

    config = FTagEventSFConfig(containerName, selectionName)
    if btagWP is not None:
        config.setOptionValue('btagWP', btagWP)
    if btagger is not None:
        config.setOptionValue('btagger', btagger)
    seq.append(config)
