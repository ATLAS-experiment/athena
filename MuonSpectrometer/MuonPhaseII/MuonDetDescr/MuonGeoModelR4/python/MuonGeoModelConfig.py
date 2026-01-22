# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("run4Layout", flags.GeoModel.Run >= LHCPeriod.Run4)
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

def MuonAlignStoreCfg(flags):
    result = ComponentAccumulator()
    if not flags.Muon.usePhaseIIGeoSetup: return result
    from MuonCondAlgR4.ConditionsConfig import MuonGeoAlignCondAlgCfg
    result.merge(MuonGeoAlignCondAlgCfg(flags))
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsAlignStoreProviderAlgCfg
    
    from MuonG4TrfCache.MuonTrfCacheConfig import MuonTransformCacheCfg
    result.merge(MuonTransformCacheCfg(flags))

    setCondDep = flags.Muon.enableAlignment or flags.Sim.ReleaseGeoModel
    from MuonConfig.MuonConfigFlags import GeoTrfCacheMode
    from ROOT.ActsTrk import DetectorType
    if flags.Detector.GeometryMDT:  
        result.merge(ActsAlignStoreProviderAlgCfg(flags, 
                                                  name="ActsDetAlignmentAlgMdt",
                                                  CondAlignStore="MdtActsAlignContainer" if setCondDep else "",
                                                  EventAlignStore="MdtActsAlignContainer",
                                                  SplitPhysVolCache =  flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                       flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SlopyCache,
                                                  SplitActsTrfCache = flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                      flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond,
                                                  FillAlignCache = False,                                                 
                                                  DetectorType=DetectorType.Mdt))
    if flags.Detector.GeometryRPC:  
        result.merge(ActsAlignStoreProviderAlgCfg(flags, 
                                                  name="ActsDetAlignmentAlgRpc",
                                                  CondAlignStore="RpcActsAlignContainer" if setCondDep else "",
                                                  EventAlignStore="RpcActsAlignContainer",
                                                  SplitPhysVolCache =  flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                       flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SlopyCache,
                                                  SplitActsTrfCache = flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                      flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond,
                                                  FillAlignCache = False, 
                                                  DetectorType=DetectorType.Rpc))
    if flags.Detector.GeometryTGC:  
        result.merge(ActsAlignStoreProviderAlgCfg(flags, 
                                                  name="ActsDetAlignmentAlgTgc",
                                                  CondAlignStore="TgcActsAlignContainer" if setCondDep else "",
                                                  EventAlignStore="TgcActsAlignContainer",
                                                  SplitPhysVolCache =  flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                       flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SlopyCache,
                                                  SplitActsTrfCache = flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                      flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond,
                                                  FillAlignCache = False, 
                                                  DetectorType=DetectorType.Tgc))
    if flags.Detector.GeometrysTGC: 
        result.merge(ActsAlignStoreProviderAlgCfg(flags, 
                                                  name="ActsDetAlignmentAlgSTGC",
                                                  CondAlignStore="sTgcActsAlignContainer" if setCondDep else "",
                                                  EventAlignStore="sTgcActsAlignContainer",
                                                  SplitPhysVolCache =  flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                       flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SlopyCache,
                                                  SplitActsTrfCache = flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                      flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond,
                                                  FillAlignCache = False, 
                                                  DetectorType=DetectorType.sTgc))

    if flags.Detector.GeometryMM:
        result.merge(ActsAlignStoreProviderAlgCfg(flags, 
                                                  name="ActsDetAlignmentAlgMM",
                                                  CondAlignStore="MmActsAlignContainer" if setCondDep or \
                                                                                           flags.Muon.applyMMPassivation else "",
                                                  EventAlignStore="MmActsAlignContainer",
                                                  SplitPhysVolCache =  flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                       flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SlopyCache,
                                                  SplitActsTrfCache = flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.SplitCache or
                                                                      flags.Muon.AlignedGeoTrfCacheMode == GeoTrfCacheMode.ActsSplitALineCond,
                                                  FillAlignCache = False,
                                                  DetectorType=DetectorType.Mm))


    return result

