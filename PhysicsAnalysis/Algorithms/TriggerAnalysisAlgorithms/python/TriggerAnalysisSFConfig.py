# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from typing import Iterable, Union

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType, ConfigAccumulator
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock, is_year_in_current_period


def is_mc_from(config: ConfigAccumulator, campaign_list: Union[Campaign, Iterable[Campaign]]) -> bool:
    """
    Utility function to check whether the data type is mc and whether the campaign is in desired list of campaigns
    without causing an invalid FlugEnum comparison
    """
    campaign_list = [campaign_list] if isinstance(campaign_list, Campaign) else campaign_list
    return config.dataType() is not DataType.Data and config.campaign() in campaign_list


def is_data_from(config, data_year_list: Union[int, Iterable[int]]) -> bool:
    """
    Utility function to check whether the data type is data and whether the year is in desired list of years
    """
    data_year_list = [data_year_list] if isinstance(data_year_list, int) else data_year_list
    return config.dataType() is DataType.Data and config.dataYear() in data_year_list


def get_year_data(dictionary: dict, year: int | str) -> list:
    """
    Utility function to get the data for a specific year from a dictionary
    """
    return dictionary.get(int(year), dictionary.get(str(year), []))


def get_input_years(config: ConfigAccumulator) -> list[int]:
    """
    Utility function to get the list of years that the input corresponds to
    """
    years = []
    if config.campaign() is Campaign.MC20a:
        years = [2015, 2016]
    elif is_data_from(config, 2015):
        years = [2015]
    elif is_data_from(config, 2016):
        years = [2016]
    elif config.campaign() is Campaign.MC20d or is_data_from(config, 2017):
        years = [2017]
    elif config.campaign() is Campaign.MC20e or is_data_from(config, 2018):
        years = [2018]
    elif config.campaign() is Campaign.MC23a or is_data_from(config, 2022):
        years = [2022]
    elif config.campaign() is Campaign.MC23d or is_data_from(config, 2023):
        years = [2023]
    elif config.campaign() is Campaign.MC23e or is_data_from(config, 2024):
        years = [2024]
    elif config.campaign() is Campaign.MC23g or is_data_from(config, 2025):
        years = [2025]
    else:
        raise ValueError('TriggerAnalysisConfig: unable to deduce data taking year for input file')

    return years


