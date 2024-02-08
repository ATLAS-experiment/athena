# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SimpleFastKillerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionNames" , ["BeampipeFwdCut"] )
    result.setPrivateTools(CompFactory.SimpleFastKillerTool(name="SimpleFastKiller", **kwargs))
    return result


def DeadMaterialShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("RegionNames",        ["DeadMaterial"])
    result.setPrivateTools(CompFactory.DeadMaterialShowerTool(name="DeadMaterialShower", **kwargs))
    return result


def FastCaloSimCfg(flags, **kwargs):
    result = ComponentAccumulator()
    # Set the parametrization service
    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2ParamSvcCfg
    kwargs.setdefault("ISF_FastCaloSimV2ParamSvc", result.getPrimaryAndMerge(FastCaloSimV2ParamSvcCfg(flags)).name)
    # Set the FastCaloSim extrapolation tool
    from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import FastCaloSimCaloExtrapolationCfg
    kwargs.setdefault("FastCaloSimCaloExtrapolation", result.addPublicTool(result.popToolsAndMerge(FastCaloSimCaloExtrapolationCfg(flags))))
    # Name of region where FastCaloSim will be triggered
    kwargs.setdefault("RegionNames", ["CALO"])
    kwargs.setdefault('CaloCellContainerSDName', "ToolSvc.SensitiveDetectorMasterTool.CaloCellContainerSD")
    result.setPrivateTools(CompFactory.FastCaloSimTool(name="FastCaloSim", **kwargs))
    return result
