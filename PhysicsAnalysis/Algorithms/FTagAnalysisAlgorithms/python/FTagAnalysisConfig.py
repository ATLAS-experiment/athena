# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib

class FTagConfig (ConfigBlock):
    """the ConfigBlock for the flavor tagging config"""

    def __init__ (self) :
        super (FTagConfig, self).__init__ ()
        self.setBlockName('FTag')
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as internally the string "
            "f'{btagger}_{btagWP}' is used.")
        self.addOption ('btagWP', "Continuous", type=str,
            info="the flavour tagging WP. The default is Continuous.")
        self.addOption ('btagger', "GN2v01", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v01. The default "
            "is GN2v01.")
        self.addOption('useCTagging', False, type=bool,
            info="whether the fixed WP refer to b-tagging or c-tagging. Set to 'True' "
            "for referring to c-tagging")
        self.addOption ('bTagCalibFile', None, type=str,
            info="calibration file for CDI")
        self.addOption('readFromBTaggingObject', True, type=bool,
            info="whether to read the b-tagging information from the BTagging object "
            "instead of the jet container. FTAG group has dropped BTagging object, all"
            "b-tagging related variables are attached to jet container. This only serves"
            "as a compatibility option for analysis that use old derivations.")
        self.addOption ('saveScores', '', type=str,
            info="whether or not to save the scores from the tagger. Set to 'True' "
            "to save only the overall score, or to 'All' to save also the per-flavour"
            "probabilities.")
        self.addOption ('saveCustomVariables', [], type=list,
            info="[Expert mode] additional variables to save from the b-tagging object associated "
            "to each jet. E.g. ['pb','pc','pu', 'ptau'] to replicate 'saveScores=All'.")

    def makeAlgs (self, config) :

        jetCollection = config.originalName (self.containerName)

        selectionName = self.selectionName
        if selectionName is None or selectionName == '' :
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_' :
            postfix = '_' + postfix

        # CDI file
        if self.bTagCalibFile is not None :
            bTagCalibFile = self.bTagCalibFile
        else:
            bTagCalibFile = getRecommendedBTagCalib(config.geometry())

        # Set up the ftag selection algorithm(s):
        if 'Continuous' in self.btagWP:
            alg = config.createAlgorithm( 'CP::BTaggingInformationDecoratorAlg', 'FTagInfoAlg' + postfix )
        else:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'FTagSelectionAlg' + postfix )

        config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
        alg.selectionTool.TaggerName = self.btagger
        alg.selectionTool.OperatingPoint = self.btagWP
        alg.selectionTool.JetAuthor = jetCollection
        alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
        alg.selectionTool.MinPt = 0.  # user in charge of imposing kinematic cuts for jets
        alg.selectionTool.readFromBTaggingObject = self.readFromBTaggingObject
        alg.selectionTool.useCTagging = self.useCTagging
        alg.preselection = config.getPreselection (self.containerName, selectionName)

        if 'Continuous' in self.btagWP:
            alg.quantileDecoration = 'ftag_quantile_' + selectionName
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, 'ftag_quantile_' + selectionName, selectionName + '_quantile', noSys=True)
        else:
            alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
            alg.particles = config.readName (self.containerName)
            config.addOutputVar (self.containerName, 'ftag_select_' + selectionName, selectionName + '_select', noSys=True)
            config.addSelection (self.containerName, selectionName, alg.selectionDecoration)

        # Save the b-tagging score
        if self.saveScores in ['True', 'All']:
            # Save the b-tagger weight
            alg = config.createAlgorithm('CP::BTaggingInformationDecoratorAlg', 'FTagInfoAlg_' + self.btagger)
            alg.jets = config.readName (self.containerName)
            alg.taggerWeightDecoration = f'{self.btagger}'
            alg.affectingSystematicsFilter = '.*' # only run it on nominal!
            config.addPrivateTool( 'selectionTool', 'BTaggingSelectionTool' )
            # Configure the b-tagging selection tool
            alg.selectionTool.TaggerName = self.btagger
            alg.selectionTool.OperatingPoint = 'Continuous'
            alg.selectionTool.JetAuthor = jetCollection
            alg.selectionTool.FlvTagCutDefinitionsFileName = bTagCalibFile
            alg.selectionTool.readFromBTaggingObject = self.readFromBTaggingObject
            alg.selectionTool.useCTagging = self.useCTagging
            alg.selectionTool.MinPt = 0.
            config.addOutputVar(self.containerName, alg.taggerWeightDecoration, alg.taggerWeightDecoration, noSys=True)

        # Save the per-flavour probabilities or additional custom variables
        if self.saveScores == 'All' or self.saveCustomVariables:
            variables = [f'{self.btagger}_{x}' for x in ['pb','pc','pu','ptau'] if x != 'ptau' or self.btagger == 'GN2v01']
            variables += self.saveCustomVariables

            if self.readFromBTaggingObject:
                alg = config.createAlgorithm('CP::BTaggingScoresAlg',
                                             'BTagScoringAlg_' + self.btagger,
                                             reentrant=True)
                alg.jets = config.readName (self.containerName).replace('%SYS%', 'NOSYS')
                alg.taggerName = self.btagger
                alg.vars = variables

            for var in variables:
                config.addOutputVar(self.containerName, var, var, noSys=True)
