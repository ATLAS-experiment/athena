# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, geoModelFileDefault
    parser = SetupArgParser()
    parser.add_argument("--setupRun4", default=True, action="store_true")
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noRpc=True)
    parser.set_defaults(noTgc=True)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])


    args = parser.parse_args()
    args.geoModelFile = geoModelFileDefault(args.setupRun4)
    flags, cfg = setupGeoR4TestCfg(args)
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    from MuonCondTest.MdtCablingTester import MdtCablingTestAlgCfg
    cfg.merge(MdtCablingTestAlgCfg(flags))
    cfg.merge(MdtCalibDbAlgCfg(flags))
    from MuonConfig.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    cfg.merge(MuonSimHitToMeasurementCfg(flags))
    executeTest(cfg)
