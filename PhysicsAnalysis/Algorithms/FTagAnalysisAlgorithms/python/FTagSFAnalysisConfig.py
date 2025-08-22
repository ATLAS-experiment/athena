# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign

from FTagAnalysisAlgorithms.FTagHelpers import getRecommendedBTagCalib, getReadFromBTaggingObject
from CalibrationDataInterface.CDIHelpers import check_CDI_campaign
from CalibrationDataInterface.MCMCGeneratorHelper import MCMC_dsid_map
from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock, is_year_in_current_period
from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import get_year_data


def trigger_set(config, triggerChainsPerYear, includeAllYearsPerRun, log):
    triggers = set()
    if includeAllYearsPerRun:
        for year in triggerChainsPerYear:
            if not is_year_in_current_period(config, year):
                continue
            triggers.update(get_year_data(triggerChainsPerYear, year))
    elif config.campaign() is Campaign.MC20a:
        triggers.update(get_year_data(triggerChainsPerYear, 2015))
        triggers.update(get_year_data(triggerChainsPerYear, 2016))
    elif config.campaign() is Campaign.MC20d:
        triggers.update(get_year_data(triggerChainsPerYear, 2017))
    elif config.campaign() is Campaign.MC20e:
        triggers.update(get_year_data(triggerChainsPerYear, 2018))
    elif config.campaign() is Campaign.MC23a:
        triggers.update(get_year_data(triggerChainsPerYear, 2022))
    elif config.campaign() is Campaign.MC23d:
        triggers.update(get_year_data(triggerChainsPerYear, 2023))
    else:
        log.warning("unknown campaign, skipping triggers: %s", str(config.campaign()))
    return triggers


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

        if config.dataType() is DataType.Data: return

        log = logging.getLogger('FTagJetSFConfig')

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
        
        jetCollection = config.originalName(jetContainer)
        # Potentially modify the readFromBTaggingObject as here determining 
        # if input files has jet tagging probabilities attached to the jet (or still only to the BTagging object)
        self.readFromBTaggingObject = getReadFromBTaggingObject(config.autoconfigFlags(), jetCollection, self.readFromBTaggingObject)

        # b-jet trigger-aware SF
        if self.triggerChainsPerYear:
            log.warning("The configuration of the FTAG trigger-aware SF is still "
                        "under development. This is not ready yet for analysis usage!")

            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun, log)
            decisionTool = TriggerAnalysisBlock.makeTriggerDecisionTool(config)
            
            ChainDict = [
                    "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                    "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                    "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                    "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"]
            
            for chain in triggers:
                if  chain not in ChainDict: 
                    raise ValueError(f"Trigger '{chain}' not supported — no known navigation issues") 
                
                chain_noHLT = chain.replace("HLT_", "")
                chain_out = chain_noHLT if self.removeHLTPrefix else chain

                if self.bTagCalibTriggerFile is not None :
                    bTagCalibTriggerFile = self.bTagCalibTriggerFile
                else:
                    # Interface to retrieve b-jet trigger CDI + tagger-wp to be implemented when available
                    # bTagCalibTriggerFile = getRecommendedBTagTrigCalib(config.geometry(), trigger)
                    # Set nothing for now
                    bTagCalibTriggerFile = ""

                # bTagOnlineTagger, bTagOnlineWP = getBTagOnlineTaggerWP(trigger)
                # For now configure fixed WP
                bTagOnlineTagger = "OnlineDL1d"
                bTagOnlineWP = "FixedCutBEff_77"
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

                alg.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"

                alg.trigger = chain
                alg.useRun3TriggerEDM = config.geometry() is LHCPeriod.Run3
                # Helper function to implement to provide cut for given trigger
                # Only used for Run 2
                #alg.btagThreshold = getBTagThreshold(chain)

                alg.scaleFactorDecoration = 'ftag_effSF_' + selectionName + '_' + chain_out + '_%SYS%'
                alg.selectionDecoration = 'ftag_select_' + selectionName + '_' + chain_out + ',as_char'
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

        if config.dataType() is DataType.Data: return

        if 'FixedCut' in self.btagWP:
            raise ValueError('FTAG calibration is only available for Continuous WP. '
                             'Please configure the Continuous btagWP to retrieve scale factors.')


        log = logging.getLogger('FTagEventSFConfig')

        selectionName = self.selectionName
        if selectionName is None or selectionName == '':
            selectionName = self.btagger + '_' + self.btagWP

        postfix = selectionName
        if postfix != "" and postfix[0] != '_':
            postfix = '_' + postfix

        triggers = set()
        if self.triggerChainsPerYear:
            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun, log)
        # Always add computation for non-trigger FTAG SF
        triggers.add("")

        particles, preselection = config.readNameAndSelection(self.containerName)

        # Set up the per-event FTAG efficiency scale factor calculation algorithm
        for chain in triggers:
            postfix2 = postfix
            if chain:
                postfix2 = postfix2 + '_' + chain
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
