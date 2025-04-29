#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def MuonDQAMonitoringConfig(flags):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaMonitoring.DQConfigFlags import DQDataType

    result = ComponentAccumulator()

    if flags.DQ.Steering.Muon.doRawMon:
        # do not run in RAW->ESD, or AOD-only
        if flags.DQ.Environment not in ('tier0Raw', 'AOD'):
            from MdtRawDataMonitoring.MDTMonitorAlgorithm import MdtMonitoringConfig
            from RpcRawDataMonitoring.RpcMonitorAlgorithm import RpcMonitoringConfig
            from TgcRawDataMonitoring.TgcRawDataMonitorAlgorithm import TgcRawDataMonitoringConfig
            result.merge(MdtMonitoringConfig(flags))
            result.merge(RpcMonitoringConfig(flags))
            result.merge(TgcRawDataMonitoringConfig(flags))
            if flags.Detector.GeometryCSC:
                from CscRawDataMonitoring.CscMonitoringESD_Alg import CscMonitoringESD_AlgConfig
                result.merge(CscMonitoringESD_AlgConfig(flags))
            if flags.Detector.GeometrysTGC:
                from StgcRawDataMonitoring.StgcMonitorAlgorithm import sTgcMonitoringConfig
                if flags.DQ.DataType is not DQDataType.MC:
                   result.merge(sTgcMonitoringConfig(flags,NSW_PadTrigKey='NSW_PadTrigger_RDO'))
                else:
                   result.merge(sTgcMonitoringConfig(flags))
            if flags.Detector.GeometryMM and flags.Detector.GeometrysTGC:
                from NSWRawDataMonitoring.NSWMonitorAlgorithm import  NSWMonitoringCfg
                result.merge(NSWMonitoringCfg(flags))

        if flags.DQ.Environment in ('online', 'tier0','tier0Raw'):
            if flags.Detector.GeometryCSC:
                from CscRawDataMonitoring.CscMonitoringRAW_Alg import CscMonitoringRAW_AlgConfig
                result.merge(CscMonitoringRAW_AlgConfig(flags))
            if flags.Detector.GeometryMM:
                from MMRawDataMonitoring.MMMonitorAlgorithm import MMMonitoringConfig
                result.merge(MMMonitoringConfig(flags))

    if flags.DQ.Steering.Muon.doTrackMon:
        # do not run in RAW->ESD
        if flags.DQ.Environment not in ('tier0Raw',):
            from MuonTrackMonitoring.MuonTrackMonitorAlgorithm import MuonTrackConfig
            result.merge(MuonTrackConfig(flags))
    return result
