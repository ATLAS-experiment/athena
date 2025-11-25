# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)

    from MuonHitCsvDump.MuonHitCsvDumpConfig import  CsvSpacePointDumpCfg, CsvMuonTruthSegmentDumpCfg
   
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    if flags.Input.isMC:
        cfg.merge(CsvMuonTruthSegmentDumpCfg(flags))
    cfg.merge(CsvSpacePointDumpCfg(flags))

    executeTest(cfg)


    
