# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(condTag="CONDBR2-BLKPA-2023-03")
    parser.set_defaults(inputFile=[
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    ])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)

    from MuonHitCsvDump.MuonHitCsvDumpConfig import CsvMuonSimHitDumpCfg, CsvSpacePointDumpCfg
    if flags.Input.isMC:
        from MuonConfig.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
        cfg.merge(MuonSimHitToMeasurementCfg(flags))

        truthContainers = []
        if flags.Detector.GeometryMDT: truthContainers += ["xMdtSimHits"]
        if flags.Detector.GeometryRPC: truthContainers += ["xMdtSimHits"] 
        if flags.Detector.GeometryTGC: truthContainers += ["xTgcSimHits"]
        if flags.Detector.GeometrysTGC: truthContainers += ["xStgcSimHits"]       
        if flags.Detector.GeometryMM: truthContainers += ["xMmSimHits"]       

        ### Truth hit conversion
        cfg.merge(CsvMuonSimHitDumpCfg(flags, MuonSimHitKey = truthContainers))

    else:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
        cfg.merge(MuonByteStreamDecodersCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
        cfg.merge(MuonRDOtoPRDConvertorsCfg(flags))


    cfg.merge(CsvSpacePointDumpCfg(flags))

    executeTest(cfg, num_events = args.nEvents)


    
