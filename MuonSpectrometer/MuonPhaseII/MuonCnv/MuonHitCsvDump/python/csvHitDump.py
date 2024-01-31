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

    from MuonHitCsvDump.MuonHitCsvDumpConfig import CsvMuonSimHitDumpCfg, CsvMuonStripDumpCfg, CsvMdtDriftCircleDumpCfg

    truthContainers = []
    if flags.Detector.GeometryMDT:
        truthContainers += ["xMdtSimHits"]    
        cfg.merge(CsvMdtDriftCircleDumpCfg(flags))

    if flags.Detector.GeometryRPC:
        truthContainers += ["xMdtSimHits"]
        cfg.merge(CsvMuonStripDumpCfg(flags, name = "RpcCsvDumper", ContainerKey="xRpcStrips", PreFix="Rpc"))
 
    if flags.Detector.GeometryTGC:
        truthContainers += ["xTgcSimHits"]
        cfg.merge(CsvMuonStripDumpCfg(flags,name = "TgcCsvDumper", ContainerKey="xTgcStrips", PreFix="Tgc"))

    if False and flags.Detector.GeometrysTGC:
        truthContainers += ["xStgcSimHits"]
        cfg.merge(CsvMuonStripDumpCfg(flags,name = "sTgcCsvDumper", ContainerKey="xAODsTGCStrips", PreFix="sTgc"))

    if False and flags.Detector.GeometryMM:
        truthContainers += ["xMmSimHits"]
        cfg.merge(CsvMuonStripDumpCfg(flags,name = "MmCsvDumper", ContainerKey="xAODMMClusters", PreFix="Mm"))

    ### Truth hit conversion
    cfg.merge(CsvMuonSimHitDumpCfg(flags, MuonSimHitKey = truthContainers))

    executeTest(cfg, num_events = args.nEvents)


    
