# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonBlueprintNodeBuilderCfg(flags, name = "MuonBlueprintNodeBuilder", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(flags))
    the_tool = CompFactory.ActsTrk.MuonBlueprintNodeBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

