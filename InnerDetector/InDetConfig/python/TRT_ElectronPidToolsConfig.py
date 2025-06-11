# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TRT_LocalOccupancyCfg(flags, name="TRT_LocalOccupancy", **kwargs):
    acc = ComponentAccumulator()
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_CalDbToolCfg
    CalDbTool = acc.popToolsAndMerge(TRT_CalDbToolCfg(flags))
    acc.addPublicTool(CalDbTool)  # public as it is has many clients to save some memory
    kwargs.setdefault("TRTCalDbTool", CalDbTool)

    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTStrawStatusCondAlgCfg, TRTStrawCondAlgCfg
    acc.merge(TRTStrawStatusCondAlgCfg(flags))
    acc.merge(TRTStrawCondAlgCfg(flags))

    kwargs.setdefault("isTrigger", False)

    if "TRT_DriftCircleCollection" not in kwargs:
        # TODO: Drift circles
        pass

    acc.setPrivateTools(CompFactory.InDet.TRT_LocalOccupancy(name, **kwargs))
    return acc

def TrigTRT_LocalOccupancyCfg(flags, name="TrigTRT_LocalOccupancy", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("isTrigger", True)
    kwargs.setdefault("TRT_DriftCircleCollection", "TRT_TrigDriftCircles")

    acc.setPrivateTools(acc.popToolsAndMerge(TRT_LocalOccupancyCfg(flags,name,**kwargs)))
    return acc


def TRT_OverlayLocalOccupancyCfg(flags, name="TRT_OverlayLocalOccupancy", **kwargs):
    """Return a ComponentAccumulator for overlay TRT_LocalOccupancy Tool"""
    kwargs.setdefault("TRT_DriftCircleCollection", "")
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTStrawStatusCondAlgCfg
    acc = TRTStrawStatusCondAlgCfg(flags, StrawStatusHTSummaryWriteKey="")
    acc.setPrivateTools(acc.popToolsAndMerge(TRT_LocalOccupancyCfg(flags, name, **kwargs)))
    return acc


def TRTOccupancyIncludeCfg(flags, name="TRTOccupancyInclude", **kwargs):
    acc = ComponentAccumulator()
    tool = acc.popToolsAndMerge(TRT_LocalOccupancyCfg(flags))
    acc.addPublicTool(tool)
    kwargs.setdefault("TRT_LocalOccupancyTool", tool)
    acc.addEventAlgo(CompFactory.TRTOccupancyInclude(name, **kwargs))
    return acc


def __TRT_dEdxToolBaseCfg(flags, name, **kwargs):
    """internal function to avoid code duplication,
    it does not  deal with LumiBlockMuWriterCfg dependancy correctly
    """

    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTToTCondAlgCfg
    acc = TRTToTCondAlgCfg(flags)

    kwargs.setdefault("TRT_dEdx_isData", not flags.Input.isMC)

    acc.setPrivateTools(CompFactory.TRT_ToT_dEdx(name,**kwargs))
    return acc


def TRT_dEdxToolCfg(flags, name="TRT_dEdxTool", **kwargs):
    acc = ComponentAccumulator()
    
    if not flags.Input.isMC:
        from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
        acc.merge(LumiBlockMuWriterCfg(flags))

    if "TRT_LocalOccupancyTool" not in kwargs:
        kwargs.setdefault("TRT_LocalOccupancyTool", acc.popToolsAndMerge(
            TRT_LocalOccupancyCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPrdAssociationToolCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPrdAssociationToolCfg(flags)))

    if "TRTStrawSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))

    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep in [ProductionStep.MinbiasPreprocessing]:
        kwargs.setdefault("averageInteractionsPerCrossingKey", f"{flags.Overlay.BkgPrefix}EventInfo.averageInteractionsPerCrossing")

    acc.setPrivateTools(acc.popToolsAndMerge(
        __TRT_dEdxToolBaseCfg(flags, name, **kwargs)))
    return acc


def TrigTRT_dEdxToolCfg(flags, name="TrigTRT_dEdxTool", **kwargs):
    """trigger version should not add LumiBlockMuWriterCfg to views as it is scheduled globally"""
    
    acc = ComponentAccumulator()
    
    if "TRT_LocalOccupancyTool" not in kwargs:
        kwargs.setdefault("TRT_LocalOccupancyTool", acc.popToolsAndMerge(TrigTRT_LocalOccupancyCfg(flags)))

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import TrigPrdAssociationToolCfg
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(TrigPrdAssociationToolCfg(flags)))

    acc.setPrivateTools(acc.popToolsAndMerge(__TRT_dEdxToolBaseCfg(flags, name, **kwargs)))
    return acc

def TRT_ElectronPidToolCfg(flags, name="TRT_ElectronPidTool", **kwargs):
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTHTCondAlgCfg, TRTPIDNNCondAlgCfg
    acc = TRTHTCondAlgCfg(flags)

    kwargs.setdefault("CalculateNNPid", True)

    if kwargs["CalculateNNPid"]:
        acc.merge(TRTPIDNNCondAlgCfg(flags))

    if "TRTStrawSummaryTool" not in kwargs:
        from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
        kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags)))

    if "TRT_LocalOccupancyTool" not in kwargs:
        kwargs.setdefault("TRT_LocalOccupancyTool", acc.popToolsAndMerge(TRT_LocalOccupancyCfg(flags)))

    if "TRT_ToT_dEdx_Tool" not in kwargs:
        kwargs.setdefault("TRT_ToT_dEdx_Tool", acc.popToolsAndMerge(TRT_dEdxToolCfg(flags)))

    acc.setPrivateTools(CompFactory.InDet.TRT_ElectronPidToolRun2(name, **kwargs))
    return acc

def TrigTRT_ElectronPidToolCfg(flags, name="InDetTrigTRT_ElectronPidTool", **kwargs):

    acc = ComponentAccumulator()

    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg
    kwargs.setdefault("TRTStrawSummaryTool", acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags,name="InDetTrigTRTStrawStatusSummaryTool")))

    kwargs.setdefault("TRT_LocalOccupancyTool", acc.popToolsAndMerge(TrigTRT_LocalOccupancyCfg(flags)))
    kwargs.setdefault("TRT_ToT_dEdx_Tool", acc.popToolsAndMerge(TrigTRT_dEdxToolCfg(flags)))

    kwargs.setdefault("CalculateNNPid", False)

    acc.setPrivateTools(acc.popToolsAndMerge(TRT_ElectronPidToolCfg(flags,name,**kwargs)))
    return acc

def GSFBuildTRT_ElectronPidToolCfg(flags, name="GSFBuildTRT_ElectronPidTool", isTrigger=False, **kwargs):
    kwargs.setdefault("CalculateNNPid", False)
    kwargs.setdefault("MinimumTrackPtForNNPid", 0.)
    if isTrigger:
        return TrigTRT_ElectronPidToolCfg(flags, name="TrigGSFBuildTRT_ElectronPidTool", **kwargs)
    else:
        return TRT_ElectronPidToolCfg(flags, name, **kwargs)


