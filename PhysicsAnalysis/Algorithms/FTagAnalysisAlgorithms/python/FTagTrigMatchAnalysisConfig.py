# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.Logging import logging

from AthenaConfiguration.Enums import LHCPeriod


from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import TriggerAnalysisBlock
from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import trigger_set

from FTagAnalysisAlgorithms.BJetTriggerByYearContent import getDecoByTrigName


def makeJetManagerTool(config, containerName):
    jmTool = config.createPublicTool("Trig::JetManagerTool", containerName)
    jmTool.JetContainerName = containerName
    jmTool.BTaggingLink = 'btaggingLink'
    jmTool.LHCPeriod = 2 if config.geometry() is LHCPeriod.Run2 else 3
    return jmTool

def makeEmulationTool(config, toBeEmulatedTriggers):
    toolName = "TrigBtagEmulationTool"
    decisionToolName = "TrigDecisionTool"
    if toolName in config._algorithms:
        return config._algorithms[toolName]

    ### determine trigger thresholds from to be emulated chain names
    chainDefinitions = {}
    for chain in toBeEmulatedTriggers:
        from ROOT.ChainNameParser import HLTChainInfo
        chainInfo = HLTChainInfo(chain)
        chainParts = ['L1item:' + chainInfo.l1Item()]
        for legInfo in chainInfo:
            eta = '0eta320'
            tagger = ''
            gscthreshold = '-99999'
            for part in legInfo.legParts:
                if part.startswith('b'):
                    tagger = part
                elif 'eta' in part:
                    eta = part
                elif part.startswith('gsc'):
                    gscthreshold = str(part)[3:]
            partDefinition = ''
            partDefinition += 'L1threshold:' + chainInfo.l1Item()
            partDefinition += '|name:' + legInfo.legName()
            partDefinition += '|multiplicity:' + str(legInfo.multiplicity)
            partDefinition += '|threshold:' + str(legInfo.threshold)
            partDefinition += '|etaRange:' + eta
            partDefinition += '|jvt:-99999'
            partDefinition += '|tagger:' + tagger
            partDefinition += '|jetpresel:nopresel'
            partDefinition += '|dijetmass:None' # TODO: add invm chain support
            partDefinition += '|isPFlow:False'
            partDefinition += '|isShared:False'
            partDefinition += '|GSCthreshold:' + gscthreshold
            chainParts.append(partDefinition)
        chainDefinitions[chain] = chainParts


    decisionTool = config._algorithms[decisionToolName] if decisionToolName in config._algorithms else config.createPublicTool("Trig::TrigDecisionTool", decisionToolName)
    emulationTool = config.createPublicTool("Trig::TrigBtagEmulationTool", toolName)
    emulationTool.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"
    emulationTool.LHCPeriod = 2 if config.geometry() is LHCPeriod.Run2 else 3

    if config.geometry() is LHCPeriod.Run2:
        from Campaigns.Utils import Campaign
        InputJetContainer_a4tcemsubjesJet = 'HLT_xAOD__JetContainer_a4tcemsubjesFS' if config.campaign() == Campaign.MC20a else 'HLT_xAOD__JetContainer_a4tcemsubjesISFS'
        InputJetContainer_SplitJet = 'HLT_xAOD__JetContainer_SplitJet'
        InputJetContainer_GSCJet = 'HLT_xAOD__JetContainer_GSCJet'


        emulationTool.JM_a4tcemsubjes_CNT = makeJetManagerTool(config, InputJetContainer_a4tcemsubjesJet)
        emulationTool.JM_Split_CNT = makeJetManagerTool(config, InputJetContainer_SplitJet)
        emulationTool.JM_GSC_CNT = makeJetManagerTool(config, InputJetContainer_GSCJet)

        working_points = {
                "mv2c2040": 0.75,
                "mv2c2050": 0.50,
                "mv2c2060": -0.0224729,
                "mv2c2070": -0.509032,
                "mv2c2077": -0.764668,
                "mv2c2085": -0.938441,
                }
        working_points.update({
            "mv2c1040": 0.978,
            "mv2c1050": 0.948,
            "mv2c1060": 0.846,
            "mv2c1070": 0.580,
            "mv2c1077": 0.162,
            "mv2c1085": -0.494
            })

        emulationTool.EmulatedChainDefinitions = chainDefinitions

        emulationTool.WorkingPoints = working_points

    return emulationTool




class FTagJetTrigMatchingBlock(ConfigBlock):
    """the ConfigBlock for the FTAG jet trigger matching"""
    def __init__(self):
        super(FTagJetTrigMatchingBlock, self).__init__()
        self.addOption('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('triggerChainsPerYear', {}, type=dict,
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

        # Need to split container name from selections, to support AnaJets.baselineJvt
        jetContainer = self.containerName.split('.')[0]

        if self.triggerChainsPerYear:
            triggers = trigger_set(config, self.triggerChainsPerYear,
                                   self.includeAllYearsPerRun)
            decisionTool = TriggerAnalysisBlock.makeTriggerDecisionTool(config)

            if config.geometry() is LHCPeriod.Run2:
                emulationTool = makeEmulationTool(config, triggers)

            for chain in triggers:
                chain_noHLT = chain.replace("HLT_", "")
                chain_out = chain_noHLT if self.removeHLTPrefix else chain
                chain_out = chain_out.replace('-', '_').replace('.', 'p')

                alg = config.createAlgorithm( 'CP::BTaggingTriggerMatchingAlg',
                                              'FTagTriggerMatchingAlg' + chain_out )
                alg.TrigDecisionTool = f"{decisionTool.getType()}/{decisionTool.getName()}"
                alg.trigger = chain
                alg.useRun3TriggerEDM = config.geometry() is LHCPeriod.Run3
                if alg.useRun3TriggerEDM:
                    decors_to_check = [deco + '_pb' for deco in getDecoByTrigName(chain)]
                    log.info(f'Configured b-tagging trigger decorations for trigger {chain}: {decors_to_check}')
                    alg.ftagRun3TriggerDecoNames = decors_to_check

                # alg.OutputLevel = 1 # VERBOSE. for detailed debug
                # Helper function to implement to provide cut for given trigger
                # Only used for Run 2
                if config.geometry() is LHCPeriod.Run2:
                    #alg.btagThreshold = getBTagThreshold(chain)
                    alg.TrigBtagEmulationTool = f"{emulationTool.getType()}/{emulationTool.getName()}"

                alg.matchingDecoration = 'ftag_jetTrigMatching_' + chain_out + '_%SYS%'
                alg.bTagMatchingDecoration = 'ftag_bTagTrigMatching_' + chain_out + '_%SYS%'
                alg.jets = config.readName(jetContainer)
                alg.preselection = config.getPreselection (jetContainer, '')

                # If there is no systematically varied preselection then only nominal can be written out.
                noSys = True
                if alg.preselection:
                    noSys = False

                config.addOutputVar (jetContainer, alg.matchingDecoration,
                                     'ftag_jetTrigMatching_' + chain_out, noSys=noSys)
                config.addOutputVar (jetContainer, alg.bTagMatchingDecoration,
                                     'ftag_bTagTrigMatching_' + chain_out, noSys=noSys)
