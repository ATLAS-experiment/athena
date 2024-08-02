#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

"""
@brief configuration for the min bias monitoring
"""
from TrigMinBiasMonitoring.TrigMBTSMonitoringMT import TrigMBTS
from TrigMinBiasMonitoring.TrigSPTRKMonitoringMT import TrigSPTRK
from TrigMinBiasMonitoring.TrigMinBiasEffMonitoring import TrigMinBiasEff
from TrigMinBiasMonitoring.TrigAFPSidHypoMonitoring import TrigAFPSidHypoMonitoring
from TrigMinBiasMonitoring.TrigFwdAFPMonitoring import TrigFwdAFPAllMonitoringCfg
from TrigMinBiasMonitoring.TrigFwdZDCMonitoring import TrigFwdZDCMonitoringAlg
from TrigMinBiasMonitoring.TrigHIMonitoringMT import TrigHIMonCfg

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType
from AthenaMonitoring.DQConfigFlags import DQDataType

def TrigMinBias(flags):
    acc = ComponentAccumulator()
    acc.merge(TrigMBTS(flags))
    if flags.Beam.Type is not BeamType.Cosmics:
        acc.merge(TrigSPTRK(flags))
        acc.merge(TrigMinBiasEff(flags))
        acc.merge(TrigAFPSidHypoMonitoring(flags))
        acc.merge(TrigFwdAFPAllMonitoringCfg(flags))
        if flags.DQ.DataType is DQDataType.HeavyIon:
            acc.merge(TrigFwdZDCMonitoringAlg(flags))
            acc.merge(TrigHIMonCfg(flags))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.DQ.Environment = "AOD"
    flags.Output.HISTFileName = 'TestMinBiasMonitorOutput.root'
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
    flags.Exec.MaxEvents = -1
    # flags.Common.doExpressProcessing=True # enable for express stream data processing
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigMinBias(flags))

    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    status = cfg.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
