# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonGeoUtilityToolCfg(flags, name = "MuonGeoUtilityTool", **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.MuonGMR4.MuonGeoUtilityTool(name, **kwargs)
    result.addPublicTool(the_tool, primary = True)
    return result
def MdtReadoutGeomToolCfg(flags, name="MdtReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.MdtReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def RpcReadoutGeomToolCfg(flags, name="RpcReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.RpcReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def TgcReadoutGeomToolCfg(flags, name="TgcReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.TgcReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def sTgcReadoutGeomToolCfg(flags, name="sTgcReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.sTgcReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MmReadoutGeomToolCfg(flags, name="MmReadoutGeomTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.MmReadoutGeomTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def ChamberAssebmbleToolCfg(flags,name="MuonChamberAssembleTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeoUtilTool", result.getPrimaryAndMerge(MuonGeoUtilityToolCfg(flags)))
    the_tool = CompFactory.MuonGMR4.ChamberAssembleTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result
def MuonDetectorToolCfg(flags, name="MuonDetectorToolR4", **kwargs):
    result = ComponentAccumulator()
    sub_detTools = []
    if flags.Detector.GeometryMDT:
        sub_detTools.append(result.popToolsAndMerge(MdtReadoutGeomToolCfg(flags)))

    if flags.Detector.GeometryRPC:
        sub_detTools.append(result.popToolsAndMerge(RpcReadoutGeomToolCfg(flags)))

    if flags.Detector.GeometryTGC:
        sub_detTools.append(result.popToolsAndMerge(TgcReadoutGeomToolCfg(flags)))

    if flags.Detector.GeometrysTGC:
        sub_detTools.append(result.popToolsAndMerge(sTgcReadoutGeomToolCfg(flags)))

    if flags.Detector.GeometryMM:
        sub_detTools.append(result.popToolsAndMerge(MmReadoutGeomToolCfg(flags)))
    
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep is not ProductionStep.Simulation:
        sub_detTools.append(result.popToolsAndMerge(ChamberAssebmbleToolCfg(flags)))
    kwargs.setdefault("ReadoutEleBuilders", sub_detTools)
    the_tool = CompFactory.MuonGMR4.MuonDetectorTool(name = name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonGeoModelCfg(flags):
    result = ComponentAccumulator()
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    geoModelSvc = result.getPrimaryAndMerge(GeoModelCfg(flags))
    geoModelSvc.DetectorTools+=[result.popToolsAndMerge(MuonDetectorToolCfg(flags))]
    return result


