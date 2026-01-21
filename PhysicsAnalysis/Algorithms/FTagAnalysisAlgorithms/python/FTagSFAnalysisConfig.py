# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.Logging import logging

from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib, getRecommendedBTagTrigCalib, getReadFromBTaggingObject
from CalibrationDataInterface.CDIHelpers import check_CDI_campaign
from CalibrationDataInterface.MCMCGeneratorHelper import MCMC_dsid_map
from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import trigger_set


def getBTagOnlineWP(chain, onlineTagger):
    # We have a chain with something like "..._bdl1d77_..."
    # Get the substring after the tagger, e.g. bdl1d
    after = chain.split(onlineTagger)[1]
    # Get the two first characters, corresponding to the WP
    wp = after[:2]
    return 'FixedCutBEff_'+wp

def getBTagOnlineTaggerWP(chain, log):
    bTagOnlineTaggers = {
        'bdl1d' : 'OnlineDL1d',
        'bgn1' : 'OnlineGN1' }

    for tag, bTagOnlineTag in bTagOnlineTaggers.items():
        if tag in chain:
            return (bTagOnlineTag, getBTagOnlineWP(chain, tag))

    return ('', '')


class FTagJetSFBlock(ConfigBlock):
    """the ConfigBlock for the FTAG scale factor per jet"""
    def __init__(self):
        super(FTagJetSFBlock, self).__init__()
        self.addOption('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption('selectionName', '', type=str,
            noneAction='error',
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as internally the string "
            "f'{btagger}_{btagWP}' is used.")
        self.addOption ('btagWP', "Continuous", type=str,
            info="the flavour tagging WP. The default is Continuous.")
        self.addOption('btagger', "GN2v01", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v01. The default is GN2v01.")
        self.addOption('useCTagging', False, type=bool,
            info="whether the fixed WP refer to b-tagging or c-tagging. Set to 'True' "
            "for referring to c-tagging")
        self.addOption ('bTagCalibFile', None, type=str,
            info="calibration file for CDI")
        self.addOption ('bTagCalibTriggerFile', None, type=str,
            info="trigger calibration file for CDI")
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
        self.addOption ('triggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
            info="if True, all configured years in the LHC run will be included in all jobs. "
            "The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
            info="remove the HLT prefix from trigger chain names, "
            "The default is True.")
        self.addOption ('bTagOnlineTagger', None, type=str,
            info="Online tagger to use to configure the CDI access",
            expertMode=True)
        self.addOption ('bTagOnlineWP', None, type=str,
            info="Online working point to use to configure the CDI access",
            expertMode=True)
        # Peculiar case default value set to None while type is bool 
        # A default value will be assigned by the getReadFromBTaggingObject function 
        # if this flag is not set 
        self.addOption('readFromBTaggingObject', None, type=bool,
            info="whether to read the b-tagging information from the BTagging object "
            "instead of the jet container. FTAG group has dropped BTagging object, all"
            "b-tagging related variables are attached to jet container. This only serves"
            "as a compatibility option for analysis that use old derivations.")

    def instanceName (self) :
        """Return the instance name for this block"""
        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP
        return self.containerName.replace('.', '_') + '_' + selectionName

    def configureEfficiencyTool(self, config, btagger, btagWP, jetContainer,
                                bTagCalibFile, DSID, tool,
                                selectionCDI="", selectionTagger=""):
        
        tool.TaggerName = btagger
        tool.OperatingPoint = btagWP
        tool.JetAuthor = config.originalName(jetContainer)
        tool.MinPt = 0.  # user in charge of imposing kinematic cuts for jets
        tool.EfficiencyFileName = bTagCalibFile
        tool.ScaleFactorFileName = bTagCalibFile
        tool.SystematicsStrategy = self.systematicsStrategy
        tool.useCTagging = self.useCTagging
        tool.readFromBTaggingObject = self.readFromBTaggingObject
        if self.systematicsStrategy == "SFEigen":
            tool.EigenvectorReductionB = self.eigenvectorReductionB
            tool.EigenvectorReductionC = self.eigenvectorReductionC
            tool.EigenvectorReductionLight = self.eigenvectorReductionLight
            tool.ExcludeFromEigenVectorTreatment = self.excludeFromEigenVectorTreatment
            tool.ExcludeFromEigenVectorBTreatment = self.excludeFromEigenVectorBTreatment
            tool.ExcludeFromEigenVectorCTreatment = self.excludeFromEigenVectorCTreatment
            tool.ExcludeFromEigenVectorLightTreatment = self.excludeFromEigenVectorLightTreatment
            tool.ExcludeRecommendedFromEigenVectorTreatment = self.excludeRecommendedFromEigenVectorTreatment
        if DSID != "default":
            tool.EfficiencyBCalibrations = DSID
            tool.EfficiencyTCalibrations = DSID
            tool.EfficiencyCCalibrations = DSID
            tool.EfficiencyLightCalibrations = DSID
        if selectionCDI:
            tool.SelectionCDIFileName = selectionCDI
        if selectionTagger:
            tool.SelectionTaggerName = selectionTagger

    def makeAlgs(self, config):
        log = logging.getLogger('FTagJetSFConfig')

        if config.dataType() is DataType.Data: return

        if config.isPhyslite() and self.triggerChainsPerYear:
            log.warning ('The b-jet trigger SF computation is currently not supported in PHYSLITE')
            return

        if 'FixedCutBEff' in self.btagWP:
            raise ValueError('FTAG calibration is only available for Continuous WP. '
                             'Please configure the Continuous btagWP to retrieve scale factors.')

        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

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
        
        jetCollection = config.originalName(jetContainer)
        # Potentially modify the readFromBTaggingObject as here determining 
        # if input files has jet tagging probabilities attached to the jet (or still only to the BTagging object)
        self.readFromBTaggingObject = getReadFromBTaggingObject(config.flags, jetCollection, self.readFromBTaggingObject)

        # b-jet trigger-aware SF
        if self.triggerChainsPerYear:
            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun)
            
            for chain in triggers:
                chain_noHLT = chain.replace("HLT_", "")
                chain_out = chain_noHLT if self.removeHLTPrefix else chain
                chain_out = chain_out.replace('-', '_').replace('.', 'p')

                if self.bTagCalibTriggerFile is not None :
                    bTagCalibTriggerFile = self.bTagCalibTriggerFile
                else:
                    bTagCalibTriggerFile = getRecommendedBTagTrigCalib(config.geometry())

                bTagOnlineTagger, bTagOnlineWP = getBTagOnlineTaggerWP(chain, log)
                if self.bTagOnlineTagger:
                    bTagOnlineTagger = self.bTagOnlineTagger
                if self.bTagOnlineWP:
                    bTagOnlineWP = self.bTagOnlineWP

                if not bTagOnlineTagger and not bTagOnlineWP:
                    raise ValueError('Trigger chain ' + chain + ' does not include any of the supported online taggers. '
                                     'Please make sure to configure manually bTagOnlineTagger and bTagOnlineWP')

                bTagConditionalTagger = "ConditionalOffline" + self.btagger + "Given" + bTagOnlineTagger + "WP" + bTagOnlineWP.split("_")[-1]
                bTagConditionalWP = self.btagWP

                alg = config.createAlgorithm( 'CP::BTaggingTriggerEfficiencyAlg',
                                              'FTagEfficiencyTriggerScaleFactorAlg' + chain )
                config.addPrivateTool( 'offlineEfficiencyTool',
                                       'BTaggingEfficiencyTool' )
                self.configureEfficiencyTool(
                    config, self.btagger, self.btagWP, jetContainer,
                    bTagCalibFile, DSID, alg.offlineEfficiencyTool)
                config.addPrivateTool( 'triggerEfficiencyTool',
                                       'BTaggingEfficiencyTool' )
                self.configureEfficiencyTool(
                    config, bTagOnlineTagger, bTagOnlineWP, jetContainer,
                    bTagCalibTriggerFile, DSID, alg.triggerEfficiencyTool)
                config.addPrivateTool( 'conditionalEfficiencyTool',
                                       'BTaggingEfficiencyTool' )
                self.configureEfficiencyTool(
                    config, bTagConditionalTagger, bTagConditionalWP, jetContainer,
                    bTagCalibTriggerFile, DSID, alg.conditionalEfficiencyTool,
                    selectionCDI=bTagCalibFile, selectionTagger=self.btagger)

                alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_' + chain_out + '_%SYS%'
                alg.matchingDecoration = 'ftag_jetTrigMatching_' + chain_out + '_%SYS%'
                alg.bTagMatchingDecoration = 'ftag_bTagTrigMatching_' + chain_out + '_%SYS%'
                alg.outOfValidity = 2  # continue silently, but decorate jet with outOfValidityDeco
                alg.outOfValidityDeco = 'no_ftag_' + selectionName + '_' + chain_out + ',as_char'
                alg.preselection = config.getPreselection (jetContainer, selectionName)
                alg.jets = config.readName(jetContainer)
                if(self.savePerJetSF):
                    config.addOutputVar (jetContainer, alg.scaleFactorDecoration,
                                         selectionName + '_' + chain_out + '_eff')

        # Set up the efficiency calculation algorithm:
        # Always compute regular FTAG SF
        alg = config.createAlgorithm( 'CP::BTaggingEfficiencyAlg',
                                      'FTagEfficiencyScaleFactorAlg' )
        config.addPrivateTool( 'efficiencyTool', 'BTaggingEfficiencyTool' )
        self.configureEfficiencyTool(
            config, self.btagger, self.btagWP, jetContainer,
            bTagCalibFile, DSID, alg.efficiencyTool)
        alg.onlyEfficiency = True

        alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_%SYS%'
        alg.selectionDecoration = 'ftag_select_' + selectionName + ',as_char'
        alg.outOfValidity = 2  # continue silently, but decorate jet with outOfValidityDeco
        alg.outOfValidityDeco = 'no_ftag_' + selectionName + ',as_char'
        alg.preselection = config.getPreselection (jetContainer, selectionName)
        alg.jets = config.readName(jetContainer)
        if(self.savePerJetSF):
            config.addOutputVar (jetContainer, alg.scaleFactorDecoration,
                                 selectionName + '_eff')


