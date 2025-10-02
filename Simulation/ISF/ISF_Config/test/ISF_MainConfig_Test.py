#!/usr/bin/env python
"""Run tests on ISF_MainConfig

This test inherits from Simulation/G4Atlas/G4AtlasAlg/test/G4AtlasAlgConfig_Test.py

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
if __name__ == '__main__':

    import time
    a = time.time()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # Set up logging
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)


    #import and set config flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 1
    flags.Exec.SkipEvents = 0
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    flags.Input.RunNumbers = [284500] #Isn't updating - todo: investigate
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1] # dummy value

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.EVNT_RUN3_50
    flags.Output.HITSFileName = "myHITSnew.pool.root"

    # Sim flags
    # flags.Sim.WorldRRange = 15000
    # flags.Sim.WorldZRange = 27000 #change defaults?
    from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization, SimulationFlavour, TruthStrategy
    flags.Sim.CalibrationRun = CalibrationRun.Off
    flags.Sim.RecordStepInfo = False
    flags.Sim.CavernBackground = CavernBackground.Signal
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim
    flags.Sim.ISFRun = True
    flags.Sim.ISF.Simulator = SimulationFlavour.FullG4MT

    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-14"
    flags.GeoModel.Align.Dynamic = False

    detectors = ['Bpipe', 'BCM', 'Pixel', 'SCT', 'TRT', 'LAr', 'Tile', 'MBTS', 'CSC', 'MDT', 'RPC', 'TGC', 'FwdRegion']
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, detectors, toggle_geometry=True)

    #Frozen showers OFF = 0
    flags.Sim.LArParameterization = LArParameterization.DeadMaterialFrozenShowers

    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus
    # Finalize
    flags.lock()

    ## Initialize a new component accumulator
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    # add the ISF_MainConfig
    from ISF_Config.ISF_MainConfig import ISF_KernelCfg
    cfg.merge(ISF_KernelCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
    cfg.merge(OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True))

    # FIXME hack to match to buggy behaviour in old style configuration
    OutputStreamHITS = cfg.getEventAlgo(outputStreamName("HITS"))
    OutputStreamHITS.ItemList.remove("xAOD::EventInfo#EventInfo")
    OutputStreamHITS.ItemList.remove("xAOD::EventAuxInfo#EventInfoAux.")

    # FIXME hack because deduplication is broken
    PoolAttributes = ["TREE_BRANCH_OFFSETTAB_LEN = '100'"]
    PoolAttributes += [f"DatabaseName = '{flags.Output.HITSFileName}'; ContainerName = 'TTree=CollectionTree'; TREE_AUTO_FLUSH = '1'"]
    cfg.getService("AthenaPoolCnvSvc").PoolAttributes += PoolAttributes

    # Dump config
    cfg.getService("StoreGateSvc").Dump = True
    cfg.getService("ConditionStore").Dump = True
    cfg.printConfig(withDetails=True, summariseProps = True)

    flags.dump()

    with open("test.pkl", "wb") as f:
        cfg.store(f)

    # Execute and finish
    sc = cfg.run()

    b = time.time()
    log.info("Run ISF_MainConfig_Test in %s seconds", str(b-a))

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
