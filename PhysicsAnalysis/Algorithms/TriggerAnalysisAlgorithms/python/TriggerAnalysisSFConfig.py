# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign


class TriggerAnalysisSFBlock (ConfigBlock):
    """the ConfigBlock for trigger analysis"""

    def __init__ (self, configName='') :
        super (TriggerAnalysisSFBlock, self).__init__ ()
        self.addDependency('Electrons', required=False)
        self.addDependency('Photons', required=False)
        self.addDependency('Muons', required=False)
        self.addDependency('OverlapRemoval', required=False)

        self.addOption ('triggerChainsPerYear', {}, type=None,
            info="a dictionary with key (string) the year and value (list of "
            "strings) the trigger chains. You can also use || within a string "
            "to enforce an OR of triggers without looking up the individual "
            "triggers. Used for both trigger selection and SFs. "
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
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. Still performs the global trigger "
            "matching (same behaviour as on data). The default is False.")
        self.addOption ('noGlobalTriggerEff', False, type=bool,
            info="disables the global trigger efficiency tool (including "
            "matching), which is only suited for electron/muon/photon "
            "trigger legs. The default is False.")
    

    def makeTriggerDecisionTool(self, config):
        # Might have already been added in TriggerAnalysisBlock
        if "TrigDecisionTool" in config._algorithms:
            return config._algorithms["TrigDecisionTool"]

        # Create public trigger tools
        xAODConfTool = config.createPublicTool( 'TrigConf::xAODConfigTool', 'xAODConfigTool' )
        decisionTool = config.createPublicTool( 'Trig::TrigDecisionTool', 'TrigDecisionTool' )
        decisionTool.ConfigTool = '%s/%s' % \
            ( xAODConfTool.getType(), xAODConfTool.getName() )
        if config.geometry() == LHCPeriod.Run3:
            decisionTool.NavigationFormat = 'TrigComposite' # Read Run 3 navigation (options are "TrigComposite" for R3 or "TriggElement" for R2, R2 navigation is not kept in most DAODs)
            decisionTool.HLTSummary = 'HLTNav_Summary_DAODSlimmed' # Name of R3 navigation container (if reading from AOD, then "HLTNav_Summary_AODSlimmed" instead)

        return decisionTool

    def makeTriggerMatchingTool(self, config, decisionTool):

        # Create public trigger tools
        drScoringTool = config.createPublicTool( 'Trig::DRScoringTool', 'DRScoringTool' )
        if config.geometry() == LHCPeriod.Run3:
            matchingTool = config.createPublicTool( 'Trig::R3MatchingTool', 'MatchingTool' )
            matchingTool.ScoringTool = '%s/%s' % \
                    ( drScoringTool.getType(), drScoringTool.getName() )
            matchingTool.TrigDecisionTool = '%s/%s' % \
                    ( decisionTool.getType(), decisionTool.getName() )
        else:
            matchingTool = config.createPublicTool( 'Trig::MatchFromCompositeTool', 'MatchingTool' )
            if config.isPhyslite():
                matchingTool.InputPrefix = "AnalysisTrigMatch_"

        return matchingTool

    
    def makeTriggerGlobalEffCorrAlg(self, config, matchingTool, noSF):

        alg = config.createAlgorithm( 'CP::TrigGlobalEfficiencyAlg', 'TrigGlobalSFAlg' )
        if config.geometry() == LHCPeriod.Run3:
            alg.triggers_2022 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2022',[])]
            alg.triggers_2023 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2023',[])]
            alg.triggers_2024 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2024',[])]
            alg.triggers_2025 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2025',[])]
            if config.campaign() in [Campaign.MC21a, Campaign.MC23a]:
                if not alg.triggers_2022:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2022!' )
            elif config.campaign() is Campaign.MC23c:
                if not alg.triggers_2023:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2023!' )
        else:
            alg.triggers_2015 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2015',[])]
            alg.triggers_2016 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2016',[])]
            alg.triggers_2017 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2017',[])]
            alg.triggers_2018 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2018',[])]
            if config.campaign() is Campaign.MC20a:
                if not (alg.triggers_2015 and alg.triggers_2016):
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the years 2015 and 2016!' )
            elif config.campaign() is Campaign.MC20d:
                if not alg.triggers_2017:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2017!' )
            elif config.campaign() is Campaign.MC20e:
                if not alg.triggers_2018:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2018!' )

        alg.matchingTool = '%s/%s' % ( matchingTool.getType(), matchingTool.getName() )
        alg.isRun3Geo = config.geometry() == LHCPeriod.Run3
        alg.scaleFactorDecoration = 'globalTriggerEffSF_%SYS%'
        alg.matchingDecoration = 'globalTriggerMatch_%SYS%'
        alg.eventDecisionOutputDecoration = 'dontsave_%SYS%'
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
            raise ValueError ('TriggerAnalysisConfig: at least one object collection must be provided! (electrons, muons, photons)' )

        if config.dataType() != DataType.Data and not alg.doMatchingOnly:
            config.addOutputVar ('EventInfo', alg.scaleFactorDecoration, 'globalTriggerEffSF')
        config.addOutputVar ('EventInfo', alg.matchingDecoration, 'globalTriggerMatch', noSys=True)

        return

    def makeAlgs (self, config) :

        # Create the decision algorithm, keeping track of the decision tool for later
        decisionTool = self.makeTriggerDecisionTool(config)

        # Now pass it to the matching algorithm, keeping track of the matching tool for later
        matchingTool = self.makeTriggerMatchingTool(config, decisionTool)

        # Calculate multi-lepton (electron/muon/photon) trigger efficiencies and SFs
        if self.triggerChainsPerYear and not self.noGlobalTriggerEff:
            self.makeTriggerGlobalEffCorrAlg(config, matchingTool, self.noEffSF)

        return