class FTagEventSFBlock(ConfigBlock):
    """the ConfigBlock for the event FTAG scale factor"""

    def __init__(self):
        super(FTagEventSFBlock, self).__init__()
        self.addDependency('OverlapRemoval', required=False)
        self.addOption('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption('selectionName', '', type=str,
            noneAction='error',
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as internally the string "
            "f'{btagger}_{btagWP}' is used.")
        self.addOption ('btagWP', "Continuous", type=str,
            info="the flavour tagging WP. The default is Continuous.")
        self.addOption('btagger', "GN2v01", type=str,
            info="the flavour tagging algorithm: DL1dv01, GN2v01. The default is GN2v01.")
        self.addOption ('triggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
            info="if True, all configured years in the LHC run will be included "
            "in all jobs. The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
            info="remove the HLT prefix from trigger chain names, "
            "The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP
        return self.containerName.replace('.', '_') + '_' + selectionName

    def makeAlgs(self, config):
        log = logging.getLogger('FTagEventSFConfig')

        if config.dataType() is DataType.Data: return

        if config.isPhyslite() and self.triggerChainsPerYear:
            log.warning ('The b-jet trigger SF computation is currently not supported in PHYSLITE')
            return

        if 'FixedCut' in self.btagWP:
            raise ValueError('FTAG calibration is only available for Continuous WP. '
                             'Please configure the Continuous btagWP to retrieve scale factors.')


        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_':
            postfix = '_' + postfix

        triggers = set()
        if self.triggerChainsPerYear:
            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun)
        # Always add computation for non-trigger FTAG SF
        triggers.add("")

        particles, preselection = config.readNameAndSelection(self.containerName)

        # Set up the per-event FTAG efficiency scale factor calculation algorithm
        for chain in triggers:
            chain_noHLT = chain.replace("HLT_", "")
            chain_out = chain_noHLT if self.removeHLTPrefix else chain
            chain_out = chain_out.replace('-', '_').replace('.', 'p')

            postfix2 = postfix
            if chain:
                postfix2 = postfix2 + '_' + chain_out
            alg = config.createAlgorithm('CP::AsgEventScaleFactorAlg',
                                         'FTagEventScaleFactorAlg' + postfix2)

            alg.particles = particles
            alg.preselection = ((preselection + '&&' if preselection else '')
                                + 'no_ftag' + postfix2 + ',as_char')
            alg.scaleFactorInputDecoration = 'ftag_effSF' + postfix2 + '_%SYS%'
            alg.scaleFactorOutputDecoration = 'ftag_effSF' + postfix2 + '_%SYS%'

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration,
                                'weight_ftag_effSF' + postfix2)


@groupBlocks
def FlavourTaggingEventSF(seq, containerName='', selectionName=''):
    seq.append(FTagJetSFBlock())
    seq.setOptionValue('containerName', containerName)
    seq.setOptionValue('selectionName', selectionName)
    seq.append(FTagEventSFBlock())
    seq.setOptionValue('containerName', containerName)
    seq.setOptionValue('selectionName', selectionName)
