# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__ == "__main__":
    from MuonCondTest.RpcCablingTester import RpcCablingTestAlgCfg
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, geoModelFileDefault
    parser = SetupArgParser()
    parser.add_argument("--JSONFile", help="External cabling JSON file", default="")
    parser.add_argument("--setupRun4", default=False, action="store_true")
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noMdt=True)
    parser.set_defaults(noTgc=True)

    args = parser.parse_args()
    args.geoModelFile = geoModelFileDefault(args.setupRun4)
    flags, cfg = setupGeoR4TestCfg(args)
    cfg.merge(RpcCablingTestAlgCfg(flags, TestStations = [], JSONFile = args.JSONFile))
    cfg.getService("MessageSvc").debugLimit = 2147483647
    cfg.getService("MessageSvc").verboseLimit = 2147483647
    cfg.getService("MessageSvc").infoLimit = 2147483647

    executeTest(cfg)



