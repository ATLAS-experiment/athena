# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign, getMCCampaign
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg

# Jet Manager Tools
def jetManagerToolCfg(flags,
                      name: str,
                      **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str), "JetManagerTool name must be a string"
    acc = ComponentAccumulator()
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
    campaign = getMCCampaign(flags.Input.Files)
    a4tcemsubjesJet = ('HLT_xAOD__JetContainer_a4tcemsubjesFS'
                       if campaign is Campaign.MC20a or flags.Input.DataYear == 2016
                       else 'HLT_xAOD__JetContainer_a4tcemsubjesISFS')
    kwargs.setdefault('JetContainerName', a4tcemsubjesJet)

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
def TrigBtagEmulationToolCfg(flags, toBeEmulatedTriggers: list,
                             **kwargs) -> ComponentAccumulator:
    assert isinstance(toBeEmulatedTriggers, list)

    from AthenaCommon.Logging import logging
    log = logging.getLogger('TrigBtagEmulationToolCfg')

    if flags.GeoModel.Run is LHCPeriod.Run2:
        # check if btagging link exists for DAOD input
        if "StreamAOD" not in flags.Input.ProcessingTags:
            import PyUtils.PoolFile as PF
            PF.PoolOpts.FAST_MODE = True
            for fname in flags.Input.Files:
                pool_file = PF.PoolFile(fname, verbose=False)
                btag_link_exists = any("BTagBtagToJetAssociator" in d.name for d in pool_file.data)
                if not btag_link_exists:
                    log.error(f"BTagBtagToJetAssociator not found in input file {fname}. Emulation of b-jet chains will be incorrect.")

    if not(flags.GeoModel.Run in [LHCPeriod.Run2, LHCPeriod.Run3]):
        raise ValueError(f"Unsupported LHC Period {flags.GeoModel.Run}.")

    from TrigBtagEmulationTool.TrigBtagEmulationToolHelpers import (
        TrigBtagEmulation_kwargs)
    kwargs = TrigBtagEmulation_kwargs(flags, toBeEmulatedTriggers, **kwargs)

    # Component Accumulator
    acc = ComponentAccumulator()

    ### add TriggerDecisionTool
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    kwargs.setdefault('TrigDecisionTool', tdt)

    if flags.GeoModel.Run is LHCPeriod.Run3:
        kwargs.setdefault('JM_PFlow_CNT' , acc.popToolsAndMerge(jetPFlowCNTManagerToolCfg(flags)))
        kwargs.setdefault('JM_EMTopo_PRESEL', acc.popToolsAndMerge(jetEMTopoPRESELManagerToolCfg(flags)))

    elif flags.GeoModel.Run is LHCPeriod.Run2:
        kwargs.setdefault('JM_a4tcemsubjes_CNT' , acc.popToolsAndMerge(jetHLTCNTManagerToolCfg(flags)))
        kwargs.setdefault('JM_Split_CNT' , acc.popToolsAndMerge(jetSplitManagerToolCfg(flags)))
        campaign = getMCCampaign(flags.Input.Files)
        if not(campaign is Campaign.MC20a or flags.Input.DataYear == 2016):
            kwargs.setdefault('JM_GSC_CNT' , acc.popToolsAndMerge(jetGSCManagerToolCfg(flags)))

    acc.setPrivateTools(CompFactory.Trig.TrigBtagEmulationTool('TrigBtagEmulationTool', **kwargs))
    return acc

def TrigBtagValidationTestCfg(flags,
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    histSvc = CompFactory.THistSvc()
    histSvc.Output += ["VALIDATION DATAFILE='validation.root' OPT='RECREATE'"]
    acc.addService(histSvc)

    options = {}
    options['EmulatedChains'] = [chain if isinstance(chain, str) else chain.name for chain in kwargs['toBeEmulatedTriggers']]
    options['TrigBtagEmulationTool'] = acc.popToolsAndMerge( TrigBtagEmulationToolCfg(flags,
                                                                                      **kwargs))

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(flags))
    options['TrigDecisionTool'] = tdt

    acc.addEventAlgo( CompFactory.Trig.TrigBtagValidationTest('TrigBtagValidation', **options) )
    return acc
