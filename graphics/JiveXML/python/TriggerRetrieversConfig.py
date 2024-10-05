# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def LVL1ResultRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.LVL1ResultRetriever(name="LVL1ResultRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def TriggerInfoRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.TriggerInfoRetriever(name="TriggerInfoRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODEmTauROIRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODEmTauROIRetriever(name="xAODEmTauROIRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODJetROIRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODJetROIRetriever(name="xAODJetROIRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODMuonROIRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODMuonROIRetriever(name="xAODMuonROIRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODTriggerTowerRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODTriggerTowerRetriever(
        name="xAODTriggerTowerRetriever"
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def TriggerRetrieversCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Reco.EnableTrigger:
        #--- LVL1 result from TrigDecision
        tools += [result.getPrimaryAndMerge(LVL1ResultRetrieverCfg(flags))]

        #--- TriggerInfo (Etmiss, etc)
        tools += [result.getPrimaryAndMerge(TriggerInfoRetrieverCfg(flags))]

        # new xAOD retrievers
        tools += [result.getPrimaryAndMerge(xAODEmTauROIRetrieverCfg(flags))]
        tools += [result.getPrimaryAndMerge(xAODJetROIRetrieverCfg(flags))]
        tools += [result.getPrimaryAndMerge(xAODMuonROIRetrieverCfg(flags))]
        tools += [result.getPrimaryAndMerge(xAODTriggerTowerRetrieverCfg(flags))]

    return result, tools
