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
    # Volumes FatrasG4 acts in, everything inside them included: the whole ID
    # envelope, and the beam pipe with Sim.FatrasG4.IncludeBeamPipe. The regions
    # nested inside are found from the geometry at run time.
    kwargs.setdefault("EnvelopeVolumes", ['IDET::IDET'] + (['BeamPipe::BeamPipe'] if flags.Sim.FatrasG4.IncludeBeamPipe else []))

    # Set the ActsFatrasG4Tool part
    from G4AtlasTools.G4AtlasToolsConfig import ActsFatrasG4ToolCfg
    if "ActsFatrasG4Tool" not in kwargs:
        kwargs.setdefault("ActsFatrasG4Tool", result.addPublicTool(result.popToolsAndMerge(ActsFatrasG4ToolCfg(flags))))

    fatrasG4Tool = CompFactory.FatrasG4Tool(name="FatrasG4", **kwargs)
    # declare produced data
    # ExtraOutputs is now declared in SimHitContainerListCfg in G4AtlasToolsConfig.py, and called from G4AtlasAlgConfig.py
    result.setPrivateTools(fatrasG4Tool)

    return result

def FatrasG4PhysicsToolCfg(flags, **kwargs):
    """Geant4 electromagnetic processes switched off where FatrasG4 runs, from
    Sim.FatrasG4.G4EMProcesses and Sim.FatrasG4.DisableAllG4Physics."""
    result = ComponentAccumulator()
    # The same volumes as FatrasG4Cfg, so that the physics is switched off exactly where FatrasG4 acts
    kwargs.setdefault("EnvelopeVolumes", ['IDET::IDET'] + (['BeamPipe::BeamPipe'] if flags.Sim.FatrasG4.IncludeBeamPipe else []))
    kwargs.setdefault("DisableAllG4Physics", flags.Sim.FatrasG4.DisableAllG4Physics)
    kwargs.setdefault("InactiveProcesses",
                      [name for name, active in flags.Sim.FatrasG4.G4EMProcesses.items() if not active])
    result.setPrivateTools(CompFactory.FatrasG4PhysicsTool(name="FatrasG4PhysicsTool", **kwargs))
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
