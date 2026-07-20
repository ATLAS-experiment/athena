# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock
from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import trigger_set


class JetTriggerMatchingBlock (ConfigBlock):

    def __init__ (self) :
        super (JetTriggerMatchingBlock, self).__init__ ()
        self.addOption ('triggerChainsPerYear', {}, type=dict,
                        info="a dictionary with key (string) the year and value (list of "
                        "strings) the trigger chains.")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
                        info="all configured years in the LHC run will "
                        "be included in all jobs.")
        self.addOption ('removeHLTPrefix', True, type=bool,
                        info="remove the HLT prefix from trigger chain names.")
        self.addOption ('containerName', '', type=str,
                        info="the input jet container, with a possible selection, in "
                        "the format `container` or `container.selection`.")
        self.addOption ('runL1Matching', True, type=bool,
                        info="Add L1 matching decorations")
        self.addOption ('runHLTMatching', True, type=bool,
                        info="Add HLT matching decorations")
        self.addOption ('l1dR', 0.4, type=float,
                        info="ΔR cone for the L1 (jFEX) trigger matching "
                        "(sets CP::JetTriggerDecoratorAlg.l1dR_cut). Default 0.4 "
                        "matches the algorithm's own default.")
        self.addOption ('hltDR', 0.4, type=float,
                        info="ΔR cone for the HLT trigger matching "
                        "(sets CP::JetTriggerDecoratorAlg.hltDR_cut). Default 0.4 "
                        "matches the algorithm's own default.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        triggers = trigger_set(config, self.triggerChainsPerYear,
                               self.includeAllYearsPerRun)
        decisionTool = TriggerAnalysisBlock.makeTriggerDecisionTool(config)

        for chain in triggers:
            chain_noHLT = chain.replace("HLT_", "")
            chain_out = chain_noHLT if self.removeHLTPrefix else chain
            chain_out = chain_out.replace("-","_").replace(".","p")

            alg = config.createAlgorithm( 'CP::JetTriggerDecoratorAlg',
                                          'JetTriggerDecoratorAlg_' + chain_out )

            alg.trigger = chain

            # To add missing HLT jets -- only for buggy triggers
            # These extra jets are added because of bug in trigger navigation
            # Will be removed once bug fixed at DAOD level
            alg.triggerBugList = [
                "HLT_j80c_020jvt_j55c_020jvt_j28c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bdl1d77_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn177_pf_ftf_presel2c20XX2c20b85_L1J45p0ETA21_3J15p0ETA25"
            ]

            alg.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"

            alg.doL1Matching = self.runL1Matching
            alg.doHLTMatching = self.runHLTMatching
            alg.jets = config.readName (self.containerName)

            # Phase-I L1Calo (jFEX) vs legacy L1Calo is not a user choice — it is
            # fixed by the data-taking year (data 2024+) and the MC campaign
            # (mc23e/mc23g). Determine it automatically from the config metadata.
            if config.dataType() is DataType.Data:
                usePhaseIL1 = config.dataYear() >= 2024
            else:
                usePhaseIL1 = config.campaign() in (Campaign.MC23e, Campaign.MC23g)
            alg.usePhaseIL1 = usePhaseIL1
            alg.l1dR_cut = self.l1dR
            alg.hltDR_cut = self.hltDR

            if config.geometry() is LHCPeriod.Run2 and self.runHLTMatching:
                # Configuration adapted from
                # https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TrigEmulation/TrigBtagEmulationTool/python/TrigBtagEmulationToolConfig.py

                alg.useEmulationTool = True
                config.addPrivateTool( 'trigEmulationTool',
                                       'Trig::TrigBtagEmulationTool' )

                from TrigBtagEmulationTool.TrigBtagEmulationToolHelpers import (
                    TrigBtagEmulation_kwargs)
                tool_kwargs = TrigBtagEmulation_kwargs(config.flags, [chain])
                for prop, value in tool_kwargs.items():
                    setattr(alg.trigEmulationTool, prop, value)

                alg.trigEmulationTool.TrigDecisionTool = (
                    f"{decisionTool.getType()}/{decisionTool.getName()}")

                config.addPrivateTool( 'trigEmulationTool.JM_a4tcemsubjes_CNT',
                                       'Trig::JetManagerTool' )
                a4tcemsubjesJet = ('HLT_xAOD__JetContainer_a4tcemsubjesFS'
                                   if config.campaign() is Campaign.MC20a or config.dataYear()==2016
                                   else 'HLT_xAOD__JetContainer_a4tcemsubjesISFS')
                alg.trigEmulationTool.JM_a4tcemsubjes_CNT.JetContainerName = a4tcemsubjesJet
                alg.trigEmulationTool.JM_a4tcemsubjes_CNT.LHCPeriod = 2

                config.addPrivateTool( 'trigEmulationTool.JM_Split_CNT',
                                       'Trig::JetManagerTool' )
                alg.trigEmulationTool.JM_Split_CNT.JetContainerName = 'HLT_xAOD__JetContainer_SplitJet'
                alg.trigEmulationTool.JM_Split_CNT.LHCPeriod = 2

                if not(config.campaign() is Campaign.MC20a or config.dataYear()==2016):
                    config.addPrivateTool( 'trigEmulationTool.JM_GSC_CNT',
                                           'Trig::JetManagerTool' )
                    alg.trigEmulationTool.JM_GSC_CNT.JetContainerName = 'HLT_xAOD__JetContainer_GSCJet'
                    alg.trigEmulationTool.JM_GSC_CNT.LHCPeriod = 2

            if self.runL1Matching:
                alg.L1Et = "match_" + chain_out + "_L1et_%SYS%"
                alg.L1Eta = "match_" + chain_out + "_L1eta_%SYS%"
                alg.L1Phi = "match_" + chain_out + "_L1phi_%SYS%"
                alg.L1DR = "match_" + chain_out + "_L1dr_%SYS%"
                alg.L1Threshold = "match_" + chain_out + "_L1thresholds_%SYS%"
                for var in ["L1et", "L1eta", "L1phi", "L1dr", "L1thresholds"]:
                    config.addOutputVar (self.containerName,
                                         "match_" + chain_out + "_" + var + "_%SYS%",
                                         "match_" + chain_out + "_" + var, noSys=True)

            if self.runHLTMatching:
                alg.HLTPt = "match_" + chain_out + "_HLTpt_%SYS%"
                alg.HLTEta = "match_" + chain_out + "_HLTeta_%SYS%"
                alg.HLTPhi = "match_" + chain_out + "_HLTphi_%SYS%"
                alg.HLTDR = "match_" + chain_out + "_HLTdr_%SYS%"
                alg.HLTThreshold = "match_" + chain_out + "_HLTthresholds_%SYS%"
                for var in ["HLTpt", "HLTeta", "HLTphi", "HLTdr", "HLTthresholds"]:
                    config.addOutputVar (self.containerName,
                                         "match_" + chain_out + "_" + var + "_%SYS%",
                                         "match_" + chain_out + "_" + var, noSys=True)
