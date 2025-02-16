#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg, TGCCablingConfigCfg
from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg

def RPCRecRoiToolCfg(flags, name="RPCRecRoiTool", useRun3Config=True):
    acc = RPCCablingConfigCfg(flags)
    acc.merge(MuonGeoModelCfg(flags))

    tool = CompFactory.getComp("LVL1::TrigT1RPCRecRoiTool")(name)
    tool.UseRun3Config = useRun3Config
    acc.setPrivateTools(tool)

    return acc

def TGCRecRoiToolCfg(flags, name="TGCRecRoiTool", useRun3Config=True):
    acc = TGCCablingConfigCfg(flags)
    acc.merge(MuonGeoModelCfg(flags))

    tool = CompFactory.getComp("LVL1::TrigT1TGCRecRoiTool")(name)
    tool.UseRun3Config = useRun3Config
    acc.setPrivateTools(tool)

    return acc
