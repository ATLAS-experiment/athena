# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline


@AccumulatorCache
def TRT_CalDbToolCfg(flags, name="TRT_CalDbTool", **kwags):
    """Return a ComponentAccumulator for TRT_CalDbTool"""
    from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
    acc = ComponentAccumulator()
    
    acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/RT", "/TRT/Calib/RT", className="TRTCond::RtRelationMultChanContainer"))
    acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/T0", "/TRT/Calib/T0", className='TRTCond::StrawT0MultChanContainer'))
    acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/errors2d", "/TRT/Calib/errors2d", className="TRTCond::RtRelationMultChanContainer"))
    acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/slopes", "/TRT/Calib/slopes", className='TRTCond::RtRelationMultChanContainer'))
    
    acc.setPrivateTools(CompFactory.TRT_CalDbTool(name, **kwags))
    return acc


def TRT_StrawStatusSummaryToolCfg(flags, name="TRT_StrawStatusSummaryTool", forceLegacyAccess=False, **kwargs):
    """Return a ComponentAccumulator for TRT_StrawStatusSummaryTool"""
    acc = ComponentAccumulator()

    if flags.GeoModel.Align.LegacyConditionsAccess: # revert to old style CondHandle in case of simulation
        # Dead/Noisy Straw Lists not used!
        # Argon straw list
        acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Cond/StatusHT", "/TRT/Cond/StatusHT"))
    else:
        # Dead/Noisy Straw Lists
        from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTStrawStatusCondAlgCfg
        acc.merge(TRTStrawStatusCondAlgCfg(flags))

    
    kwargs.setdefault("isGEANT4", flags.GeoModel.Align.LegacyConditionsAccess or forceLegacyAccess)

    acc.setPrivateTools(CompFactory.TRT_StrawStatusSummaryTool(name, **kwargs))
    return acc

def TRT_StrawNeighbourSvcCfg(flags, name="TRT_StrawNeighbourSvc"):
    acc = ComponentAccumulator()
    
    svc = CompFactory.TRT_StrawNeighbourSvc(name)
    
    acc.addService(svc)
    return acc
    



def TRT_MCCalDbToolCfg(flags, name="TRT_CalDbTool2", **kwargs):
    """Return a ComponentAccumulator for TRT_CalDbTool configured for MC in case of data overlay."""
    # TODO: conditions (DataOverlay)
    kwargs.setdefault("RtFolderName", "/TRT/Calib/MC/RT")
    kwargs.setdefault("T0FolderName", "/TRT/Calib/MC/T0")
    return TRT_CalDbToolCfg(flags, name, **kwargs)
