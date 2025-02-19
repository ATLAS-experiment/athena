# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaborationation

if __name__=="__main__":
    
    from MuonGeoModelTest.testGeoModel import SetupArgParser

    parser = SetupArgParser()
    parser.set_defaults(condTag="CONDBR2-BLKPA-2024-03")
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"])
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.lock()
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    ### Build segments from the leagcy chain
    from MuonConfig.MuonReconstructionConfig import MuonReconstructionCfg
    cfg.merge(MuonReconstructionCfg(flags))
  
  
    from MuonGeoModelTestR4.testGeoModel import executeTest
    executeTest(cfg)
    
