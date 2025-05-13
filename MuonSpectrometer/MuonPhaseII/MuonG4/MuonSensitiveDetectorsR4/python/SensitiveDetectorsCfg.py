# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MdtSensitiveDetectorToolCfg(flags, name = "MdtSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawMdtSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::MDTDriftGas"])
    the_tool = CompFactory.MuonG4R4.MdtSensitiveDetectorTool(name, **kwargs)
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="MdtSimHitSorterAlg",
                                               InContainers=["xRawMdtSimHits"],
                                               OutContainer ="xMdtSimHits",
                                               deepCopy = True))
    result.setPrivateTools(the_tool)
    return result

def MmSensitiveDetectorToolCfg(flags, name = "MmSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawMmSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::actMicroMegaGas"])
    the_tool = CompFactory.MuonG4R4.MmSensitiveDetectorTool(name, **kwargs)
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="MmSimHitSorterAlg",
                                               InContainers=["xRawMmSimHits"],
                                               OutContainer ="xMmSimHits",
                                               deepCopy = True))
    result.setPrivateTools(the_tool)
    return result

def RpcSensitiveDetectorToolCfg(flags, name = "RpcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawRpcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::RpcGasGap"])
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="RpcSimHitSorterAlg",
                                               InContainers=["xRawRpcSimHits"],
                                               OutContainer ="xRpcSimHits",
                                               deepCopy = True))
    the_tool = CompFactory.MuonG4R4.RpcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def TgcSensitiveDetectorToolCfg(flags, name = "TgcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawTgcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::TgcGas"])
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="TgcSimHitSorterAlg",
                                               InContainers=["xRawTgcSimHits"],
                                               OutContainer ="xTgcSimHits",
                                               deepCopy = True))
    the_tool = CompFactory.MuonG4R4.TgcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def sTgcSensitiveDetectorToolCfg(flags, name = "sTgcSensitiveDetector", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OutputCollectionNames", [ "xRawStgcSimHits"])
    kwargs.setdefault("LogicalVolumeNames", ["MuonR4::sTgcGas"])
    from MuonSimHitSorting.MuonSimHitSortingCfg import MuonSimHitSortingAlgCfg
    result.merge(MuonSimHitSortingAlgCfg(flags,name="sTgcSimHitSorterAlg",
                                               InContainers=["xRawStgcSimHits"],
                                               OutContainer ="xStgcSimHits",
                                               deepCopy = True))
    the_tool = CompFactory.MuonG4R4.sTgcSensitiveDetectorTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


def SetupSensitiveDetectorsCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableMDT:
        tools += [result.popToolsAndMerge(MdtSensitiveDetectorToolCfg(flags))]

    if flags.Detector.EnableRPC:
        tools += [result.popToolsAndMerge(RpcSensitiveDetectorToolCfg(flags))]

    if flags.Detector.EnableMM:
        tools += [result.popToolsAndMerge(MmSensitiveDetectorToolCfg(flags))]

    if flags.Detector.EnableTGC:
        tools += [result.popToolsAndMerge(TgcSensitiveDetectorToolCfg(flags))]
    if flags.Detector.EnablesTGC:
        tools += [result.popToolsAndMerge(sTgcSensitiveDetectorToolCfg(flags))]
    result.setPrivateTools(tools)
    return result

def SimHitContainerListCfg(flags):
    simHitContainers = []
    if flags.Detector.EnableMDT:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMdtSimHits")]
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('MUON', True)):
            simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMdtSimHits_G4")]
            if (not ('G4MS' in flags.Sim.ISF.Simulator.value and 'ACTS' in flags.Sim.ISF.Simulator.value)):
                simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMdtSimHits_Fatras")]
    if flags.Detector.EnableMM:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMmSimHits")]
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('MUON', True)):
            simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMmSimHits_G4")]
            if (not ('G4MS' in flags.Sim.ISF.Simulator.value and 'ACTS' in flags.Sim.ISF.Simulator.value)):
                simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawMmSimHits_Fatras")]
    if flags.Detector.EnableRPC:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawRpcSimHits")]
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('MUON', True)):
            simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawRpcSimHits_G4")]
            if (not ('G4MS' in flags.Sim.ISF.Simulator.value and 'ACTS' in flags.Sim.ISF.Simulator.value)):
                simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawRpcSimHits_Fatras")]
    if flags.Detector.EnableTGC:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawTgcSimHits")]
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('MUON', True)):
            simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawTgcSimHits_G4")]
            if (not ('G4MS' in flags.Sim.ISF.Simulator.value and 'ACTS' in flags.Sim.ISF.Simulator.value)):
                simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawTgcSimHits_Fatras")]
    if flags.Detector.EnablesTGC:
        simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawStgcSimHits")]
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('MUON', True)):
            simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawStgcSimHits_G4")]
            if (not ('G4MS' in flags.Sim.ISF.Simulator.value and 'ACTS' in flags.Sim.ISF.Simulator.value)):
                simHitContainers+=[("xAOD::MuonSimHitContainer", "xRawStgcSimHits_Fatras")]

    return simHitContainers
