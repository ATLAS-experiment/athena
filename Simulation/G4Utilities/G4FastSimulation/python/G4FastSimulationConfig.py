# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SimpleFastKillerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName" , "BeampipeFwdCut")
    result.setPrivateTools(CompFactory.SimpleFastKillerTool(name="SimpleFastKiller", **kwargs))
    return result


def DeadMaterialShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionName", "DeadMaterial")
    result.setPrivateTools(CompFactory.DeadMaterialShowerTool(name="DeadMaterialShower", **kwargs))
    return result


def FatrasG4Cfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Name of region where FatrasG4 will be triggered
    kwargs.setdefault("RegionName", "InDet")

    # Set the ActsFatrasG4Tool part
    from G4AtlasTools.G4AtlasToolsConfig import ActsFatrasG4ToolCfg
    if "ActsFatrasG4Tool" not in kwargs:        
        kwargs.setdefault("ActsFatrasG4Tool", result.addPublicTool(result.popToolsAndMerge(ActsFatrasG4ToolCfg(flags))))
    
    fatrasG4Tool = CompFactory.FatrasG4Tool(name="FatrasG4", **kwargs)
    # declare produced data
    # ExtraOutputs is now declared in SimHitContainerListCfg in G4AtlasToolsConfig.py, and called from G4AtlasAlgConfig.py
    result.setPrivateTools(fatrasG4Tool)

    return result

def AFatrasG4Cfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Name of region where AFatrasG4 will be triggered
    kwargs.setdefault("RegionName", "InDet")

    # Set the ActsFatrasG4Tool part
    from G4AtlasTools.G4AtlasToolsConfig import ActsFatrasG4ToolCfg
    if "ActsFatrasG4Tool" not in kwargs:
        kwargs.setdefault("ActsFatrasG4Tool", result.addPublicTool(result.popToolsAndMerge(ActsFatrasG4ToolCfg(flags))))

    aFatrasG4Tool = CompFactory.AFatrasG4Tool(name="AFatrasG4", **kwargs)
    # declare produced data
    # ExtraOutputs is now declared in SimHitContainerListCfg in G4AtlasToolsConfig.py, and called from G4AtlasAlgConfig.py
    result.setPrivateTools(aFatrasG4Tool)

    return result

def FastCaloSimCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Set the parametrization service
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ParamSvcCfg
    kwargs.setdefault("ISF_FastCaloSimV2ParamSvc", result.getPrimaryAndMerge(FastCaloSimV2ParamSvcCfg(flags)))
    # Set the FastCaloSim extrapolation tool
    from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import FastCaloSimCaloExtrapolationCfg
    kwargs.setdefault("FastCaloSimCaloExtrapolation", result.addPublicTool(result.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))
    # Name of region where FastCaloSim will be triggered
    kwargs.setdefault("RegionName", "CALO")
    kwargs.setdefault('CaloCellContainerSDName', "ToolSvc.SensitiveDetectorMasterTool.CaloCellContainerSD")
    
    # Set the G4CaloTransportTool
    from G4AtlasTools.G4AtlasToolsConfig import G4CaloTransportToolCfg
    kwargs.setdefault("G4CaloTransportTool", result.addPublicTool(result.popToolsAndMerge(G4CaloTransportToolCfg(flags))))

    # Set the PunchThrough G4 part
    from G4AtlasTools.G4AtlasToolsConfig import PunchThroughSimWrapperCfg
    if "PunchThroughSimWrapper" not in kwargs:
        kwargs.setdefault("PunchThroughSimWrapper", result.addPublicTool(result.popToolsAndMerge(PunchThroughSimWrapperCfg(flags))))
    
    # Config PunchThroughG4Tool
    kwargs.setdefault('doPunchThrough', flags.Sim.FastCalo.doPunchThrough)

    result.setPrivateTools(CompFactory.FastCaloSimTool(name="FastCaloSim", **kwargs))
    return result
