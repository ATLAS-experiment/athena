# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
## @brief this function sets up the L1 simulation sequence with the TRT
## it covers the case of rerunning the L1 on run2 HI data


def L1TRTSimCfg(flags, name="TrigT1TRT"):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
    acc.merge(L1ConfigSvcCfg(flags))
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConfig import TRTStrawStatusCondAlgCfg
    acc.merge(TRTStrawStatusCondAlgCfg(flags))
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo(CompFactory.LVL1.TrigT1TRT(name,
                                                TTCMultiplicity = flags.Trigger.TRT.TTCMultiplicity,
                                                maskedChipsFile = flags.Trigger.TRT.maskedChipsFile
                                                ))
    return acc
