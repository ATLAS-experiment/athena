# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
## @brief this function sets up the L1 simulation sequence with the TRT
## it covers the case of rerunning the L1 on run2 HI data

from AthenaConfiguration.Enums import Format

def L1TRTSimCfg(flags, name="TrigT1TRT"):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTStrawStatusCondAlgCfg
    acc.merge(TRTStrawStatusCondAlgCfg(flags))
    from TRT_ConditionsServices.TRT_ConditionsServicesConfig import TRT_StrawStatusSummaryToolCfg,TRT_StrawNeighbourSvcCfg
    trtStrawStatusSummaryTool = acc.popToolsAndMerge(TRT_StrawStatusSummaryToolCfg(flags))
    trtStrawNeighbourSvc = acc.getPrimaryAndMerge(TRT_StrawNeighbourSvcCfg(flags))
    from AthenaConfiguration.ComponentFactory import CompFactory
    if flags.Input.Format is Format.BS:
        from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import TRTRawDataProviderCfg
        acc.merge(TRTRawDataProviderCfg(flags))
    else:
        from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
        acc.merge(L1ConfigSvcCfg(flags))
    acc.addEventAlgo(CompFactory.LVL1.TrigT1TRT(name,
                                                TTCMultiplicity = flags.Trigger.TRT.TTCMultiplicity,
                                                maskedChipsFile = flags.Trigger.TRT.maskedChipsFile,
                                                ConditionsSummaryTool = trtStrawStatusSummaryTool,
                                                StrawNeighbourSvc = trtStrawNeighbourSvc
                                                ))
    return acc
