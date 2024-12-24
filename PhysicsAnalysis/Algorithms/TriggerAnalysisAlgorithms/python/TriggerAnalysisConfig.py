# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod


class TriggerAnalysisBlock (ConfigBlock):
    """the ConfigBlock for trigger analysis"""

    # configName is not used
    def __init__ (self, configName='') :
        super (TriggerAnalysisBlock, self).__init__ ()
        self.addOption ('triggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. You can also use || within a string "
            "to enforce an OR of triggers without looking up the individual "
            "triggers. Used for both trigger selection and SFs. "
            "The default is {} (empty dictionary).")
        self.addOption ('multiTriggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) a trigger set name and value a "
            "triggerChainsPerYear dictionary, following the previous convention. "
            "Relevant for analyses using different triggers in different categories, "
            "where the trigger global scale factors shouldn't be combined. "
            "The default is {} (empty dictionary).")
        self.addOption ('triggerChainsForSelection', [], type=None,
            info="a list of trigger chains (list of strings) to be used for "
            "trigger selection. Only set it if you need a different setup "
            "than for trigger SFs. The default is [] (empty list).")
        self.addOption ('prescaleLumiCalcFiles', [], type=None,
            info="a list of lumical files (list of strings) to calculate "
            "trigger prescales. The default is [] (empty list).")
        self.addOption ('prescaleTriggersFormula', '', type=str,
            info="a formula used in (un)prescaling, producing overall prescale "
            "factor instead of prescale per trigger.")
        self.addOption ('prescaleMC', False, type=bool,
            info="prescale MC instead of unprescaling of data.")
        self.addOption ('noFilter', False, type=bool,
            info="do not apply an event filter. The default is False, i.e. "
            "remove events not passing trigger selection and matching.")
        # TODO: add info string
        self.addOption ('noL1', False, type=bool,
            info="")

    @staticmethod
    def makeTriggerDecisionTool(config):
        # Might have already been added
        toolName = "TrigDecisionTool"
        if toolName in config._algorithms:
            return config._algorithms[toolName]

        # Create public trigger tools
        xAODConfTool = config.createPublicTool("TrigConf::xAODConfigTool", "xAODConfigTool")
        decisionTool = config.createPublicTool("Trig::TrigDecisionTool", toolName)
        decisionTool.ConfigTool = f"{xAODConfTool.getType()}/{xAODConfTool.getName()}"
        decisionTool.HLTSummary = config.hltSummary()
        if config.geometry() is LHCPeriod.Run3:
            # Read Run 3 navigation
            # (options are "TrigComposite" for R3 or "TriggElement" for R2,
            # R2 navigation is not kept in most DAODs)
            decisionTool.NavigationFormat = "TrigComposite"

        return decisionTool

    @staticmethod
    def makeTriggerMatchingTool(config, decisionTool):
        # Might have already been added
        toolName = "TrigMatchingTool"
        if toolName in config._algorithms:
            return config._algorithms[toolName]

        # Create public trigger tools
        if config.geometry() is LHCPeriod.Run3:
            drScoringTool = config.createPublicTool("Trig::DRScoringTool", "DRScoringTool")
            matchingTool = config.createPublicTool("Trig::R3MatchingTool", toolName)
            matchingTool.ScoringTool = f"{drScoringTool.getType()}/{drScoringTool.getName()}"
            matchingTool.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"
        else:
            matchingTool = config.createPublicTool("Trig::MatchFromCompositeTool", toolName)
            if config.isPhyslite():
                matchingTool.InputPrefix = "AnalysisTrigMatch_"
        return matchingTool


    def makeTriggerSelectionAlg(self, config, decisionTool):

        # Set up the trigger selection:
        alg = config.createAlgorithm( 'CP::TrigEventSelectionAlg', 'TrigEventSelectionAlg' )
        alg.tool = '%s/%s' % \
            ( decisionTool.getType(), decisionTool.getName() )
        alg.triggers = self.triggerChainsForSelection
        alg.selectionDecoration = 'trigPassed'
        alg.noFilter = self.noFilter
        alg.noL1 = self.noL1

        for t in self.triggerChainsForSelection :
            t = t.replace(".", "p").replace("-", "_")
            config.addOutputVar ('EventInfo', 'trigPassed_' + t, 'trigPassed_' + t, noSys=True)

        # Calculate trigger prescales
        if ((config.dataType() is DataType.Data) != self.prescaleMC) and self.prescaleLumiCalcFiles:

            alg = config.createAlgorithm( 'CP::TrigPrescalesAlg', 'TrigPrescalesAlg' )
            config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
            alg.pileupReweightingTool.LumiCalcFiles = self.prescaleLumiCalcFiles
            alg.pileupReweightingTool.TrigDecisionTool = '%s/%s' % \
                    ( decisionTool.getType(), decisionTool.getName() )
            if self.prescaleTriggersFormula != '':
                alg.prescaleTriggersFormula = self.prescaleTriggersFormula
            else:
                alg.triggers = [lumicalc.split(':')[-1] for lumicalc in self.prescaleLumiCalcFiles if ':' in lumicalc]
                alg.triggersAll = self.triggerChainsForSelection
            alg.prescaleMC = self.prescaleMC
            alg.prescaleDecoration = 'prescale'

        return


    def makeAlgs (self, config) :

        if (self.multiTriggerChainsPerYear and self.triggerChainsPerYear and
            self.triggerChainsPerYear is not self.multiTriggerChainsPerYear.get('')):
            raise Exception('multiTriggerChainsPerYear and triggerChainsPerYear cannot be configured at the same time!')

        if self.triggerChainsPerYear and not self.multiTriggerChainsPerYear:
            self.multiTriggerChainsPerYear = {'': self.triggerChainsPerYear}

        # if we are only given the trigger dictionary, we fill the selection list automatically
        if self.triggerChainsPerYear and not self.triggerChainsForSelection:
            triggers = set()
            for trigger_chains in self.multiTriggerChainsPerYear.values():
                for chain_list in self.triggerChainsPerYear.values():
                    for chain in chain_list:
                        if '||' in chain:
                            chains = chain.split('||')
                            triggers.update(map(str.strip, chains))
                        else:
                            triggers.add(chain.strip())
            self.triggerChainsForSelection = list(triggers)

        # Create the decision algorithm, keeping track of the decision tool for later
        decisionTool = self.makeTriggerDecisionTool(config)

        if self.triggerChainsForSelection:
            self.makeTriggerSelectionAlg(config, decisionTool)

        return



@groupBlocks
def Trigger(seq):
    seq.append(TriggerAnalysisBlock())
    from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import TriggerAnalysisSFBlock
    seq.append(TriggerAnalysisSFBlock())
