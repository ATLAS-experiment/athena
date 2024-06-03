# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ZdcNtupleCfg(flags, name="AnalysisAlg", **kwargs):
    acc = ComponentAccumulator()

    if "TrigDecisionTool" not in kwargs:
        from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
        kwargs.setdefault("TrigDecisionTool", acc.getPrimaryAndMerge(
            TrigDecisionToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ZdcNtuple(name, **kwargs))
    return acc
