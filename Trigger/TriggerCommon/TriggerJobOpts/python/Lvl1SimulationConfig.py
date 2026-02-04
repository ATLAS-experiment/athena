# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @brief this function sets up the top L1 simulation sequence
##
## it covers the two cases of running L1 in the MC simulation and for rerunning on data


def Lvl1SimulationCfg(flags, seqName = None):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    if seqName:
        from AthenaCommon.CFElements import parOR
        acc = ComponentAccumulator(sequence=parOR(seqName))
    else:
        acc = ComponentAccumulator()

    from AthenaCommon.CFElements import seqAND
    acc.addSequence(seqAND('L1SimSeq'))

    if flags.Trigger.enableL1CaloLegacy:
        acc.addSequence(seqAND('L1CaloLegacySimSeq'), parentName='L1SimSeq')
        from TrigT1CaloSim.TrigT1CaloSimRun2Config import L1CaloLegacySimCfg
        acc.merge(L1CaloLegacySimCfg(flags), sequenceName='L1CaloLegacySimSeq')

    acc.addSequence(seqAND('L1CaloSimSeq'), parentName='L1SimSeq')

    if flags.Trigger.enableL1CaloPhase1:
        from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
        acc.merge(L1CaloFEXSimCfg(flags), sequenceName = 'L1CaloSimSeq')


    if flags.Trigger.enableL1MuonPhase1:
        acc.addSequence(seqAND('L1MuonSimSeq'), parentName='L1SimSeq')
        from TriggerJobOpts.Lvl1MuonSimulationConfig import Lvl1MuonSimulationCfg
        acc.merge(Lvl1MuonSimulationCfg(flags), sequenceName='L1MuonSimSeq')

    if flags.Trigger.L1.doTopo:
        acc.addSequence(seqAND('L1TopoSimSeq'), parentName='L1SimSeq')
        from L1TopoSimulation.L1TopoSimulationConfig import L1TopoSimulationCfg
        acc.merge(L1TopoSimulationCfg(flags), sequenceName='L1TopoSimSeq')

    if flags.Trigger.doZDC:
        acc.addSequence(seqAND('L1ZDCSimSeq'),parentName='L1SimSeq')
        from TrigT1ZDC.TrigT1ZDCConfig import L1ZDCSimCfg
        acc.merge(L1ZDCSimCfg(flags), sequenceName = 'L1ZDCSimSeq')

    if flags.Trigger.doTRT:
        acc.addSequence(seqAND('L1TRTSimSeq'),parentName='L1SimSeq')
        from TrigT1TRT.TrigT1TRTConfig import L1TRTSimCfg
        acc.merge(L1TRTSimCfg(flags), sequenceName = 'L1TRTSimSeq')


    acc.addSequence(seqAND('L1CTPSimSeq'), parentName='L1SimSeq')
    from TrigT1CTP.CTPSimulationConfig import CTPSimulationCfg
    acc.merge(CTPSimulationCfg(flags), sequenceName="L1CTPSimSeq")


    return acc

if __name__ == '__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = ['root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/data-art/large-input/trig-val/TriggerTest/valid1.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8514_e8528_s4369_s4370_r16083_tid42189392_00/RDO.42189392._000001.pool.root.1']
    flags.Exec.MaxEvents = 5
    flags.Concurrency.NumThreads = 1
    flags.Trigger.triggerMenuSetup = 'Dev_pp_run3_v1'
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    flags.Trigger.doHLT = True # this is necessary so that the simulation of L1Calo (if running on MC) gets output with keys that Topo sim expects
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)

    acc.merge(Lvl1SimulationCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    sys.exit(acc.run().isFailure())
