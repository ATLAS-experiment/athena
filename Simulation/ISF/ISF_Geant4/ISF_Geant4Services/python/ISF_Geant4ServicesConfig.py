"""Geant4 services config for ISF with ComponentAccumulator

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from ISF_Geant4Tools.ISF_Geant4ToolsConfig import (
    Geant4ToolCfg, FullGeant4ToolCfg, PassBackGeant4ToolCfg,
    AFIIGeant4ToolCfg
)


def Geant4SimCfg(flags, name="ISFG4SimSvc", **kwargs):
    result = ComponentAccumulator()

    G4_DDDBEnvelopeDefSvc = CompFactory.DetDescrDBEnvelopeSvc("G4EnvelopeDefSvc")
    G4_DDDBEnvelopeDefSvc.DBBeamPipeNode = "BPipeEnvelopeG4"
    G4_DDDBEnvelopeDefSvc.DBInDetNode = "InDetEnvelopeG4" if flags.GeoModel.Run < LHCPeriod.Run4 else "ITkEnvelopeG4"
    G4_DDDBEnvelopeDefSvc.DBCaloNode = "CaloEnvelopeG4"
    G4_DDDBEnvelopeDefSvc.DBMSNode = "MuonEnvelopeG4"
    G4_DDDBEnvelopeDefSvc.DBCavernNode = "CavernEnvelopeG4"
    result.addService(G4_DDDBEnvelopeDefSvc)

    if "SimulatorTool" not in kwargs:
        kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(Geant4ToolCfg(flags))))
    kwargs.setdefault("Identifier", "Geant4")
    result.addService(CompFactory.iGeant4.Geant4SimSvc(name, **kwargs), primary = True)
    return result


def FullGeant4SimCfg(flags, name="ISF_FullGeant4SimSvc", **kwargs):
    result = ComponentAccumulator()
    if "SimulatorTool" not in kwargs:
        kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(FullGeant4ToolCfg(flags))))
    svc = result.getPrimaryAndMerge(Geant4SimCfg(flags, name, **kwargs))
    result.addService(svc, primary = True)
    return result


def PassBackGeant4SimCfg(flags, name="ISF_PassBackGeant4SimSvc", **kwargs):
    result = ComponentAccumulator()
    if "SimulatorTool" not in kwargs:
        kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(PassBackGeant4ToolCfg(flags))))
    svc = result.getPrimaryAndMerge(Geant4SimCfg(flags, name, **kwargs))
    result.addService(svc, primary = True)
    return result


def AFIIGeant4SimCfg(flags, name="ISF_AFIIGeant4SimSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(AFIIGeant4ToolCfg(flags))))
    svc = result.getPrimaryAndMerge(PassBackGeant4SimCfg(flags, name, **kwargs))
    result.addService(svc, primary = True)
    return result


def AFII_QS_Geant4SimCfg(flags, name="ISF_AFII_QS_Geant4SimSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("SimulatorTool", result.addPublicTool(result.popToolsAndMerge(AFIIGeant4ToolCfg(flags))))
    svc = result.getPrimaryAndMerge(PassBackGeant4SimCfg(flags, name, **kwargs))
    result.addService(svc, primary = True)
    return result
