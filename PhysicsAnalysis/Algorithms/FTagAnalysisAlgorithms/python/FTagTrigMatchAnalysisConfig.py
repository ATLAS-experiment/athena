# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.Logging import logging

from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign

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


class FTagJetTrigMatchingBlock(ConfigBlock):
    """the ConfigBlock for the FTAG jet trigger matching"""
    def __init__(self):
        super(FTagJetTrigMatchingBlock, self).__init__()
        self.addOption('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('triggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
            info="if True, all configured years in the LHC run will be included in all jobs. "
            "The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
            info="remove the HLT prefix from trigger chain names, "
            "The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs(self, config):
        log = logging.getLogger('FTagTrigMatchConfig')

        if config.isPhyslite():
            log.warning ('The b-jet trigger matching is currently not supported in PHYSLITE')
            return
        if config.geometry() is LHCPeriod.Run2:
            log.warning ('The b-jet trigger matching is currently not supported for Run 2')
            return

        # Need to split container name from selections, to support AnaJets.baselineJvt
        jetContainer = self.containerName.split('.')[0]

        if self.triggerChainsPerYear:
            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun, log)
            decisionTool = TriggerAnalysisBlock.makeTriggerDecisionTool(config)
            
            for chain in triggers:
                chain_noHLT = chain.replace("HLT_", "")
                chain_out = chain_noHLT if self.removeHLTPrefix else chain
                chain_out = chain_out.replace('-', '_').replace('.', 'p')

                alg = config.createAlgorithm( 'CP::BTaggingTriggerMatchingAlg',
                                              'FTagTriggerMatchingAlg' + chain )
                alg.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"
                alg.trigger = chain
                alg.useRun3TriggerEDM = config.geometry() is LHCPeriod.Run3
                # Helper function to implement to provide cut for given trigger
                # Only used for Run 2
                #alg.btagThreshold = getBTagThreshold(chain)

                alg.matchingDecoration = 'ftag_jetTrigMatching_' + chain_out + '_%SYS%'
                alg.bTagMatchingDecoration = 'ftag_bTagTrigMatching_' + chain_out + '_%SYS%'
                alg.preselection = config.getPreselection (jetContainer, '')
                alg.jets = config.readName(jetContainer)
                config.addOutputVar (jetContainer, alg.matchingDecoration,
                                     'ftag_jetTrigMatching_' + chain_out)
                config.addOutputVar (jetContainer, alg.bTagMatchingDecoration,
                                     'ftag_bTagTrigMatching_' + chain_out)
