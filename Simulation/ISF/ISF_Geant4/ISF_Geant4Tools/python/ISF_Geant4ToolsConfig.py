"""Geant4 tools config for ISF with ComponentAccumulator

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from RngComps.RandomServices import AthRNGSvcCfg
from G4AtlasServices.G4AtlasServicesConfig import (
    DetectorGeometrySvcCfg, PhysicsListSvcCfg
)
from G4AtlasServices.G4AtlasUserActionConfig import (
    ISFUserActionSvcCfg, ISFFullUserActionSvcCfg,
    ISFPassBackUserActionSvcCfg, ISF_ATLFAST_UserActionSvcCfg,
)
from G4AtlasTools.G4AtlasToolsConfig import (
    SensitiveDetectorMasterToolCfg, FastSimulationMasterToolCfg
)
from ISF_Services.ISF_ServicesConfig import (
    InputConverterCfg
)


def G4RunManagerHelperCfg(flags, name="G4RunManagerHelper", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.iGeant4.G4RunManagerHelper(name, **kwargs))
    return acc


def Geant4ToolCfg(flags, name="ISF_Geant4Tool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("DetGeoSvc", acc.getPrimaryAndMerge(DetectorGeometrySvcCfg(flags)).name)

    kwargs.setdefault("RandomNumberService", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    # Only add it if it's not added already
    if "InputConverter" not in kwargs.keys():
        kwargs.setdefault("InputConverter", acc.getPrimaryAndMerge(InputConverterCfg(flags)).name)

    if "UserActionSvc" not in kwargs.keys():
        kwargs.setdefault("UserActionSvc", acc.getPrimaryAndMerge(ISFUserActionSvcCfg(flags)).name)

    kwargs.setdefault("RecordFlux", flags.Sim.RecordFlux)

    kwargs.setdefault("MultiThreading", flags.Concurrency.NumThreads > 0)
    # Set commands for the G4AtlasAlg
    kwargs.setdefault("G4Commands", flags.Sim.G4Commands)
    kwargs.setdefault("PrintTimingInfo", flags.Sim.ISF.DoTimeMonitoring)
    if "SenDetMasterTool" not in kwargs:
        kwargs.setdefault("SenDetMasterTool", acc.addPublicTool(acc.popToolsAndMerge(SensitiveDetectorMasterToolCfg(flags))))
    if "FastSimMasterTool" not in kwargs:
        kwargs.setdefault("FastSimMasterTool", acc.addPublicTool(acc.popToolsAndMerge(FastSimulationMasterToolCfg(flags))))

    # PhysicsListSvc
    kwargs.setdefault("PhysicsListSvc", acc.getPrimaryAndMerge(PhysicsListSvcCfg(flags)).name)

    # Workaround to keep other simulation flavours working while we migrate everything to be AthenaMT-compatible.
    from SimulationConfig.SimEnums import SimulationFlavour
    if flags.Sim.ISF.Simulator in [SimulationFlavour.FullG4MT, SimulationFlavour.FullG4MT_QS, SimulationFlavour.PassBackG4MT, SimulationFlavour.ATLFAST3MT, SimulationFlavour.ATLFAST3MT_QS]:
        acc.setPrivateTools(CompFactory.iGeant4.G4TransportTool(name, **kwargs))
    else:
        kwargs.setdefault("G4RunManagerHelper", acc.addPublicTool(acc.popToolsAndMerge(G4RunManagerHelperCfg(flags))))
        acc.setPrivateTools(CompFactory.iGeant4.G4LegacyTransportTool(name, **kwargs))
    return acc


def FullGeant4ToolCfg(flags, name="ISF_FullGeant4Tool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UserActionSvc", acc.getPrimaryAndMerge(ISFFullUserActionSvcCfg(flags)).name)
    FullGeant4Tool = acc.popToolsAndMerge(Geant4ToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(FullGeant4Tool)
    return acc


def PassBackGeant4ToolCfg(flags, name="ISF_PassBackGeant4Tool", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("UserActionSvc", acc.getPrimaryAndMerge(ISFPassBackUserActionSvcCfg(flags)).name)
    PassBackGeant4Tool = acc.popToolsAndMerge(Geant4ToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(PassBackGeant4Tool)
    return acc


def ATLFAST_Geant4ToolCfg(flags, name="ISF_ATLFAST_Geant4Tool", **kwargs): # TODO Rename
    acc = ComponentAccumulator()
    kwargs.setdefault("UserActionSvc", acc.getPrimaryAndMerge(ISF_ATLFAST_UserActionSvcCfg(flags)).name)
    PassBackGeant4Tool = acc.popToolsAndMerge(Geant4ToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(PassBackGeant4Tool)
    return acc
