# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2024-03")
    parser.set_defaults(inputFile=[
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"
                                    # "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    ])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)

    from MuonHitCsvDump.MuonHitCsvDumpConfig import  CsvSpacePointDumpCfg, CsvMuonTruthSegmentDumpCfg
   
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    if flags.Input.isMC:
        cfg.merge(CsvMuonTruthSegmentDumpCfg(flags))
    cfg.merge(CsvSpacePointDumpCfg(flags))

    executeTest(cfg)


    
