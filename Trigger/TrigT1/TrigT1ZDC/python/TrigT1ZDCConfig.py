# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
## @brief this function sets up the L1 simulation sequence with the ZDC
## it covers the case of rerunning the L1 on run2 HI data


def L1ZDCSimCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from ZdcRec.ZdcRecConfig import ZdcRecRun2Cfg, ZdcRecRun3Cfg,ZdcRecOutputCfg
    from AthenaConfiguration.ComponentFactory import CompFactory

    pn = flags.Input.ProjectName
    year = int(pn.split('_')[0].split('data')[1])
    if (year < 20):
        acc.merge(ZdcRecRun2Cfg(flags))
        acc.addEventAlgo(CompFactory.LVL1.TrigT1ZDC(filepath_LUT = flags.Trigger.ZdcLUT,
                                                EnergyADCScale = 0.4))
    else:
        acc.merge(ZdcRecRun3Cfg(flags))
        acc.addEventAlgo(CompFactory.LVL1.TrigT1ZDC(filepath_LUT = 'TrigT1ZDC/zdc_json_PbPb5.36TeV_2023.json',
                                                EnergyADCScale = 0)) #all EB runs > 462494 --> all use this LUT
    
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZdcRecOutputCfg(flags))

    return acc

if __name__ == '__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data15_hi.00287843.physics_EnhancedBias.merge.RAW._lb0226._SFO-2._0001.1_100evt']
    flags.Common.isOnline=False
    flags.Exec.MaxEvents=100
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents=1
    flags.Scheduler.ShowDataDeps=True
    flags.Scheduler.CheckDependencies=True
    flags.Scheduler.ShowDataFlow=True
    flags.Trigger.enableL1MuonPhase1=True
    flags.Trigger.triggerMenuSetup='Dev_HI_run3_v1'
    flags.Trigger.EDMVersion=3
    flags.Trigger.doZDC=True
    flags.Trigger.L1.doAlfaCtpin=True
    flags.Trigger.enableL1CaloPhase1 = False # FIXME: ATR-27095
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
    from AthenaCommon.Constants import DEBUG, INFO
    acc.getEventAlgo("CTPSimulation").OutputLevel=INFO  # noqa: ATL900
    acc.getEventAlgo("LVL1::TrigT1ZDC").OutputLevel=DEBUG


    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    with open("L1Sim.pkl", "wb") as p:
        acc.store(p)
        p.close()

    sys.exit(acc.run().isFailure())
