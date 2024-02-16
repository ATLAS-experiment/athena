# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def TriggerMatchingToolCfg(flags, name="TriggerMatchingTool", **kwargs):
    result = ComponentAccumulator()
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    kwargs.setdefault("TrigDecisionTool", result.getPrimaryAndMerge(TrigDecisionToolCfg(flags)))

    if flags.GeoModel.Run == LHCPeriod.Run3:
        matching_tool = CompFactory.Trig.R3MatchingTool(name, **kwargs)
    else:
        matching_tool = CompFactory.Trig.MatchingTool(name, **kwargs)

    result.setPrivateTools(matching_tool)
    return result
