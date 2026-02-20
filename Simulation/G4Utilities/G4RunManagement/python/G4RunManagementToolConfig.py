# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from G4AtlasServices.G4AtlasUserActionConfig import UserActionSvcCfg
from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg, UserLimitsSvcCfg
from G4AtlasTools.G4GeometryToolConfig import G4AtlasDetectorConstructionToolCfg
from SimulationConfig.SimEnums import LArParameterization

def G4RunToolCfg(flags, name="G4RunTool", **kwargs):
    result = ComponentAccumulator()
    
    # Configure service handles
    kwargs.setdefault("DetectorConstruction", result.popToolsAndMerge(G4AtlasDetectorConstructionToolCfg(flags)))
    kwargs.setdefault("PhysicsListSvc", result.getPrimaryAndMerge(PhysicsListSvcCfg(flags)))
    kwargs.setdefault("UserLimitsSvc", result.getPrimaryAndMerge(UserLimitsSvcCfg(flags)))
    kwargs.setdefault("UserActionSvc", result.getPrimaryAndMerge(UserActionSvcCfg(flags)))
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        from G4AtlasTools.G4AtlasToolsConfig import PunchThroughG4ToolCfg
        physics_initialization_tools = kwargs.setdefault("PhysicsInitializationTools", [])
        physics_initialization_tools.append(result.addPublicTool(result.popToolsAndMerge(PunchThroughG4ToolCfg(flags))))

    # Threading configuration
    kwargs.setdefault("NG4threads", max(1, flags.Concurrency.NumThreads))
    kwargs.setdefault("NG4eventsPerRun", 100000)

    result.setPrivateTools(CompFactory.G4RunTool(name, **kwargs))
    return result