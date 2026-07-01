# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonBlueprintNodeBuilderCfg(flags, name = "MuonBlueprintNodeBuilder", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg
    from AthenaConfiguration.Enums import LHCPeriod
    result.merge(MuonGeoModelCfg(flags))
    kwargs.setdefault("run4Layout", flags.GeoModel.Run >= LHCPeriod.Run4)
    kwargs.setdefault("AssignActiveMaterial", flags.Muon.trackGeometryActiveMaterial)
    kwargs.setdefault("BuildPassiveVolumes", flags.Muon.trackGeometryPassiveMaterial)
    the_tool = CompFactory.ActsTrk.MuonBlueprintNodeBuilder(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonMaterialDecoratorToolCfg(flags, name = "MuonMaterialDecoratorTool", **kwargs):    
    result = ComponentAccumulator()
    kwargs.setdefault('MuonMaterialDbFile', flags.Muon.trackGeometryMaterialMap)
    the_tool = CompFactory.MuonGMR4.MuonMaterialDecoratorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
