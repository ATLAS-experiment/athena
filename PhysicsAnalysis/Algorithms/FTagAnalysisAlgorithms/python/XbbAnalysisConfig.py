# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from CalibrationDataInterface.MCMCGeneratorHelper import MCMC_dsid_map

class XbbConfig (ConfigBlock):
    """the ConfigBlock for the Xbb tagging config"""
    def __init__ (self):
        super (XbbConfig, self).__init__ ()
        self.setBlockName('Xbb')
        self.addOption ('containerName', '', type=str,
                        noneAction='error',
                        info="the name of the input container.",
                        meta={'role':'containerRef'})
        self.addOption('XbbWP', 'FlatMassQCDEff_0p25', type=str,
                       info="the Xbb tagging WP." )
        self.addOption('Xbbtagger', 'GN2Xv01', type=str,
                       info="the output name of the Xbb tagger - should match the first level key of the calibration json file." )
        self.addOption('calibFile', None, type=str,
                       info="path to a Xbb JSON calibration file.")
        self.addOption ('generator', "autoconfig", type=str,
            info="MC generator setup, for MC/MC SFs. The default is `autoconfig`"
            " (relies on the sample metadata).")
        self.addOption('noEffSF', False, type=bool,
                       info="do not compute the efficiency SF.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + '_' + self.Xbbtagger + '_' + self.XbbWP

    def makeAlgs(self, config):

        jetContainer = config.originalName(self.containerName)
        selectionName = f'{self.Xbbtagger}_{self.XbbWP}'
        
        # Create selection tool as public so it can be shared as the efficiency tool also needs the selection tool
        # Must be created before the algorithm so the tool is initialized first
        btagSelectionTool = config.createPublicTool( "BTaggingSelectionJsonTool", f"XbbLargeRJetSelectionJsonTool_{jetContainer}_{selectionName}")
        # Get type/name tool as this is what is passed to initialize tool for AnalysisBase 
        btagSelectionToolTypeName = f"{btagSelectionTool.getType()}/{btagSelectionTool.getName()}"
        
        btagSelectionTool.MaxEta = 4.5
        btagSelectionTool.MinPt = 0.
        btagSelectionTool.OutputName = self.Xbbtagger
        btagSelectionTool.JetAuthor = jetContainer
        btagSelectionTool.OperatingPoint = self.XbbWP
        btagSelectionTool.JsonConfigFile = self.calibFile
        
        alg = config.createAlgorithm( 'CP::XbbInformationDecoratorAlg', 'XbbInfoAlg' )
        alg.selectionTool = btagSelectionToolTypeName
        alg.preselection = config.getPreselection (self.containerName, '')

        alg.taggerDecisionDecoration = 'xbb_select_' + selectionName
        alg.jets = config.readName (self.containerName)
        config.addOutputVar (self.containerName, 'xbb_select_' + selectionName, selectionName + '_select', noSys=True, auxType="int")

        if not self.noEffSF and config.dataType() != DataType.Data:
            alg = config.createAlgorithm( 'CP::XbbEfficiencyAlg',
                                         'XbbSFAlg' )
            config.addPrivateTool('efficiencyTool', 'BTaggingEfficiencyJsonTool' )
            alg.efficiencyTool.MaxEta = 4.5
            alg.efficiencyTool.MinPt = 0.
            alg.efficiencyTool.OutputName = self.Xbbtagger
            alg.efficiencyTool.JetAuthor = jetContainer
            alg.efficiencyTool.OperatingPoint = self.XbbWP
            alg.efficiencyTool.JsonConfigFile = self.calibFile
            
            # MC/MC efficiency map for the generator
            mcGenerator = MCMC_dsid_map(config.geometry(), config.generatorInfo(), self.generator, self.Xbbtagger)
            alg.efficiencyTool.MCGenerator = mcGenerator
            
            # Provide the selection tool to the efficiency tool
            alg.efficiencyTool.btagSelectionJsonTool = btagSelectionToolTypeName
            
            alg.preselection = config.getPreselection (self.containerName, '')
            alg.outOfValidity = 2  # continue silently, but decorate jet with outOfValidityDeco
            alg.outOfValidityDeco = 'no_xbb_' + selectionName + ',as_char'

            alg.scaleFactorDecoration = 'xbb_effSF_' + selectionName + '_%SYS%'
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, selectionName + '_effSF', auxType="float")
