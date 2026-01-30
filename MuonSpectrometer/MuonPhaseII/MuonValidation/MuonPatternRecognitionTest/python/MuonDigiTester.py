# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()

    parser.add_argument("--doFullsTGCDigi", default=False, action='store_true')
    parser.add_argument("--doFullMMDigi", default=False, action='store_true')
    
    parser.set_defaults(nEvents = -1)
 
    from MuonGeoModelTestR4.testGeoModel import MuonPhaseIITestDefaults
    parser.set_defaults(inputFile = MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(eventPrintoutLevel = 50)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Muon.doFastMMDigitization = not args.doFullMMDigi
    flags.Muon.doFastsTGCDigitization = not args.doFullsTGCDigi

    flags, cfg = setupGeoR4TestCfg(args,flags)
  
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonPRDTestR4.MuonHitTestConfig import MuonDigiTestCfg
    cfg.merge(MuonDigiTestCfg(flags))

    cfg.getService("MessageSvc").setVerbose = []

    executeTest(cfg)