# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib
from CalibrationDataInterface.CDIHelpers import check_CDI_campaign
from CalibrationDataInterface.MCMCGeneratorHelper import MCMC_dsid_map

class FTagConfig (ConfigBlock):
    """the ConfigBlock for the flavor tagging config"""

    def __init__ (self, containerName='', selectionName='') :
        super (FTagConfig, self).__init__ ()
        self.setBlockName('FTag')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', selectionName, type=str,
            noneAction='error',
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as internally the string "
            "f'{btagger}_{btagWP}' is used.")
        self.addOption ('btagWP', "Continuous", type=str,
            info="the flavour tagging WP. The default is Continuous.")
        self.addOption ('btagger', "GN2v01", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v01. The default "
            "is GN2v01.")
        self.addOption ('generator', "autoconfig", type=str,
            info="MC generator setup, for MC/MC SFs. The default is 'autoconfig'"
            " (relies on the sample metadata).")
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale factors "
            "are not available. The default is False.")
        self.addOption ('bTagCalibFile', None, type=str,
            info="calibration file for CDI")
        self.addOption ('systematicsStrategy', 'SFEigen', type=str,
            info="name of systematics model; presently choose between 'SFEigen' "
            "and 'Envelope'")
        self.addOption ('eigenvectorReductionB', 'Loose', type=str,
            info="b-jet scale factor Eigenvector reduction strategy; choose between "
            "'Loose', 'Medium', 'Tight'")
        self.addOption ('eigenvectorReductionC', 'Loose', type=str,
            info="b-jet scale factor Eigenvector reduction strategy; choose between "
            "'Loose', 'Medium', 'Tight'")
        self.addOption ('eigenvectorReductionLight', 'Loose', type=str,
            info="b-jet scale factor Eigenvector reduction strategy; choose between "
            "'Loose', 'Medium', 'Tight'")
        self.addOption ('excludeFromEigenVectorTreatment', '', type=str,
            info="(semicolon-separated) names of uncertainties to be excluded from "
            "all eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorBTreatment', '', type=str,
            info="(semicolon-separated) names of uncertainties to be excluded from "
            "b-jet eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorCTreatment', '', type=str,
            info="(semicolon-separated) names of uncertainties to be excluded from "
            "c-jet eigenvector decompositions (if used)")
        self.addOption ('excludeFromEigenVectorLightTreatment', '', type=str,
            info="(semicolon-separated) names of uncertainties to be excluded from "
            "light-flavour-jet eigenvector decompositions (if used)")
        self.addOption ('excludeRecommendedFromEigenVectorTreatment', False, type=str,
            info="whether or not to add recommended lists to the user specified "
            "eigenvector decomposition exclusion lists")
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

        DSID = "default"
        if config.dataType() is not DataType.Data:
            # Check if the right CDI is used for the MC campaign
            check_CDI_campaign(config.campaign(), bTagCalibFile)
            # MC/MC efficiency map for the generator 
            DSID = MCMC_dsid_map(config.geometry(), config.generatorInfo(), self.generator, self.btagger)

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

        if not self.noEffSF and config.dataType() is not DataType.Data:
            if 'FixedCutBEff' in self.btagWP:
                raise ValueError('FTAG calibration is only available for Continuous WP. '
                                 'Please configure the Continuous btagWP in addition to the FixedCutBEff one to retrieve scale factors')

            # Set up the efficiency calculation algorithm:
            alg = config.createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                                          'FTagEfficiencyScaleFactorAlg' + postfix )
            config.addPrivateTool( 'efficiencyTool', 'BTaggingEfficiencyTool' )
            alg.efficiencyTool.TaggerName = self.btagger
            alg.efficiencyTool.OperatingPoint = self.btagWP
            alg.efficiencyTool.JetAuthor = jetCollection
            alg.efficiencyTool.MinPt = 0.  # user in charge of imposing kinematic cuts for jets
            alg.efficiencyTool.EfficiencyFileName = bTagCalibFile
            alg.efficiencyTool.ScaleFactorFileName = bTagCalibFile
            alg.efficiencyTool.SystematicsStrategy = self.systematicsStrategy
            if self.systematicsStrategy == "SFEigen":
                alg.efficiencyTool.EigenvectorReductionB = self.eigenvectorReductionB
                alg.efficiencyTool.EigenvectorReductionC = self.eigenvectorReductionC
                alg.efficiencyTool.EigenvectorReductionLight = self.eigenvectorReductionLight
                alg.efficiencyTool.ExcludeFromEigenVectorTreatment = self.excludeFromEigenVectorTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorBTreatment = self.excludeFromEigenVectorBTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorCTreatment = self.excludeFromEigenVectorCTreatment
                alg.efficiencyTool.ExcludeFromEigenVectorLightTreatment = self.excludeFromEigenVectorLightTreatment
                alg.efficiencyTool.ExcludeRecommendedFromEigenVectorTreatment = self.excludeRecommendedFromEigenVectorTreatment
            if DSID != "default":
                alg.efficiencyTool.EfficiencyBCalibrations = DSID
                alg.efficiencyTool.EfficiencyTCalibrations = DSID
                alg.efficiencyTool.EfficiencyCCalibrations = DSID
                alg.efficiencyTool.EfficiencyLightCalibrations = DSID
            alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
            alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
            alg.onlyEfficiency = 'Continuous' in self.btagWP
            alg.outOfValidity = 2  # continue silently, but decorate jet with outOfValidityDeco
            alg.outOfValidityDeco = 'no_ftag_' + selectionName + ',as_char'
            alg.preselection = config.getPreselection (self.containerName, selectionName)
            alg.jets = config.readName (self.containerName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, selectionName + '_eff')

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
            alg.selectionTool.MinPt = 0.
            config.addOutputVar(self.containerName, alg.taggerWeightDecoration, alg.taggerWeightDecoration, noSys=True)

        # Save the per-flavour probabilities or additional custom variables
        if self.saveScores == 'All' or self.saveCustomVariables:
            alg = config.createAlgorithm('CP::BTaggingScoresAlg', 'BTagScoringAlg_' + self.btagger)
            alg.jets = config.readName (self.containerName).replace('%SYS%', 'NOSYS')
            alg.taggerName = self.btagger

            variables = [f'{self.btagger}_{x}' for x in ['pb','pc','pu','ptau'] if x != 'ptau' or self.btagger == 'GN2v01']
            variables += self.saveCustomVariables

            alg.vars = variables
            for var in variables:
                config.addOutputVar(self.containerName, var, var, noSys=True)

def makeFTagAnalysisConfig( seq, containerName,
                            selectionName,
                            btagWP = None,
                            btagger = None,
                            generator = None,
                            noEffSF = None ):
    """Create a ftag analysis algorithm config

    Keyword arguments:
      btagWP -- Flavour tagging working point
      btagger -- Flavour tagger
      generator -- Generator for MC/MC scale factors
      noEffSF -- Disables efficiency and scale factor calculations
    """

    config = FTagConfig (containerName, selectionName)
    config.setOptionValue ('btagWP', btagWP)
    config.setOptionValue ('btagger', btagger)
    config.setOptionValue ('generator', generator)
    config.setOptionValue ('noEffSF', noEffSF)
    seq.append (config)
