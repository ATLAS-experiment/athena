# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib
from CalibrationDataInterface.CDIHelpers import check_CDI_campaign
from CalibrationDataInterface.MCMCGeneratorHelper import MCMC_dsid_map


class FTagEventSFConfig(ConfigBlock):
    """the ConfigBlock for the FTAG scale factor"""

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
        self.addOption ('btagWP', "Continuous", type=str,
            info="the flavour tagging WP. The default is Continuous.")
        self.addOption('btagger', "GN2v01", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v01. The default is GN2v01.")
        self.addOption ('bTagCalibFile', None, type=str,
            info="calibration file for CDI")
        self.addOption ('generator', "autoconfig", type=str,
            info="MC generator setup, for MC/MC SFs. The default is 'autoconfig'"
            " (relies on the sample metadata).")
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
        self.addOption ('savePerJetSF', False, type=bool,
            info="whether or not to save the per jet FTAG SF as output variable")

    def makeAlgs(self, config):

        if config.dataType() is DataType.Data: return

        if 'FixedCutBEff' in self.btagWP:
            raise ValueError('FTAG calibration is only available for Continuous WP. '
                             'Please configure the Continuous btagWP to retrieve scale factors.')

        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_':
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

        # Need to split container name from selections, to support AnaJets.baselineJvt
        jetContainer = self.containerName.split('.')[0]

        # Set up the efficiency calculation algorithm:
        alg = config.createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                                      'FTagEfficiencyScaleFactorAlg' + postfix )
        config.addPrivateTool( 'efficiencyTool', 'BTaggingEfficiencyTool' )
        alg.efficiencyTool.TaggerName = self.btagger
        alg.efficiencyTool.OperatingPoint = self.btagWP
        alg.efficiencyTool.JetAuthor = config.originalName(jetContainer)
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
        alg.onlyEfficiency = True
        alg.outOfValidity = 2  # continue silently, but decorate jet with outOfValidityDeco
        alg.outOfValidityDeco = 'no_ftag_' + selectionName + ',as_char'
        alg.preselection = config.getPreselection (jetContainer, selectionName)
        alg.jets = config.readName(jetContainer)
        if(self.savePerJetSF):
            config.addOutputVar (jetContainer, alg.scaleFactorDecoration,
                                 selectionName + '_eff')

        # Set up the per-event FTAG efficiency scale factor calculation algorithm
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
