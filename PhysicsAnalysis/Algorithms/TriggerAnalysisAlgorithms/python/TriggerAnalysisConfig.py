# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

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
        self.addOption ('triggerChainsForDecoration', [], type=None,
            info="a list of trigger chains (list of strings) to be used for "
            "trigger decoration, if it needs to be different from the selection one. "
            "The default is [] (empty list).")
        self.addOption ('prescaleDecoration', 'prescale', type=str,
            info="name (prefix) of decoration for trigger prescales.")
        self.addOption ('prescaleLumiCalcFiles', [], type=None,
            info="a list of lumical files (list of strings) to calculate "
            "trigger prescales. The default is [] (empty list). Mutually "
            "exclusive with prescaleLumiCalcFilesPerYear")
        self.addOption ('prescaleLumiCalcFilesPerYear', {}, type=None,
            info="a dicrionary with key (string) the year and value (list of "
            "strings) the list of lumicalc files to calculate trigger prescales "
            "for an individual data year. The default is {} (empty dictionary). "
            "Mutually exclusive with prescaleLumiCalcFiles")
        self.addOption ('prescaleTriggersFormula', '', type=str,
            info="a formula used in (un)prescaling, producing overall prescale "
            "factor instead of prescale per trigger.")
        self.addOption ('prescaleMC', False, type=bool,
            info="ouput trigger prescales when running on MC. The default is False.")
        self.addOption ('unprescaleData', False, type=bool,
            info="ouput trigger prescales when running on Data. The default is False.")
        self.addOption ('prescaleIncludeAllYears', False, type=bool,
            info="if True, trigger prescales will include all configured years "
            "from prescaleLumiCalcFilesPerYear in all jobs. The default is False.")
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

    @staticmethod
    def _get_lumicalc_triggers(lumicalc_files: list[str]) -> list[str]:
        return [lumicalc.split(":")[-1] for lumicalc in lumicalc_files if ":" in lumicalc]

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

        # for decoration, make sure we only get the triggers not already in the selection list
        triggerChainsForDeco = list(set(self.triggerChainsForDecoration) - set(self.triggerChainsForSelection))
        if triggerChainsForDeco :
            alg = config.createAlgorithm( 'CP::TrigEventSelectionAlg', 'TrigEventSelectionAlgDeco' )
            alg.tool = '%s/%s' % \
                ( decisionTool.getType(), decisionTool.getName() )
            alg.triggers = triggerChainsForDeco
            alg.selectionDecoration = 'trigPassed'
            alg.noFilter = True
            alg.noL1 = self.noL1

            for t in triggerChainsForDeco :
                t = t.replace(".", "p").replace("-", "_")
                config.addOutputVar ('EventInfo', 'trigPassed_' + t, 'trigPassed_' + t, noSys=True)

        # Calculate trigger prescales
        if (self.prescaleLumiCalcFiles or self.prescaleLumiCalcFilesPerYear) and (
            self.unprescaleData if config.dataType() is DataType.Data else self.prescaleMC
        ):

            lumicalc_files = []
            if self.prescaleLumiCalcFiles:
                lumicalc_files = self.prescaleLumiCalcFiles
            elif self.prescaleLumiCalcFilesPerYear:
                from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import get_input_years, get_year_data

                years = get_input_years(config)
                for year in years:
                    lumicalc_files.extend(get_year_data(self.prescaleLumiCalcFilesPerYear, year))

            alg = config.createAlgorithm( 'CP::TrigPrescalesAlg', 'TrigPrescalesAlg' )
            config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
            alg.pileupReweightingTool.LumiCalcFiles = lumicalc_files
            alg.selectionDecoration = 'trigPassed'
            alg.prescaleMC = config.dataType() is not DataType.Data
            alg.prescaleDecoration = self.prescaleDecoration
            if self.prescaleTriggersFormula != '':
                alg.prescaleTriggersFormula = self.prescaleTriggersFormula
                config.addOutputVar("EventInfo", alg.prescaleDecoration, alg.prescaleDecoration, noSys=True)
            else:
                alg.triggers = self._get_lumicalc_triggers(lumicalc_files)
                alg.triggersAll = self.triggerChainsForSelection

                # Schedule trigger prescale output branches
                triggers_output = set(alg.triggers)
                if self.prescaleIncludeAllYears and self.prescaleLumiCalcFilesPerYear:
                    all_lumicalc_files = [
                        lumicalc
                        for lumicalc_year in self.prescaleLumiCalcFilesPerYear.values()
                        for lumicalc in lumicalc_year
                    ]
                    triggers_output.update(self._get_lumicalc_triggers(all_lumicalc_files))
                    alg.triggersAll = list(triggers_output)
                for trigger in triggers_output:
                    trigger = trigger.replace("-", "_")
                    config.addOutputVar(
                        "EventInfo",
                        alg.prescaleDecoration + "_" + trigger,
                        alg.prescaleDecoration + "_" + trigger,
                        noSys=True,
                    )

        return


    def makeAlgs (self, config) :

        if (self.multiTriggerChainsPerYear and self.triggerChainsPerYear and
            self.triggerChainsPerYear is not self.multiTriggerChainsPerYear.get('')):
            raise Exception('multiTriggerChainsPerYear and triggerChainsPerYear cannot be configured at the same time!')

        if self.prescaleLumiCalcFiles and self.prescaleLumiCalcFilesPerYear:
            raise Exception('prescaleLumiCalcFiles and prescaleLumiCalcFilesPerYear cannot be configured at the same time!')

        if self.prescaleIncludeAllYears and not self.prescaleLumiCalcFilesPerYear:
            raise Exception('prescaleIncludeAllYears requires prescaleLumiCalcFilesPerYear to be configured!')

        if (self.prescaleLumiCalcFiles or self.prescaleLumiCalcFilesPerYear) and not (
            self.unprescaleData or self.prescaleMC
        ):
            raise Exception('Lumicalc files are provided but no trigger prescale output is configured! Specify output with "unprescaleData" and/or "prescaleMC".')

        if self.triggerChainsPerYear and not self.multiTriggerChainsPerYear:
            self.multiTriggerChainsPerYear = {'': self.triggerChainsPerYear}

        # if we are only given the trigger dictionary, we fill the selection list automatically
        if self.triggerChainsPerYear and not self.triggerChainsForSelection:
            triggers_for_selection = set()
            triggers_for_decoration = set()
            from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import get_input_years
            years = get_input_years(config)
            for trigger_chains in self.multiTriggerChainsPerYear.values():
                for year, chain_list in trigger_chains.items():
                    target_triggers = triggers_for_selection if int(year) in years else triggers_for_decoration
                    for chain in chain_list:
                        if '||' in chain:
                            chains = chain.split('||')
                            target_triggers.update(map(str.strip, chains))
                        else:
                            target_triggers.add(chain.strip())
            self.triggerChainsForSelection = list(triggers_for_selection)
            self.triggerChainsForDecoration += list(triggers_for_decoration)

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
