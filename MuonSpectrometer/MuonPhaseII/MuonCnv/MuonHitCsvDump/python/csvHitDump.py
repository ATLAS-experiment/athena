# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile = ["myMuonSimTestStream.pool.root"])
    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    cfg.merge(MuonSimHitToMeasurementCfg(flags))

    from MuonHitCsvDump.MuonHitCsvDumpConfig import CsvMuonSimHitDumpCfg, CsvSpacePointDumpCfg

    truthContainers = []
    if flags.Detector.GeometryMDT:
        truthContainers += ["xMdtSimHits"]    


    if flags.Detector.GeometryRPC:
        truthContainers += ["xMdtSimHits"]
 
    if flags.Detector.GeometryTGC:
        truthContainers += ["xTgcSimHits"]

    if flags.Detector.GeometrysTGC:
        truthContainers += ["xStgcSimHits"]       

    if flags.Detector.GeometryMM:
        truthContainers += ["xMmSimHits"]       

    ### Truth hit conversion
    cfg.merge(CsvMuonSimHitDumpCfg(flags, MuonSimHitKey = truthContainers))
    cfg.merge(CsvSpacePointDumpCfg(flags))

    executeTest(cfg, num_events = args.nEvents)


    
