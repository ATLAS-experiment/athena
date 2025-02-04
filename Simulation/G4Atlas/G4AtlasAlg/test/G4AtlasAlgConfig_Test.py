#!/usr/bin/env python
"""Run tests on G4AtlasAlgConfig

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

    flags.Exec.MaxEvents = 4
    flags.Exec.SkipEvents = 0
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation
    flags.Input.RunNumbers = [284500] #Isn't updating - todo: investigate
    flags.Input.OverrideRunNumber = True
    flags.Input.LumiBlockNumbers = [1]
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1'] #defaultTestFiles.EVNT
    flags.Output.HITSFileName = "myHITSnew.pool.root"

    #Sim flags
    #flags.Sim.WorldRRange = 15000
    #flags.Sim.WorldZRange = 27000 #change defaults?
    from SimulationConfig.SimEnums import CalibrationRun, CavernBackground, SimulationFlavour, TruthStrategy
    flags.Sim.CalibrationRun = CalibrationRun.Off
    flags.Sim.RecordStepInfo = False
    flags.Sim.CavernBackground = CavernBackground.Off
    flags.Sim.ISFRun = False
    flags.Sim.TruthStrategy = TruthStrategy.MC15aPlus
    flags.Sim.ISF.Simulator = SimulationFlavour.AtlasG4
    flags.Sim.TightMuonStepping=True
    from SimulationConfig.G4Optimizations import enableBeamPipeKill, enableFrozenShowersFCalOnly
    enableBeamPipeKill(flags)
    enableFrozenShowersFCalOnly(flags)

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.GeoModel.Align.Dynamic = False
    flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-14"

    # To respect --athenaopts
    flags.fillFromArgs()

    # Finalize
    flags.lock()

    ## Initialize a new component accumulator
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    if flags.Input.Files:
        if "xAOD::EventInfo#EventInfo" not in flags.Input.TypedCollections:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
            cfg.merge(EventInfoCnvAlgCfg(flags))
        else:
            from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
            cfg.merge(EventInfoUpdateFromContextAlgCfg(flags))

    #add the G4AtlasAlg
    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    cfg.merge(G4AtlasAlgCfg(flags))
    AcceptAlgNames = ['G4AtlasAlg']

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
    cfg.merge(OutputStreamCfg(flags, "HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True, AcceptAlgs=AcceptAlgNames))

   # Add MT-safe PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, "HITS", AcceptAlgs=AcceptAlgNames))

    # Print config
    cfg.printConfig(withDetails=True, summariseProps = True)

    flags.dump()

    with open("test.pkl", "wb") as f:
        cfg.store(f)

    # Execute and finish
    sc = cfg.run()

    b = time.time()
    log.info("Run G4AtlasAlg in %s seconds", str(b-a))

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