class TriggerAnalysisSFBlock(ConfigBlock):
    """the ConfigBlock for trigger analysis"""
    def __init__(self):
        super(TriggerAnalysisSFBlock, self).__init__()
        self.addDependency('Electrons', required=False)
        self.addDependency('Photons', required=False)
        self.addDependency('Muons', required=False)
        self.addDependency('Taus', required=False)
        self.addDependency('OverlapRemoval', required=False)

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
        self.addOption ('noFilter', False, type=bool,
            info="do not apply an event filter. The default is False, i.e. "
            "remove events not passing trigger selection and matching.")
        self.addOption ('electronID', '', type=str,
            info="the electron ID WP (string) to use.")
        self.addOption ('electronIsol', '', type=str,
            info="the electron isolation WP (string) to use.")
        self.addOption ('photonIsol', '', type=str,
            info="the photon isolation WP (string) to use.")
        self.addOption ('muonID', '', type=str,
            info="the muon quality WP (string) to use.")
        self.addOption ('electrons', '', type=str,
            info="the input electron container, with a possible selection, in "
            "the format container or container.selection.")
        self.addOption ('muons', '', type=str,
            info="the input muon container, with a possible selection, in the "
            "format container or container.selection.")
        self.addOption ('photons', '', type=str,
            info="the input photon container, with a possible selection, in "
            "the format container or container.selection.")
        self.addOption ('taus', '', type=str,
            info="the input tau container, with a possible selection, in "
            "the format container or container.selection.")
        self.addOption ('numberOfToys', 0, type=int,
            info="Number of toy experiments to run to estimate the trigger efficiencies, "
            "instead of using explicit formulas. The default is 0 (not using toys).")
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. Still performs the global trigger "
            "matching (same behaviour as on data). The default is False.",
            expertMode=True)
        self.addOption ('noGlobalTriggerEff', False, type=bool,
            info="disables the global trigger efficiency tool (including "
            "matching), which is only suited for electron/muon/photon "
            "trigger legs. The default is False.")
        self.addOption ('separateChainMatching', False, type=bool,
            info="Store the matching status for each trigger separately")
        self.addOption ('triggerMatchingChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption("includeAllYearsPerRun", False, type=bool,
            info="if True, trigger matching will include all configured years "
            "in the LHC run in all jobs. The default is False.")
        self.addOption ('postfix', '', type=str,
            info="a unique identifier for the trigger matching decorations. Only "
            "useful when defining multiple setups. The default is '' (empty string).")

    def makeTriggerGlobalEffCorrAlg(
        self,
        config: ConfigAccumulator,
        matchingTool,
        noSF: bool,
        triggerSuffix: str = ''
    ) -> None:
        alg = config.createAlgorithm( 'CP::TrigGlobalEfficiencyAlg', 'TrigGlobalSFAlg' + triggerSuffix + self.postfix)
        if config.geometry() is LHCPeriod.Run3:
            alg.triggers_2022 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2022)]
            alg.triggers_2023 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2023)]
            alg.triggers_2024 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2024)]
            alg.triggers_2025 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2025)]
            if is_mc_from(config, Campaign.MC23a) or is_data_from(config, 2022):
                if not alg.triggers_2022:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2022!')
            elif is_mc_from(config, Campaign.MC23d) or is_data_from(config, 2023):
                if not alg.triggers_2023:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2023!')
        else:
            alg.triggers_2015 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2015)]
            alg.triggers_2016 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2016)]
            alg.triggers_2017 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2017)]
            alg.triggers_2018 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in get_year_data(self.triggerChainsPerYear, 2018)]
            if is_mc_from(config, Campaign.MC20a):
                if not (alg.triggers_2015 and alg.triggers_2016):
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the years 2015 and 2016!')
            elif is_data_from(config, 2015):
                if not alg.triggers_2015:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2015!')
            elif is_data_from(config, 2016):
                if not alg.triggers_2016:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2016!')
            elif is_mc_from(config, Campaign.MC20d) or is_data_from(config, 2017):
                if not alg.triggers_2017:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2017!')
            elif is_mc_from(config, Campaign.MC20e) or is_data_from(config, 2018):
                if not alg.triggers_2018:
                    raise ValueError('TriggerAnalysisConfig: you must provide a set of triggers for the year 2018!')

        triggerMatchingChains = set()
        if self.separateChainMatching:
            for year in get_input_years(config):
                for trig in get_year_data(self.triggerChainsPerYear, year):
                    triggerMatchingChains.update(trig.replace(' || ', '_OR_').split('_OR_'))
        alg.separateMatchingTriggers = list(triggerMatchingChains)
        alg.separateMatchingDecorationSuffix = triggerSuffix + self.postfix

        alg.matchingTool = '%s/%s' % ( matchingTool.getType(), matchingTool.getName() )
        alg.isRun3Geo = config.geometry() is LHCPeriod.Run3
        alg.numberOfToys = self.numberOfToys
        alg.scaleFactorDecoration = 'globalTriggerEffSF' + triggerSuffix + self.postfix + '_%SYS%'
        alg.matchingDecoration = 'globalTriggerMatch' + triggerSuffix + self.postfix + '_%SYS%'
        alg.eventDecisionOutputDecoration = 'globalTriggerMatch' + triggerSuffix + self.postfix + '_dontsave_%SYS%'
        alg.doMatchingOnly = config.dataType() is DataType.Data or noSF
        alg.noFilter = self.noFilter
        alg.electronID = self.electronID
        alg.electronIsol = self.electronIsol
        alg.photonIsol = self.photonIsol
        alg.muonID = self.muonID
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        if self.photons:
            alg.photons, alg.photonSelection = config.readNameAndSelection(self.photons)
        if not (self.electrons or self.muons or self.photons):
            raise ValueError('TriggerAnalysisConfig: at least one object collection must be provided! (electrons, muons, photons)' )

        if config.dataType() is not DataType.Data and not alg.doMatchingOnly:
            config.addOutputVar('EventInfo', alg.scaleFactorDecoration, 'globalTriggerEffSF' + triggerSuffix + self.postfix)
        config.addOutputVar('EventInfo', alg.matchingDecoration, 'globalTriggerMatch' + triggerSuffix + self.postfix, noSys=False)

        for trig in triggerMatchingChains:
            var = f'triggerMatch_{trig.replace("-", "_").replace(".", "p")}{alg.separateMatchingDecorationSuffix}'
            config.addOutputVar('EventInfo', f'{var}_%SYS%', var, noSys=False)

        return

    def createTrigMatching(
        self,
        config: ConfigAccumulator,
        matchingTool,
        particles,
        trig_string: str = "",
        trig_chains: set = None,
        trig_chains_dummy: set = None,
    ) -> None:
        if not particles or (not trig_chains and not trig_chains_dummy):
            return

        if not any(trig_string in trig for trig in trig_chains) \
            and not any(trig_string in trig for trig in trig_chains_dummy):
            return

        alg = config.createAlgorithm("CP::TrigMatchingAlg", f"TrigMatchingAlg_{trig_string}{self.postfix}")
        alg.matchingTool = f"{matchingTool.getType()}/{matchingTool.getName()}"
        alg.matchingDecoration = f"trigMatched{self.postfix}"
        alg.trigSingleMatchingList = [trig for trig in trig_chains if trig_string in trig]
        alg.trigSingleMatchingListDummy = [trig for trig in trig_chains_dummy if trig_string in trig]
        alg.particles, alg.particleSelection = config.readNameAndSelection(particles)

        for trig in list(alg.trigSingleMatchingList) + list(alg.trigSingleMatchingListDummy):
            trig = trig.replace(".", "p").replace("-", "_").replace(" ", "")
            if trig_string in trig:
                config.addOutputVar(particles.split(".")[0], f"trigMatched_{self.postfix}{trig}", f"trigMatched_{self.postfix}{trig}", noSys=True)

    def makeTrigMatchingAlg(
        self,
        config: ConfigAccumulator,
        matchingTool,
    ) -> None:
        years = get_input_years(config)

        triggerMatchingChains = set()
        triggerMatchingChainsDummy = set()
        for year in years:
            for trig in get_year_data(self.triggerMatchingChainsPerYear, year):
                trig = trig.replace(' || ', '_OR_')
                triggerMatchingChains.update(trig.split('_OR_'))
        if self.includeAllYearsPerRun:
            triggerMatchingChainsAll = set()
            for year in self.triggerMatchingChainsPerYear:
                if not is_year_in_current_period(config, year):
                    continue
                for trig in get_year_data(self.triggerMatchingChainsPerYear, year):
                    trig = trig.replace(' || ', '_OR_')
                    triggerMatchingChainsAll.update(trig.split('_OR_'))
            triggerMatchingChainsDummy = triggerMatchingChainsAll - triggerMatchingChains

        self.createTrigMatching(config, matchingTool, self.electrons, 'HLT_e',   triggerMatchingChains, triggerMatchingChainsDummy)
        self.createTrigMatching(config, matchingTool, self.muons,     'HLT_mu',  triggerMatchingChains, triggerMatchingChainsDummy)
        self.createTrigMatching(config, matchingTool, self.photons,   'HLT_g',   triggerMatchingChains, triggerMatchingChainsDummy)
        self.createTrigMatching(config, matchingTool, self.taus,      'HLT_tau', triggerMatchingChains, triggerMatchingChainsDummy)

        return

    def makeAlgs(self, config: ConfigAccumulator) -> None:
        if (
            self.multiTriggerChainsPerYear and
            self.triggerChainsPerYear and
            self.triggerChainsPerYear not in self.multiTriggerChainsPerYear.values()
        ):
            raise Exception('multiTriggerChainsPerYear and triggerChainsPerYear cannot be configured at the same time!')

        if self.triggerChainsPerYear and not self.multiTriggerChainsPerYear:
            self.multiTriggerChainsPerYear = {'': self.triggerChainsPerYear}

        # Create the decision algorithm, keeping track of the decision tool for later
        decisionTool = TriggerAnalysisBlock.makeTriggerDecisionTool(config)

        # Now pass it to the matching algorithm, keeping track of the matching tool for later
        matchingTool = TriggerAnalysisBlock.makeTriggerMatchingTool(config, decisionTool)

        # Calculate multi-lepton (electron/muon/photon) trigger efficiencies and SFs
        if self.multiTriggerChainsPerYear and not self.noGlobalTriggerEff:
            for suffix, trigger_chains in self.multiTriggerChainsPerYear.items():
                self.triggerChainsPerYear = trigger_chains
                self.makeTriggerGlobalEffCorrAlg(config, matchingTool, self.noEffSF, suffix)

        # Save trigger matching information (currently only single leg trigger are supported)
        if self.triggerMatchingChainsPerYear:
            self.makeTrigMatchingAlg(config, matchingTool)

        return
