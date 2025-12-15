# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
## @brief this function sets up the L1 simulation sequence with the ZDC
## it covers the case of rerunning the L1 on run2 HI data

from AthenaCommon.Logging import logging

log = logging.getLogger("ZdcRecConfig")

def L1ZDCSimCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from ZdcRec.ZdcRecConfig import ZdcRecRun2Cfg, ZdcRecRun3Cfg, ZdcRecOutputCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import LHCPeriod

    if flags.GeoModel.Run is LHCPeriod.Run2:
        acc.merge(ZdcRecRun2Cfg(flags))
        acc.addEventAlgo(CompFactory.LVL1.TrigT1Run2ZDC(filepath_LUT = flags.Trigger.ZdcLUT, EnergyADCScale = 0.4))
    elif flags.GeoModel.Run is LHCPeriod.Run3:
        acc.merge(ZdcRecRun3Cfg(flags))
        acc.addEventAlgo(CompFactory.LVL1.TrigT1Run3ZDC(filepath_LUT = 'TrigT1ZDC/zdc_json_PbPb5.36TeV_2024_TriggerSim.json',
                                                MinSampleAna = 7,
                                                MaxSampleAna = 15,
                                                NegHG2ndDerivThresh = 45,
                                                NegLG2ndDerivThresh = 15,
                                                BaselineDelta = 3
                                                )) #all EB runs > 462494 --> all use this LUT
    else:
        log.error(f'L1ZDCSimCfg: Unsupported LHCPeriod: {flags.GeoModel.Run}')
    
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))

    return acc

if __name__ == '__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    flags = initConfigFlags()
    #flags.Input.Files = ['/eos/user/m/mhoppesc/datasets/data23_hi/data23_hi.00462533.physics_EnhancedBias.merge.RAW._lb0680._SFO-17._0001.1']
    flags.Input.Files = ['/eos/atlas/atlascerngroupdisk/phys-hi/trig-heavyion/data18_hi.00367273.physics_EnhancedBias.merge.RAW/data18_hi.00367273.physics_EnhancedBias.merge.RAW._lb0251._SFO-7._0002.1']
    flags.Common.isOnline=False
    flags.Exec.MaxEvents=100
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Scheduler.ShowDataFlow=True
    flags.Trigger.enableL1MuonPhase1=True
    flags.Trigger.triggerMenuSetup='PhysicsP1_HI_run3_v1'
    flags.Trigger.EDMVersion=3
    flags.Trigger.doZDC=True
    flags.Trigger.enableL1CaloPhase1 = False # FIXME: ATR-27095
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
    flags.fillFromArgs()
    flags.lock()
    


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)

    from TriggerJobOpts.Lvl1SimulationConfig import Lvl1SimulationCfg
    acc.merge(Lvl1SimulationCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    with open("L1Sim.pkl", "wb") as p:
        acc.store(p)
        p.close()

    sys.exit(acc.run().isFailure())
