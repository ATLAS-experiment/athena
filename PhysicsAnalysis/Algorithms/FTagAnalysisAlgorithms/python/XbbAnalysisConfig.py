# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

class XbbConfig (ConfigBlock):
    """the ConfigBlock for the Xbb tagging config"""
    def __init__ (self, containerName=''):
        super (XbbConfig, self).__init__ ()
        self.setBlockName('Xbb')
        self.addOption ('containerName', containerName, type=str,
                        noneAction='error',
                        info="the name of the input container.")
        self.addOption('XbbWP', 'FlatMassQCDEff_0p25', type=str,
                       info="the Xbb tagging WP." )
        self.addOption('Xbbtagger', 'GN2Xv01', type=str,
                       info="the Xbb tagger: GN2Xv01. The default is GN2Xv01")
        self.addOption('calibFile', None, type=str,
                       info="the calibration json file")
        self.addOption('noEffSF', False, type=bool,
                       info="do not apply the eff SF")

    def makeAlgs(self, config):

        jetContainer = config.originalName(self.containerName)
        selectionName = f'{self.Xbbtagger}_{self.XbbWP}'

        postfix = self.XbbWP

        alg = config.createAlgorithm( 'CP::XbbInformationDecoratorAlg', 'XbbInfoAlg' + postfix )
        config.addPrivateTool('selectionTool', 'BTaggingSelectionJsonTool' )
        alg.selectionTool.MaxEta = 4.5
        alg.selectionTool.MinPt = 0.
        alg.selectionTool.TaggerName = self.Xbbtagger
        alg.selectionTool.JetAuthor = jetContainer
        alg.selectionTool.OperatingPoint = self.XbbWP
        alg.selectionTool.JsonConfigFile = self.calibFile
        alg.preselection = config.getPreselection (self.containerName, '')

        alg.taggerDecisionDecoration = 'xbb_select_' + selectionName
        alg.jets = config.readName (self.containerName)
        config.addOutputVar (self.containerName, 'xbb_select_' + selectionName, selectionName + '_select', noSys=True)

        if not self.noEffSF and config.dataType() != DataType.Data:
            alg = config.createAlgorithm( 'CP::XbbEfficiencyAlg',
                                         'XbbSFAlg' + postfix )
            config.addPrivateTool('efficiencyTool', 'BTaggingEfficiencyJsonTool' )
            alg.efficiencyTool.MaxEta = 4.5
            alg.efficiencyTool.MinPt = 0.
            alg.efficiencyTool.TaggerName = self.Xbbtagger
            alg.efficiencyTool.JetAuthor = jetContainer
            alg.efficiencyTool.OperatingPoint = self.XbbWP
            alg.efficiencyTool.JsonConfigFile = self.calibFile
            alg.preselection = config.getPreselection (self.containerName, '')

            alg.scaleFactorDecoration = 'xbb_effSF_' + selectionName + '_%SYS%'
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, selectionName + '_effSF')
