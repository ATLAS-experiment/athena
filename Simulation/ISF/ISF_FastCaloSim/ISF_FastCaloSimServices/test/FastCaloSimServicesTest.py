#!/usr/bin/env python
"""Run tests for FastCaloSimServices configuration

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""


def FCSServicesMainCfg(flags):
    """Configure event loop for FCSServices"""
    acc = MainServicesCfg(flags)
    if flags.Concurrency.NumThreads > 0:
        AthenaHiveEventLoopMgr = CompFactory.AthenaHiveEventLoopMgr
        elmgr = AthenaHiveEventLoopMgr()
    else:
        AthenaEventLoopMgr = CompFactory.AthenaEventLoopMgr
        elmgr = AthenaEventLoopMgr()
    elmgr.UseSecondaryEventNumber = True
    acc.addService(elmgr)
    return acc


def FastCaloSimServicesMainCfg(flags):
    """Main FCSServices configuration"""

    # Construct our accumulator to run
    acc = FCSServicesMainCfg(flags)
    acc.merge(PoolReadCfg(flags))

    # Add TileInfoLoader
    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge(TileInfoLoaderCfg(flags))

    # Add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    acc.merge(BeamEffectsAlgCfg(flags))

    # Add Kernel_ATLFAST3MT from ISF_MainConfig
    from ISF_Config.ISF_MainConfig import ISF_KernelCfg
    acc.merge(ISF_KernelCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
    acc.merge( OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True))

    # FIXME hack to match to buggy behaviour in old style configuration
    OutputStreamHITS = acc.getEventAlgo(outputStreamName("HITS"))
    OutputStreamHITS.ItemList.remove("xAOD::EventInfo#EventInfo")
    OutputStreamHITS.ItemList.remove("xAOD::EventAuxInfo#EventInfoAux.")

    # FIXME hack because deduplication is broken
    PoolAttributes = ["TREE_BRANCH_OFFSETTAB_LEN = '100'"]
    PoolAttributes += [f"DatabaseName = '{flags.Output.HITSFileName}'; ContainerName = 'TTree=CollectionTree'; TREE_AUTO_FLUSH = '1'"]
    acc.getService("AthenaPoolCnvSvc").PoolAttributes += PoolAttributes

    # Dump config
    acc.getService("StoreGateSvc").Dump = True
    acc.getService("ConditionStore").Dump = True
    acc.printConfig(withDetails=True, summariseProps=True)

    return acc


if __name__ == '__main__':

    import sys
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    from ISF_FastCaloSimServices.ISF_FastCaloSimServicesTestHelpers import (
        CommonTestArgumentParser, JobOptsDumperCfg, TestMessageSvcCfg,
        defaultTestFlags, postprocessAndLockFlags, printAndRun
    )

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from SimulationConfig.SimEnums import SimulationFlavour
    flags = initConfigFlags()
    flags.Sim.ISF.Simulator = SimulationFlavour.ATLFAST3MT_QS
    from AthenaConfiguration.ComponentFactory import CompFactory

    # Configure
    args = CommonTestArgumentParser().parse_args()
    defaultTestFlags(flags, args)
    postprocessAndLockFlags(flags, args)

    # Construct our accumulator to run
    acc = FastCaloSimServicesMainCfg(flags)
    acc.merge(JobOptsDumperCfg(flags))
    acc.merge(TestMessageSvcCfg(flags))

    # Ignore checking compatibility of G4 simulation version and
    # G4 version used to calculate Tile sampling fractions
    acc.getCondAlgo("TileSamplingFractionCondAlg").G4Version = -1

    # Dump pickle
    with open("FCSServices_Config.pkl", "wb") as f:
        acc.store(f)

    # Print and run
    sys.exit(printAndRun(acc, flags, args))
