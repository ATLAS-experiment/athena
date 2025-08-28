# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

# Jet Manager Tools
def jetManagerToolCfg(flags,
                      name: str,
                      **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str), "JetManagerTool name must be a string"
    acc = ComponentAccumulator()
    kwargs.setdefault('BTaggingLink', 'btaggingLink')
    kwargs.setdefault('LHCPeriod', 3 if flags.GeoModel.Run is LHCPeriod.Run3 else 2)
    acc.setPrivateTools(CompFactory.Trig.JetManagerTool(name, **kwargs))
    return acc

# CNT
def jetPFlowCNTManagerToolCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('JetContainerName', 'HLT_AntiKt4EMPFlowJets_subjesIS')
    return jetManagerToolCfg(flags,
                             name = "JM_PFlow_CNT",
                             **kwargs)

# PRESEL
def jetEMTopoPRESELManagerToolCfg(flags,
                                  **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('JetContainerName', 'HLT_AntiKt4EMTopoJets_subjesIS')
    return jetManagerToolCfg(flags, 
                             name = "JM_EMTopo_PRESEL",
                             **kwargs)

# Run2 HLT jet collection
def jetHLTCNTManagerToolCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('JetContainerName', 'HLT_xAOD__JetContainer_a4tcemsubjesJet')
    return jetManagerToolCfg(flags,
                             name = "JM_a4tcemsubjes_CNT",
                             **kwargs)

# Run2 jet collection with additional calibration
def jetSplitManagerToolCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('JetContainerName', 'HLT_xAOD__JetContainer_SplitJet')
    return jetManagerToolCfg(flags,
                             name = "JM_Split_CNT",
                             **kwargs)

def jetGSCManagerToolCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('JetContainerName', 'HLT_xAOD__JetContainer_GSCJet')
    return jetManagerToolCfg(flags,
                             name = "JM_GSC_CNT",
                             **kwargs)

# Emulation Tool                            
def TrigBtagEmulationToolCfg(flags, 
                             toBeEmulatedTriggers: list, 
                             InputChain_EMTopo: str = '',
                             InputJetContainer_EMTopo: str = '',
                             InputJetContainer_EMTopoPresel: str = '',
                             InputJetContainer_PFlow: str = '',
                             InputJetContainer_PFlowPresel: str = '',
                             InputJetContainer_a4tcemsubjesJet: str = '',
                             InputJetContainer_SplitJet: str = '',
                             InputJetContainer_GSCJet: str = '',
                             **kwargs) -> ComponentAccumulator:
    assert isinstance(toBeEmulatedTriggers, list)
    assert isinstance(InputChain_EMTopo, str)
    assert isinstance(InputJetContainer_EMTopo, str)
    assert isinstance(InputJetContainer_EMTopoPresel, str)
    assert isinstance(InputJetContainer_PFlow, str)
    assert isinstance(InputJetContainer_PFlowPresel, str)
    assert isinstance(InputJetContainer_a4tcemsubjesJet, str)
    assert isinstance(InputJetContainer_SplitJet, str)
    assert isinstance(InputJetContainer_GSCJet, str)

    from AthenaCommon.Logging import logging
    log = logging.getLogger('TrigBtagEmulationToolCfg')

    period = -1
    if flags.GeoModel.Run is LHCPeriod.Run2:
        period = 2
        from Campaigns.Utils import Campaign, getMCCampaign
        campaign = getMCCampaign(flags.Input.Files)
        if len(InputJetContainer_a4tcemsubjesJet) == 0:
            InputJetContainer_a4tcemsubjesJet = 'HLT_xAOD__JetContainer_a4tcemsubjesFS' if campaign == Campaign.MC20a or flags.Input.DataYear == 2016 else 'HLT_xAOD__JetContainer_a4tcemsubjesISFS'
        if len(InputJetContainer_SplitJet) == 0:
            InputJetContainer_SplitJet = 'HLT_xAOD__JetContainer_SplitJet'
        if len(InputJetContainer_GSCJet) == 0:
            if campaign != Campaign.MC20a and flags.Input.DataYear != 2016:
                InputJetContainer_GSCJet = 'HLT_xAOD__JetContainer_GSCJet'
        # check if btagging link exists for DAOD input
        if "StreamAOD" not in flags.Input.ProcessingTags:
            import PyUtils.PoolFile as PF
            PF.PoolOpts.FAST_MODE = True
            for fname in flags.Input.Files:
                pool_file = PF.PoolFile(fname, verbose=False)
                btag_link_exists = any("BTagBtagToJetAssociator" in d.name for d in pool_file.data)
                if not btag_link_exists:
                    log.error(f"BTagBtagToJetAssociator not found in input file {fname}. Emulation of b-jet chains will be incorrect.")
    elif flags.GeoModel.Run is LHCPeriod.Run3:
        period = 3
        if len(InputChain_EMTopo) == 0:
            InputChain_EMTopo = 'HLT_j45_subjesgsc_ftf_L1J15'
        if len(InputJetContainer_EMTopo) == 0:
            InputJetContainer_EMTopo = 'HLT_AntiKt4EMTopoJets_subjesIS'
        if len(InputJetContainer_EMTopoPresel) == 0:
            InputJetContainer_EMTopoPresel = 'HLT_AntiKt4EMTopoJets_subjesIS'
        if len(InputJetContainer_PFlow) == 0:
            InputJetContainer_PFlow = 'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf'
        if len(InputJetContainer_PFlowPresel) == 0:
            InputJetContainer_PFlowPresel = 'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf'
    else:
        raise ValueError(f"Unsupported LHC Period {flags.GeoModel.Run}.")

    ### determine trigger thresholds from to be emulated chain names
    chainDefinitions = {}
    for chain in toBeEmulatedTriggers:
        if period == 3:
            from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName
            from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
            chainDict = dictFromChainName(flags, chain)
            chainParts = ['L1item:' + chainDict['L1item']]
            for chainPart in chainDict['chainParts']:
                # Gaudi::Property has limited support for nested structures, put everything in a long string and parse in C++
                # and update TrigBtagEmulationChain::parseChainDefinition(...) accordingly
                partDefinition = ''
                partDefinition += 'L1threshold:' + chainPart['L1threshold']
                partDefinition += '|name:' + chainPart['chainPartName']
                partDefinition += '|multiplicity:' + chainPart['multiplicity']
                partDefinition += '|threshold:' + chainPart['threshold']
                partDefinition += '|etaRange:' + chainPart['etaRange']
                partDefinition += '|jvt:' + (chainPart['jvt'] if chainPart['jvt'] else '-99999')
                partDefinition += '|tagger:' + chainPart['bTag']
                partDefinition += '|jetpresel:' + chainPart['trkpresel']
                partDefinition += '|dijetmass:' + (chainPart['hypoScenario'][len('DJMASS'):] if ('DJMASS' in chainPart['hypoScenario']) else 'None')
                partDefinition += '|isPFlow:' + ('True' if (chainPart['constitType'] == 'pf') else 'False')
                partDefinition += '|isShared:' + ('True' if ('SHARED' in chainPart['chainPartName']) else 'False')
                partDefinition += '|GSCthreshold:-99999'
                chainParts.append(partDefinition)
            chainName = chain.name if isinstance(chain, ChainProp) else chain
            chainDefinitions[chainName] = chainParts
        elif period == 2:
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

    ### all trigger thresholds parsed

    # Component Accumulator
    acc = ComponentAccumulator()

    ### add TriggerDecisionTool
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    ### set tool options
    kwargs.setdefault('EmulatedChainDefinitions', chainDefinitions)
    kwargs.setdefault('TrigDecisionTool', tdt)
    kwargs.setdefault('LHCPeriod', period)

    if period == 3:
        kwargs.setdefault('JM_PFlow_CNT' , acc.popToolsAndMerge(jetPFlowCNTManagerToolCfg(flags,
                                                                                          JetContainerName=InputJetContainer_PFlow)))

        kwargs.setdefault('JM_EMTopo_PRESEL', acc.popToolsAndMerge(jetEMTopoPRESELManagerToolCfg(flags,
                                                                                                 JetContainerName=InputJetContainer_EMTopoPresel)))
        from TrigBjetHypo.TrigBjetBtagHypoTool import bTaggingWP
        from TrigBjetHypo.TrigBjetBtagHypoTool import bbTaggingWP

        working_points = { "newTagger" : 1.234 }
        working_points.update(bTaggingWP)
        working_points.update(bbTaggingWP)
        kwargs.setdefault('FTD_Remapping', 
                          {"DL1d20210519r22_pu" : "DL1dEMUL_pu",
                           "DL1d20210519r22_pc" : "DL1dEMUL_pc",
                           "DL1d20210519r22_pb" : "DL1dEMUL_pb"})

    elif period == 2:
        kwargs.setdefault('JM_a4tcemsubjes_CNT' , acc.popToolsAndMerge(jetHLTCNTManagerToolCfg(flags,
                                                                                          JetContainerName=InputJetContainer_a4tcemsubjesJet)))

        kwargs.setdefault('JM_Split_CNT' , acc.popToolsAndMerge(jetSplitManagerToolCfg(flags,
                                                                                          JetContainerName=InputJetContainer_SplitJet)))

        kwargs.setdefault('JM_GSC_CNT' , acc.popToolsAndMerge(jetGSCManagerToolCfg(flags,
                                                                                          JetContainerName=InputJetContainer_GSCJet)))
        # Run2 taggers from https://twiki.cern.ch/twiki/bin/view/Atlas/OnlineBTaggingBenchmarks
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

    kwargs.setdefault('WorkingPoints', working_points)
    acc.setPrivateTools(CompFactory.Trig.TrigBtagEmulationTool('TrigBtagEmulationTool', **kwargs))
    return acc

def TrigBtagValidationTestCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    histSvc = CompFactory.THistSvc()
    histSvc.Output += ["VALIDATION DATAFILE='validation.root' OPT='RECREATE'"]
    acc.addService(histSvc)

    options = {}
    from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
    options['EmulatedChains'] = [chain.name if isinstance(chain, ChainProp) else chain for chain in kwargs['toBeEmulatedTriggers']]
    options['TrigBtagEmulationTool'] = acc.popToolsAndMerge( TrigBtagEmulationToolCfg(flags,
                                                                                      **kwargs))

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    options['TrigDecisionTool'] = tdt

    acc.addEventAlgo( CompFactory.Trig.TrigBtagValidationTest('TrigBtagValidation', **options) )
    return acc
